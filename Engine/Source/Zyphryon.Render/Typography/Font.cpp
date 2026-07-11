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

#include "Font.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Font::Font(AnyRef<Content::Uri> Key)
        : AbstractResource { Move(Key) }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Font::Setup(AnyRef<Metrics> Metrics, AnyRef<Glyphs> Glyphs, AnyRef<Kerning> Kerning, ConstRetainer<Graphic::Material> Material)
    {
        SetFootprint(Glyphs.GetSize() * sizeof(Glyph) + Kerning.GetSize() * sizeof(UInt64));

        mMetrics  = Move(Metrics);
        mGlyphs   = Move(Glyphs);
        mKerning  = Move(Kerning);
        mMaterial = Material;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Rect Font::Enclose(Text Content, Real32 Size) const
    {
        const Real32 LineHeight = (mMetrics.Ascender - mMetrics.Descender);

        Real32 LineY    = 0.0f;
        Real32 MinimumX = 0.0f;
        Real32 MaximumX = 0.0f;
        Real32 MinimumY = 0.0f;
        Real32 MaximumY = 0.0f;
        Real32 CurrentX = 0.0f;
        Real32 CurrentY = 0.0f;

        UInt32 Previous = 0;

        // Iterate through each UTF-8 codepoint in the content string and calculate the bounding rectangle.
        StrIterateUTF8(Content, [&](UInt32 Codepoint)
        {
            switch (Codepoint)
            {
            case '\r':
                MaximumX = Max(MaximumX, CurrentX);
                CurrentX = 0.0f;
                Previous = 0;
                break;
            case '\n':
                LineY   -= LineHeight;
                CurrentY = 0.0f;
                Previous = 0;
                break;
            default:
            {
                if (const ConstPtr<Glyph> Glyph = GetGlyph(Codepoint); Glyph)
                {
                    if (Previous)
                    {
                        CurrentX += GetKerning(Previous, Codepoint);
                    }
                    else
                    {
                        MinimumX = Min(MinimumX, -Glyph->LocalBounds.GetMinimumX());
                    }

                    CurrentX += Glyph->Advance;

                    if (LineY == 0.0f)
                    {
                        MaximumY = Max(MaximumY, Glyph->LocalBounds.GetMaximumY());
                    }

                    CurrentY = Min(CurrentY, LineY + Glyph->LocalBounds.GetMinimumY());
                }
                break;
            }
            }
        });

        MaximumX = Max(MaximumX, CurrentX);
        MinimumY = Min(MinimumY, CurrentY);

        return Rect(MinimumX, MinimumY, MaximumX, MaximumY) * Size;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Font::OnCreate(Ref<Engine::Subsystem::Host> Host)
    {
        if (mMaterial && mMaterial->GetPolicy() == Policy::Exclusive)
        {
            return mMaterial->OnCreate(Host);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Font::OnDelete(Ref<Engine::Subsystem::Host> Host)
    {
        if (mMaterial && mMaterial->GetPolicy() == Policy::Exclusive)
        {
            mMaterial->OnDelete(Host);
        }
    }
}