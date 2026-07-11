// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Font.hpp"
#include "Zyphryon.Math/Color.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Encapsulates text rendering properties including font, size, tint, and character spacing.
    class TextStyle final
    {
    public:

        /// \brief Constructs a text style with default properties.
        ZY_INLINE TextStyle()
            : mSize { 12.0f }
        {
        }

        /// \brief Constructs a text style with the specified font, size, tint, and spacing.
        ///
        /// \param Font    The font resource to use for text rendering.
        /// \param Size    The font size in points.
        /// \param Tint    The color tint applied to the rendered text.
        /// \param Spacing The horizontal and vertical spacing adjustments between characters.
        ZY_INLINE TextStyle(ConstRetainer<Font> Font, Real32 Size, IntColor8 Tint, Vector2 Spacing)
            : mFont    { Font },
              mSize    { Size },
              mTint    { Tint },
              mSpacing { Spacing }
        {
        }

        /// \brief Sets the font resource to use for text rendering.
        ///
        /// \param Font The font resource.
        ZY_INLINE void SetFont(ConstRetainer<Font> Font)
        {
            mFont = Font;
        }

        /// \brief Gets the font resource used for text rendering.
        ///
        /// \return The font resource.
        ZY_INLINE ConstRetainer<Font> GetFont() const
        {
            return mFont;
        }

        /// \brief Sets the font size in points.
        ///
        /// \param Size The font size.
        ZY_INLINE void SetSize(Real32 Size)
        {
            mSize = Size;
        }

        /// \brief Gets the font size in points.
        ///
        /// \return The font size.
        ZY_INLINE Real32 GetSize() const
        {
            return mSize;
        }

        /// \brief Sets the color tint applied to rendered text.
        ///
        /// \param Tint The color tint.
        ZY_INLINE void SetTint(IntColor8 Tint)
        {
            mTint = Tint;
        }

        /// \brief Gets the color tint applied to rendered text.
        ///
        /// \return The color tint.
        ZY_INLINE IntColor8 GetTint() const
        {
            return mTint;
        }

        /// \brief Sets the horizontal and vertical spacing adjustments between characters.
        ///
        /// \param Spacing The spacing vector where X controls horizontal offset and Y controls vertical offset.
        ZY_INLINE void SetSpacing(Vector2 Spacing)
        {
            mSpacing = Spacing;
        }

        /// \brief Gets the horizontal and vertical spacing adjustments between characters.
        ///
        /// \return The spacing vector.
        ZY_INLINE Vector2 GetSpacing() const
        {
            return mSpacing;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Font> mFont;
        Real32         mSize;
        IntColor8      mTint;
        Vector2        mSpacing;
    };
}