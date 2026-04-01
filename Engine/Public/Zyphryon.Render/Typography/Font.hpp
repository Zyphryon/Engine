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
    /// \brief Represents a font resource used for text rendering.
    class Font final : public Content::AbstractResource<Font>
    {
        friend class AbstractResource;

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
            Real32 Size              = 0.0f;

            /// Signed distance field spread value.
            Real32 Distance          = 0.0f;

            /// Distance from baseline to top of typical ascenders.
            Real32 Ascender          = 0.0f;

            /// Distance from baseline to bottom of typical descenders.
            Real32 Descender         = 0.0f;

            /// Vertical offset from baseline to top of underline.
            Real32 UnderlineOffset   = 0.0f;

            /// Total vertical space occupied by the underline.
            Real32 UnderlineSize     = 0.0f;

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

        /// \brief Loads the font data.
        ///
        /// \param Metrics  Information about the font’s sizing, spacing, and alignment.
        /// \param Glyphs   Table of glyphs indexed by UTF-32 codepoints.
        /// \param Kerning  Table of kerning adjustments indexed by character pairs.
        /// \param Material The material resource used for rendering the font.
        void Load(AnyRef<Metrics> Metrics, AnyRef<Glyphs> Glyphs, AnyRef<Kerning> Kerning, ConstTracker<Graphic::Material> Material);

        /// \brief Gets the metrics that describe the overall properties of the font.
        ///
        /// \return The metrics that describe this font.
        ZYPHRYON_INLINE ConstRef<Metrics> GetMetrics() const
        {
            return mMetrics;
        }

        /// \brief Calculates the line height for the font at a given size and padding.
        ///
        /// \param Size    The size at which the font is evaluated.
        /// \param Padding Additional spacing applied between lines, in pixels.
        /// \return The line height in pixels, which is the distance from one baseline to the
        ZYPHRYON_INLINE Real32 GetLineHeight(Real32 Size, Vector2 Padding) const
        {
            return (mMetrics.Ascender - mMetrics.Descender + Padding.GetY()) * Size;
        }

        /// \brief Retrieves the glyph data for a specific codepoint.
        ///
        /// \param Codepoint The UTF-32 codepoint to look up.
        /// \param Fallback  The fallback codepoint to use if the primary glyph is missing.
        /// \return Pointer to the glyph if found, nullptr if neither the requested glyph nor fallback glyph exists.
        ZYPHRYON_INLINE ConstPtr<Glyph> GetGlyph(UInt32 Codepoint, UInt32 Fallback = '?') const
        {
            if (const auto Iterator = mGlyphs.find(Codepoint); Iterator != mGlyphs.end())
            {
                return &Iterator->second;
            }
            return Fallback != 0 && Codepoint != Fallback ? GetGlyph(Fallback, 0) : nullptr;
        }

        /// \brief Retrieves the kerning adjustment between two characters.
        ///
        /// \param First  The character on the left side of the pair.
        /// \param Second The character on the right side of the pair.
        /// \return The recommended spacing adjustment in font units.
        ZYPHRYON_INLINE Real32 GetKerning(UInt32 First, UInt32 Second) const
        {
            const UInt64 Key = static_cast<UInt64>(First) << 32 | Second;

            if (const auto Iterator = mKerning.find(Key); Iterator != mKerning.end())
            {
                return Iterator->second;
            }
            return 0.0f;
        }

        /// \brief Gets the material resource used for rendering the font.
        ///
        /// \return The material of the font.
        ZYPHRYON_INLINE ConstTracker<Graphic::Material> GetMaterial() const
        {
            return mMaterial;
        }

        /// \brief Computes the dimensions of text at the specified font size.
        ///
        /// \param Content The text string encoded in UTF-8.
        /// \param Size    The size at which the font is evaluated.
        /// \param Padding Additional spacing applied between characters.
        /// \return The dimensions of the text in pixels.
        Vector2 Measure(ConstStr8 Content, Real32 Size, Vector2 Padding) const;

        /// \brief Calculates the bounding rectangle for a text string at the specified font size.
        ///
        /// \param Content The text string encoded in UTF-8.
        /// \param Size    The size at which the font is evaluated.
        /// \param Origin  The pivot point defining the text alignment.
        /// \param Padding Additional spacing applied between characters, in pixels.
        /// \return The bounding rectangle of the text in pixels, positioned according to the alignment.
        Rect Layout(ConstStr8 Content, Real32 Size, Pivot Origin, Vector2 Padding) const;

        /// \brief Calculates the origin point for the text based on the specified alignment.
        ///
        /// \param Content The text string encoded in UTF-8.
        /// \param Size    The size at which the font is evaluated.
        /// \param Origin  The pivot point defining the text alignment.
        /// \param Padding Additional spacing applied between characters, in pixels.
        /// \return The origin point, which can be used to position the text according.
        ZYPHRYON_INLINE Vector2 GetOrigin(ConstStr8 Content, Real32 Size, Pivot Origin, Vector2 Padding) const
        {
            const Vector2 Boundaries = Measure(Content, Size, Padding);

            return Vector2(Boundaries.GetX() * Origin.GetX(), Boundaries.GetY() * Origin.GetY());
        }

    private:

        /// \copydoc Resource::OnCreate
        Bool OnCreate(Ref<Service::Host> Host) override;

        /// \copydoc Resource::OnDelete
        void OnDelete(Ref<Service::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Metrics                    mMetrics;
        Glyphs                     mGlyphs;
        Kerning                    mKerning;
        Tracker<Graphic::Material> mMaterial;
    };
}