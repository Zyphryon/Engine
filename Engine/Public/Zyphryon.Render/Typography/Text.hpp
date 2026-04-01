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
    /// \brief Represent rendering properties of a text.
    class Text final
    {
    public:

        /// \brief Constructor initializing default text properties.
        ZYPHRYON_INLINE Text()
            : mSize { 12.0f }
        {
        }

        /// \brief Constructs a new Text instance with specified properties.
        ///
        /// \param Font    The font used for rendering the text.
        /// \param Size    The size of the text.
        /// \param Tint    The tint color applied to the text.
        /// \param Padding The padding between characters in the text.
        ZYPHRYON_INLINE Text(ConstTracker<Font> Font, Real32 Size, IntColor8 Tint, Vector2 Padding)
            : mFont    { Font },
              mSize    { Size },
              mTint    { Tint },
              mPadding { Padding }
        {
        }

        /// \brief Sets the font used for rendering the text.
        ///
        /// \param Font The font to set.
        ZYPHRYON_INLINE void SetFont(ConstTracker<Font> Font)
        {
            mFont = Font;
        }

        /// \brief Gets the font used for rendering the text.
        ///
        /// \return The current font.
        ZYPHRYON_INLINE ConstTracker<Font> GetFont() const
        {
            return mFont;
        }

        /// \brief Sets the size of the text.
        ///
        /// \param Size The size to set.
        ZYPHRYON_INLINE void SetSize(Real32 Size)
        {
            mSize = Size;
        }

        /// \brief Gets the size of the text.
        ///
        /// \return The current text size.
        ZYPHRYON_INLINE Real32 GetSize() const
        {
            return mSize;
        }

        /// \brief Sets the tint color applied to the text.
        ///
        /// \param Tint The tint color  to set.
        ZYPHRYON_INLINE void SetTint(IntColor8 Tint)
        {
            mTint = Tint;
        }

        /// \brief Gets the tint color applied to the text.
        ///
        /// \return The current tint color .
        ZYPHRYON_INLINE IntColor8 GetTint() const
        {
            return mTint;
        }

        /// \brief Sets the padding between characters in the text.
        ///
        /// \param Padding The padding to set.
        ZYPHRYON_INLINE void SetPadding(Vector2 Padding)
        {
            mPadding = Padding;
        }

        /// \brief Gets the padding between characters in the text.
        ///
        /// \return The current character padding.
        ZYPHRYON_INLINE Vector2 GetPadding() const
        {
            return mPadding;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tracker<Font> mFont;
        Real32        mSize;
        IntColor8     mTint;
        Vector2       mPadding;
    };
}