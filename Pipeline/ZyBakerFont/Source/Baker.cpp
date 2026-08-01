// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Baker.hpp"
#include "Importer/STBImporter.hpp"
#include "Process/Atlas.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Font
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Baker::Extension Normalize(Text Type)
    {
        const Text Trimmed = (!Type.IsEmpty() && Type[0] == '.') ? Type.Slice(1) : Type;

        Baker::Extension Result(Trimmed.GetSize());

        for (UInt Index = 0; Index < Trimmed.GetSize(); ++Index)
        {
            Result.Append(StrLowercase(Trimmed[Index]));
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Baker::Baker()
    {
        Register(Retainer<STBImporter>::Create());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Register(ConstRetainer<Importer> Codec)
    {
        ZY_ASSERT(Codec != nullptr, "Cannot register a null importer");

        for (const Text Type : Codec->GetTypes())
        {
            mRegistry.Assign(Normalize(Type), Codec);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Unregister(Text Type)
    {
        return mRegistry.Erase(Normalize(Type));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Importer> Baker::Find(Text Type) const
    {
        const Extension Key = Normalize(Type);

        if (const ConstPtr<Retainer<Importer>> Found = mRegistry.Find(Key))
        {
            return static_cast<Ptr<Importer>>(* Found);
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Baker::Bake(ConstSpan<Byte> Source, Text Type, ConstRef<Profile> Profile) const
    {
        if (Profile.Size <= 0.0f || Profile.Range <= 0.0f)
        {
            LOG_E("Font: the em size and the range both have to be positive");

            return Blob();
        }

        if (Profile.Charset.IsEmpty())
        {
            LOG_E("Font: the charset selects no codepoint at all");

            return Blob();
        }

        const ConstPtr<Importer> Codec = Find(Type);

        if (Codec == nullptr)
        {
            LOG_E("Font: '{0}' is not a source format this baker understands", Type);

            return Blob();
        }

        Typeface Face = Codec->Import(Source, Profile);

        if (Face.IsEmpty())
        {
            LOG_E("Font: the typeface carries none of the requested codepoints");

            return Blob();
        }

        Sequence<Cell> Cells;
        Cells.Reserve(Face.GetGlyphs().GetSize());

        for (ConstRef<Typeface::Glyph> Glyph : Face.GetGlyphs())
        {
            Ref<Cell> Entry = Cells.Append();
            Entry.Codepoint = Glyph.Codepoint;

            Generator::Generate(Glyph.Outline, Profile.Range, Profile.Angle, Entry.Data);
        }

        const Atlas::Layout Sheet = Atlas::Arrange(Cells, Profile.Padding, Profile.Limit);

        if (Sheet.Pages == 0 || Sheet.Side > 65535)
        {
            LOG_E("Font: a glyph is larger than a whole atlas page, so lower the size or the range");

            return Blob();
        }

        Sequence<Blob> Pages;
        Pages.Reserve(Sheet.Pages);

        for (UInt32 Page = 0; Page < Sheet.Pages; ++Page)
        {
            Pages.Append(Atlas::Compose(Cells, Sheet.Side, Page));
        }

        // The glyph table is the engine's own type, so the loader reads back exactly what is written here.
        Render::Font::Glyphs Glyphs;
        Glyphs.Reserve(Cells.GetSize());

        const Real32 Extent = static_cast<Real32>(Sheet.Side);

        for (ConstRef<Cell> Entry : Cells)
        {
            // Value-initialized because the glyph table is written to disk as one dense block, so any padding
            // the compiler leaves between the fields would otherwise put indeterminate bytes in the file.
            Render::Font::Glyph Glyph { };
            Glyph.Page = static_cast<UInt16>(Entry.Page);

            // A blank carries no field, and the canvas skips a zero-size rectangle when drawing text.
            if (Entry.Data.Width > 0 && Entry.Data.Height > 0)
            {
                ConstRef<Rect> Bounds = Entry.Data.Bounds;

                // Everything the engine stores is in em units, which is what lets one bake serve every size.
                Glyph.LocalBounds = Rect(
                    Bounds.GetMinimumX() / Profile.Size,
                    Bounds.GetMinimumY() / Profile.Size,
                    Bounds.GetMaximumX() / Profile.Size,
                    Bounds.GetMaximumY() / Profile.Size);

                // The quad's corners land on the centres of the outermost texels rather than on their outer
                // edges, which is the half-texel inset the field was generated against.
                Glyph.AtlasBounds = Rect(
                    (static_cast<Real32>(Entry.X) + 0.5f) / Extent,
                    (static_cast<Real32>(Entry.Y) + 0.5f) / Extent,
                    (static_cast<Real32>(Entry.X + Entry.Data.Width)  - 0.5f) / Extent,
                    (static_cast<Real32>(Entry.Y + Entry.Data.Height) - 0.5f) / Extent);
            }
            Glyphs.Assign(Entry.Codepoint, Glyph);
        }

        // Packing reorders the cells by height, so the advance is read back from the decoded glyph rather than
        // from whatever cell happens to sit at the same index.
        for (ConstRef<Typeface::Glyph> Glyph : Face.GetGlyphs())
        {
            if (const Ptr<Render::Font::Glyph> Entry = Glyphs.Find(Glyph.Codepoint))
            {
                Entry->Advance = Glyph.Advance;
            }
        }

        Render::Font::Kerning Kerning = Face.GetKerning();

        Render::Font::Metrics Metrics;
        Metrics.Size               = Profile.Size;
        Metrics.Distance           = Profile.Range;
        Metrics.Ascender           = Face.GetMetrics().Ascender;
        Metrics.Descender          = Face.GetMetrics().Descender;
        Metrics.UnderlineOffset    = Face.GetMetrics().UnderlineOffset;
        Metrics.UnderlineSize      = Profile.Underline;
        Metrics.UnderlineThickness = Face.GetMetrics().UnderlineThickness;

        LOG_I("Font: {0} glyph(s), {1} kerning pair(s), {2} page(s) of {3}x{3}",
            Glyphs.GetSize(), Kerning.GetSize(), Sheet.Pages, Sheet.Side);

        return Exporter::Export(Metrics, Glyphs, Kerning, Pages, static_cast<UInt16>(Sheet.Side));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Bake(Text Source, Text Destination, ConstRef<Profile> Profile) const
    {
        Blob Input;

        if (Filesystem::Read(Source, Input) != Filesystem::Result::Success || Input == nullptr)
        {
            LOG_E("Font: failed to read '{0}'", Source);

            return false;
        }

        const Blob Output = Bake(Input, StrAfterLast(Source, '.'), Profile);

        if (Output == nullptr)
        {
            return false;
        }

        Filesystem::Ensure(Destination);

        if (Filesystem::Write(Destination, Output) != Filesystem::Result::Success)
        {
            LOG_E("Font: failed to write '{0}'", Destination);

            return false;
        }

        LOG_I("Font: '{0}' -> '{1}' ({2} bytes)", Source, Destination, Output.GetSize());
        return true;
    }
}