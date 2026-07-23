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

        /// \brief Describes an individual glyph in the font.
        struct Glyph final
        {
            /// Bounds of the glyph in font units (relative to baseline).
            Rect   LocalBounds;

            /// Bounds of the glyph in texture coordinates (within the font atlas).
            Rect   AtlasBounds;

            /// Advance width (pen movement in X after drawing this glyph).
            Real32 Advance;
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

    public:

        /// \brief Constructs a font resource with the given content key.
        ///
        /// \param Key The URI key identifying this font resource.
        explicit Font(AnyRef<Content::Uri> Key);

        /// \brief Initializes the font with metrics, glyph data, kerning pairs, and a material.
        ///
        /// \param Metrics  The font metrics.
        /// \param Glyphs   The table of glyphs indexed by Unicode codepoint.
        /// \param Kerning  The table of kerning adjustments for character pairs.
        /// \param Material The material used for rendering the font atlas.
        void Setup(AnyRef<Metrics> Metrics, AnyRef<Glyphs> Glyphs, AnyRef<Kerning> Kerning, ConstRetainer<Graphic::Material> Material);

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
            return Fallback != 0 && Codepoint != Fallback ? GetGlyph(Fallback, 0) : nullptr;
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

        /// \brief Gets the material used to render this font's glyph atlas.
        ///
        /// \return The material containing the font atlas texture.
        ZY_INLINE ConstRetainer<Graphic::Material> GetMaterial() const
        {
            return mMaterial;
        }

        /// \brief Computes the bounding rectangle for the given text when rendered at the specified size.
        ///
        /// \param Content The text string to measure.
        /// \param Size    The font size in points.
        /// \param Spacing The horizontal (between characters) and vertical (between lines) spacing adjustments.
        /// \return The bounding rectangle in pixels.
        Rect Enclose(Text Content, Real32 Size, Vector2 Spacing = Vector2()) const;

    public:

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override;

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Metrics                     mMetrics;
        Glyphs                      mGlyphs;
        Kerning                     mKerning;
        Retainer<Graphic::Material> mMaterial;
    };
}