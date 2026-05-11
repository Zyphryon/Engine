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
        : AbstractResource(Move(Key))
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Font::Load(AnyRef<Metrics> Metrics, AnyRef<Glyphs> Glyphs, AnyRef<Kerning> Kerning, ConstTracker<Graphic::Material> Material)
    {
        mMetrics  = Move(Metrics);
        mGlyphs   = Move(Glyphs);
        mKerning  = Move(Kerning);
        mMaterial = Material;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector2 Font::Measure(ConstStr8 Content, Real32 Size, Vector2 Padding) const
    {
        const Real32 LineHeight = (mMetrics.Ascender - mMetrics.Descender) + Padding.GetY();

        Real32 CurrentX = 0.0f;
        Real32 MaximumX = 0.0f;
        Real32 MaximumY = LineHeight;
        UInt32 Previous = 0;

        // Iterate through each UTF-8 codepoint in the content string and calculate the dimensions of the text.
        IterateUTF8(Content, [&](UInt32 Codepoint)
        {
            switch (Codepoint)
            {
            case '\r':
                CurrentX = 0.0f;
                Previous = 0;
                break;
            case '\n':
                MaximumY += LineHeight;
                Previous = 0;
                break;
            default:
            {
                if (const ConstPtr<Glyph> Glyph = GetGlyph(Codepoint); Glyph)
                {
                    if (Previous)
                    {
                        CurrentX += GetKerning(Previous, Codepoint) + Padding.GetX();
                    }
                    CurrentX += Glyph->Advance;
                    MaximumX = Max(MaximumX, CurrentX);
                    Previous = Codepoint;
                }
                break;
            }
            }
        });
        return Vector2(MaximumX, MaximumY) * Size;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Rect Font::Enclose(ConstStr8 Content, Real32 Size, Vector2 Padding) const
    {
        const Real32 LineHeight = (mMetrics.Ascender - mMetrics.Descender) + Padding.GetY();

        Real32 LineY    = 0.0f;
        Real32 MinimumX = 0.0f;
        Real32 MaximumX = 0.0f;
        Real32 MinimumY = 0.0f;
        Real32 MaximumY = 0.0f;
        Real32 CurrentX = 0.0f;
        Real32 CurrentY = 0.0f;

        UInt32 Previous = 0;

        // Iterate through each UTF-8 codepoint in the content string and calculate the bounding rectangle.
        IterateUTF8(Content, [&](UInt32 Codepoint)
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
                        CurrentX += GetKerning(Previous, Codepoint) + Padding.GetX();
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

    Bool Font::OnCreate(Ref<Service::Host> Host)
    {
        if (mMaterial && mMaterial->HasPolicy(Policy::Exclusive))
        {
            return mMaterial->Create(Host);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Font::OnDelete(Ref<Service::Host> Host)
    {
        if (mMaterial && mMaterial->HasPolicy(Policy::Exclusive))
        {
            mMaterial->Delete(Host);
        }
    }
}