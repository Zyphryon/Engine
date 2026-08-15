// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A font resource containing glyph metrics, kerning data, and a material for rendering text.
    class Font final : public Content::AbstractResource<Font>
    {
    public:

        /// \brief Describes where the layout puts one glyph of a run.
        struct Placement final
        {
            /// The glyph's bounds in font units, offset by the pen and relative to the run's origin.
            Rect   Local;

            /// The glyph's bounds in texture coordinates, within its atlas page.
            Rect   Atlas;

            /// The atlas page the glyph was baked onto, which selects the material to draw it with.
            UInt32 Page;
        };

        /// \brief Describes an individual glyph in the font.
        struct Glyph final
        {
            /// The glyph's bounds in font units, relative to the baseline.
            Rect   Local;

            /// The glyph's bounds in texture coordinates, within its atlas page.
            Rect   Atlas;

            /// Advance width (pen movement in X after drawing this glyph).
            Real32 Advance;

            /// The atlas page the glyph was baked onto, which selects the material to draw it with.
            UInt32 Page;
        };

        /// \brief Metrics describing the font typeface.
        struct Metrics final
        {
            /// Nominal font size used when generating glyphs.
            Real32 Size               = 0.0f;

            /// Signed distance field spread value.
            Real32 Distance           = 0.0f;

            /// Distance from baseline to top of typical ascenders.
            Real32 Ascender           = 0.0f;

            /// Distance from baseline to bottom of typical descenders.
            Real32 Descender          = 0.0f;

            /// Vertical offset from baseline to top of underline.
            Real32 UnderlineOffset    = 0.0f;

            /// Total vertical space occupied by the underline.
            Real32 UnderlineSize      = 0.0f;

            /// Visual thickness of the underline stroke.
            Real32 UnderlineThickness = 0.0f;
        };

        /// \brief Table mapping codepoints to glyphs.
        using Glyphs  = Table<UInt32, Glyph>;

        /// \brief Table mapping character pairs to kerning adjustments.
        using Kerning = Table<UInt64, Real32>;

        /// \brief Sequence holding one material per atlas page, indexed by \c Glyph::Page.
        using Atlases = Sequence<Retainer<Graphic::Material>>;

    public:

        /// \brief Constructs a font resource with the given content key.
        ///
        /// \param Key The URI key identifying this font resource.
        explicit Font(AnyRef<Content::Uri> Key);

        /// \brief Initializes the font with metrics, glyph data, kerning pairs, and a material.
        ///
        /// \param Metrics The font metrics.
        /// \param Glyphs  The table of glyphs indexed by Unicode codepoint.
        /// \param Kerning The table of kerning adjustments for character pairs.
        /// \param Atlases One material per atlas page, indexed by \c Glyph::Page.
        void Setup(AnyRef<Metrics> Metrics, AnyRef<Glyphs> Glyphs, AnyRef<Kerning> Kerning, AnyRef<Atlases> Atlases);

        /// \brief Gets the font metrics describing ascent, descent, and underline properties.
        ///
        /// \return The font metrics structure.
        ZY_INLINE ConstRef<Metrics> GetMetrics() const
        {
            return mMetrics;
        }

        /// \brief Gets the line height for the specified font size.
        ///
        /// \param Size The desired font size in points.
        /// \return The vertical spacing between baselines in pixels.
        ZY_INLINE Real32 GetLineHeight(Real32 Size) const
        {
            return (mMetrics.Ascender - mMetrics.Descender) * Size;
        }

        /// \brief Gets the glyph data for a specific Unicode codepoint.
        ///
        /// \param Codepoint The Unicode codepoint to look up.
        /// \param Fallback  The codepoint to use if the requested glyph is not available.
        /// \return The glyph data, or nullptr if neither the codepoint nor fallback exist.
        ZY_INLINE ConstPtr<Glyph> GetGlyph(UInt32 Codepoint, UInt32 Fallback = '?') const
        {
            if (const ConstPtr<Glyph> Result = mGlyphs.Find(Codepoint))
            {
                return Result;
            }
            return Fallback != 0 && Codepoint != Fallback ? mGlyphs.Find(Fallback) : nullptr;
        }

        /// \brief Gets the kerning adjustment between two consecutive glyphs.
        ///
        /// \param First  The Unicode codepoint of the first character.
        /// \param Second The Unicode codepoint of the second character.
        /// \return The horizontal kerning offset in font units, or zero if no kerning pair exists.
        ZY_INLINE Real32 GetKerning(UInt32 First, UInt32 Second) const
        {
            return mKerning.FindOrDefault(static_cast<UInt64>(First) << 32 | Second);
        }

        /// \brief Gets the material that renders one of this font's atlas pages.
        ///
        /// \param Page The atlas page, as carried by \c Glyph::Page.
        /// \return The material containing that page's atlas texture.
        ZY_INLINE ConstRetainer<Graphic::Material> GetMaterial(UInt32 Page = 0) const
        {
            ZY_ASSERT(Page < mAtlases.GetSize(), "Font atlas page is out of bounds");

            return mAtlases[Page];
        }

        /// \brief Lays the text out, handing back each glyph where the layout puts it.
        ///
        /// \note Placements come back in font units, so a caller applies the size it sets the text at itself.
        ///
        /// \param Content  The text string to lay out.
        /// \param Spacing  The horizontal (between characters) and vertical (between lines) spacing adjustments.
        /// \param Callback A callable invoked with the \ref Placement of each glyph, in reading order.
        template<typename Callable>
        void Shape(Text Content, Vector2 Spacing, AnyRef<Callable> Callback) const
        {
            const Real32 LineHeight = (mMetrics.Ascender - mMetrics.Descender) + Spacing.GetY();

            Real32 PenX     = 0.0f;
            Real32 LineY    = 0.0f;
            UInt32 Previous = 0;

            StrIterateUTF8(Content, [&](UInt32 Codepoint)
            {
                switch (Codepoint)
                {
                case '\r':
                    PenX     = 0.0f;
                    Previous = 0;
                    break;
                case '\n':
                    PenX     = 0.0f;
                    LineY   -= LineHeight;
                    Previous = 0;
                    break;
                default:
                {
                    if (const ConstPtr<Glyph> Glyph = GetGlyph(Codepoint); Glyph)
                    {
                        if (Previous)
                        {
                            PenX += GetKerning(Previous, Codepoint) + Spacing.GetX();
                        }

                        Callback(Placement {
                            .Local = Glyph->Local + Vector2(PenX, LineY),
                            .Atlas = Glyph->Atlas,
                            .Page        = Glyph->Page
                        });

                        PenX    += Glyph->Advance;
                        Previous = Codepoint;
                    }
                    break;
                }
                }
            });
        }

        /// \brief Computes the bounding rectangle for the given text when rendered at the specified size.
        ///
        /// \param Content The text string to measure.
        /// \param Size    The font size in points.
        /// \param Spacing The horizontal (between characters) and vertical (between lines) spacing adjustments.
        /// \return The bounding rectangle in pixels.
        Rect Enclose(Text Content, Real32 Size, Vector2 Spacing = Vector2()) const
        {
            Real32 MinimumX = 0.0f;
            Real32 MaximumX = 0.0f;
            Real32 MinimumY = 0.0f;
            Real32 MaximumY = 0.0f;

            // The box always holds the origin, so an empty string still measures as a point rather than as nothing.
            Shape(Content, Spacing, [&](ConstRef<Placement> Placement)
            {
                MinimumX = Min(MinimumX, Placement.Local.GetMinimumX());
                MaximumX = Max(MaximumX, Placement.Local.GetMaximumX());
                MinimumY = Min(MinimumY, Placement.Local.GetMinimumY());
                MaximumY = Max(MaximumY, Placement.Local.GetMaximumY());
            });
            return Rect(MinimumX, MinimumY, MaximumX, MaximumY) * Size;
        }

    public:

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override;

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Metrics mMetrics;
        Glyphs  mGlyphs;
        Kerning mKerning;
        Atlases mAtlases;
    };
}