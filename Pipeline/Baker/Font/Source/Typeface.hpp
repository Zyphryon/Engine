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

#include "Shape.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Font
{
    /// \brief A decoded typeface, handed from an importer to the generator.
    class Typeface final
    {
    public:

        /// \brief The typeface-wide metrics, in em units.
        struct Metrics final
        {
            /// The distance from the baseline to the top of typical ascenders.
            Real32 Ascender           = 0.0f;

            /// The distance from the baseline to the bottom of typical descenders, which is negative.
            Real32 Descender          = 0.0f;

            /// The vertical offset from the baseline to the top of the underline.
            Real32 UnderlineOffset    = 0.0f;

            /// The visual thickness of the underline stroke.
            Real32 UnderlineThickness = 0.0f;
        };

        /// \brief One decoded glyph.
        struct Glyph final
        {
            /// The Unicode codepoint the glyph draws.
            UInt32 Codepoint = 0;

            /// The distance the pen advances after drawing the glyph, in em units.
            Real32 Advance   = 0.0f;

            /// The glyph's contours, in pixels, with the origin on the baseline and Y pointing up.
            Shape  Outline;
        };

        /// \brief The kerning table, keyed by both codepoints packed into one integer.
        using Kerning = Table<UInt64, Real32>;

    public:

        /// \brief Constructs an empty typeface that carries no glyph.
        ZY_INLINE Typeface() = default;

        /// \brief Constructs a typeface from decoded data.
        ///
        /// \param Metrics The typeface-wide metrics.
        /// \param Glyphs  The decoded glyphs, in ascending codepoint order.
        /// \param Kerning The kerning pairs the typeface defines between those glyphs.
        ZY_INLINE Typeface(AnyRef<Metrics> Metrics, AnyRef<Sequence<Glyph>> Glyphs, AnyRef<Kerning> Kerning)
            : mMetrics { Move(Metrics) },
              mGlyphs  { Move(Glyphs) },
              mKerning { Move(Kerning) }
        {
        }

        /// \brief Gets the typeface-wide metrics.
        ///
        /// \return The metrics, in em units.
        ZY_INLINE ConstRef<Metrics> GetMetrics() const
        {
            return mMetrics;
        }

        /// \brief Gets the decoded glyphs.
        ///
        /// \return The glyphs.
        ZY_INLINE ConstSpan<Glyph> GetGlyphs() const
        {
            return mGlyphs;
        }

        /// \brief Gets the kerning pairs.
        ///
        /// \return The kerning table.
        ZY_INLINE ConstRef<Kerning> GetKerning() const
        {
            return mKerning;
        }

        /// \brief Checks whether the typeface carries any glyph at all.
        ///
        /// \return `true` when nothing was decoded, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mGlyphs.IsEmpty();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Metrics         mMetrics;
        Sequence<Glyph> mGlyphs;
        Kerning         mKerning;
    };
}