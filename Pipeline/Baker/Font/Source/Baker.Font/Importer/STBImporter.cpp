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

#include "STBImporter.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#include <stb_truetype.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Font
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt16 ReadBig16(ConstPtr<Byte> Source)
    {
        return static_cast<UInt16>((static_cast<UInt8>(Source[0]) << 8) | static_cast<UInt8>(Source[1]));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 ReadBig32(ConstPtr<Byte> Source)
    {
        return (static_cast<UInt32>(ReadBig16(Source)) << 16) | ReadBig16(Source + 2);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 FindTable(ConstSpan<Byte> Source, UInt32 Base, UInt32 Tag)
    {
        // The table directory sits right after the twelve-byte header, sixteen bytes per record.
        if (Base + 12 > Source.GetSize())
        {
            return 0;
        }

        const UInt32 Count = ReadBig16(Source.GetData() + Base + 4);

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const UInt32 Record = Base + 12 + Index * 16;

            if (Record + 16 > Source.GetSize())
            {
                break;
            }

            if (ReadBig32(Source.GetData() + Record) == Tag)
            {
                return ReadBig32(Source.GetData() + Record + 8);
            }
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Typeface::Metrics ReadMetrics(Ref<stbtt_fontinfo> Info, ConstSpan<Byte> Source, UInt32 Offset)
    {
        const Real32 Normalize = stbtt_ScaleForMappingEmToPixels(AddressOf(Info), 1.0f);

        int Ascent  = 0;
        int Descent = 0;
        int LineGap = 0;
        stbtt_GetFontVMetrics(AddressOf(Info), &Ascent, &Descent, &LineGap);

        Typeface::Metrics Result;
        Result.Ascender  = static_cast<Real32>(Ascent)  * Normalize;
        Result.Descender = static_cast<Real32>(Descent) * Normalize;

        // The parser exposes no underline metrics, so the `post` table is read directly. A typeface that omits
        // it falls back to proportions that sit correctly under most Latin faces.
        Result.UnderlineOffset    = -0.1f;
        Result.UnderlineThickness =  0.05f;

        const UInt32 Post = FindTable(Source, Offset, ReadBig32(reinterpret_cast<ConstPtr<Byte>>("post")));

        if (Post != 0 && Post + 12 <= Source.GetSize())
        {
            const SInt16 Position  = static_cast<SInt16>(ReadBig16(Source.GetData() + Post + 8));
            const SInt16 Thickness = static_cast<SInt16>(ReadBig16(Source.GetData() + Post + 10));

            // A zero thickness means the typeface left the field unfilled rather than asking for a hairline.
            if (Thickness != 0)
            {
                Result.UnderlineOffset    = static_cast<Real32>(Position)  * Normalize;
                Result.UnderlineThickness = static_cast<Real32>(Thickness) * Normalize;
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void ReadOutline(Ref<stbtt_fontinfo> Info, int Index, Real32 Scale, Ref<Shape> Output)
    {
        Ptr<stbtt_vertex> Vertices = nullptr;
        const int         Count    = stbtt_GetGlyphShape(AddressOf(Info), Index, &Vertices);

        if (Count <= 0 || Vertices == nullptr)
        {
            return;
        }

        const auto At = [Scale](stbtt_vertex_type X, stbtt_vertex_type Y)
        {
            return Vector2(static_cast<Real32>(X) * Scale, static_cast<Real32>(Y) * Scale);
        };

        Contour Current;
        Vector2 Cursor;
        Vector2 Anchor;

        for (int Step = 0; Step < Count; ++Step)
        {
            ConstRef<stbtt_vertex> Vertex = Vertices[Step];
            const Vector2          Target = At(Vertex.x, Vertex.y);

            switch (Vertex.type)
            {
            case STBTT_vmove:
                if (!Current.IsEmpty())
                {
                    if (Cursor != Anchor)
                    {
                        Current.Append(Edge(Cursor, Anchor));
                    }
                    Output.Push(Move(Current));
                    Current.Clear();
                }
                Anchor = Target;
                break;
            case STBTT_vline:
                Current.Append(Edge(Cursor, Target));
                break;
            case STBTT_vcurve:
                Current.Append(Edge(Cursor, At(Vertex.cx, Vertex.cy), Target));
                break;
            case STBTT_vcubic:
                Current.Append(Edge(Cursor, At(Vertex.cx, Vertex.cy), At(Vertex.cx1, Vertex.cy1), Target));
                break;
            default:
                break;
            }
            Cursor = Target;
        }

        if (!Current.IsEmpty())
        {
            if (Cursor != Anchor)
            {
                Current.Append(Edge(Cursor, Anchor));
            }
            Output.Push(Move(Current));
        }
        stbtt_FreeShape(AddressOf(Info), Vertices);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Typeface STBImporter::Import(ConstSpan<Byte> Source, ConstRef<Profile> Profile) const
    {
        if (Source.GetSize() < 12)
        {
            LOG_E("Font: the source is too small to be a typeface");

            return Typeface();
        }

        const ConstPtr<unsigned char> Bytes = Source.GetData();

        // A collection holds several typefaces behind one file, and the requested one is found by index.
        if (const int Faces = stbtt_GetNumberOfFonts(Bytes); Faces > 0 && Profile.Face >= static_cast<UInt32>(Faces))
        {
            LOG_E("Font: the source holds {0} typeface(s), so index {1} does not exist", Faces, Profile.Face);

            return Typeface();
        }

        const int Offset = stbtt_GetFontOffsetForIndex(Bytes, static_cast<int>(Profile.Face));

        stbtt_fontinfo Info;

        if (Offset < 0 || !stbtt_InitFont(&Info, Bytes, Offset))
        {
            LOG_E("Font: the source is not a readable TrueType or OpenType typeface");

            return Typeface();
        }

        const Real32 Normalize = stbtt_ScaleForMappingEmToPixels(&Info, 1.0f);
        const Real32 Scale     = stbtt_ScaleForMappingEmToPixels(&Info, Profile.Size);

        // Only the codepoints the typeface actually carries are decoded. One it has no outline for is skipped
        // rather than emitted blank, so the engine's fallback glyph can take over at draw time.
        Sequence<Typeface::Glyph> Glyphs;
        Sequence<int>             Indices;

        // The spans may overlap or repeat, so a codepoint already decoded is skipped rather than baked twice.
        Table<UInt32, Bool> Seen;

        for (ConstRef<Interval> Span : Profile.Charset)
        {
            for (UInt32 Codepoint = Span.Minimum; Codepoint <= Span.Maximum; ++Codepoint)
            {
                const int Index = stbtt_FindGlyphIndex(&Info, static_cast<int>(Codepoint));

                if (Index == 0 || Seen.Contains(Codepoint))
                {
                    continue;
                }
                Seen.Assign(Codepoint, true);

                int Advance = 0;
                int Bearing = 0;
                stbtt_GetGlyphHMetrics(&Info, Index, &Advance, &Bearing);

                Ref<Typeface::Glyph> Glyph = Glyphs.Append();
                Glyph.Codepoint = Codepoint;
                Glyph.Advance   = static_cast<Real32>(Advance) * Normalize;

                ReadOutline(Info, Index, Scale, Glyph.Outline);

                Indices.Append(Index);
            }
        }

        // Kerning is read from `GPOS` when the typeface has one and from the legacy `kern` table otherwise.
        Typeface::Kerning Kerning;

        for (UInt First = 0; First < Indices.GetSize(); ++First)
        {
            for (UInt Second = 0; Second < Indices.GetSize(); ++Second)
            {
                if (const int Adjust = stbtt_GetGlyphKernAdvance(&Info, Indices[First], Indices[Second]); Adjust != 0)
                {
                    const UInt64 Key = static_cast<UInt64>(Glyphs[First].Codepoint) << 32 | Glyphs[Second].Codepoint;

                    Kerning.Assign(Key, static_cast<Real32>(Adjust) * Normalize);
                }
            }
        }

        Typeface::Metrics Metrics = ReadMetrics(Info, Source, static_cast<UInt32>(Offset));

        return Typeface(Move(Metrics), Move(Glyphs), Move(Kerning));
    }
}