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

#include "Zyphryon.Math/Color.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Defines the effect parameters for text rendering.
    class ZY_ALIGN(16) FontEffect final
    {
    public:

        /// \brief Constructs a text effect with default parameters.
        ZY_INLINE constexpr FontEffect()
            : mOutsetOffset   { 0.0f },
              mOutsetWidth    { 0.0f },
              mOutsetBias     { 0.0f },
              mOutsetBlur     { 0.0f },
              mInsetRoundness { 1.0f },
              mInsetThreshold { 0.5f }
        {
        }

        /// \brief Constructs a text effect with specified parameters.
        ///
        /// \param OutsetColor    The color of the text outset.
        /// \param OutsetOffset   The offset of the text outset.
        /// \param OutsetWidth    The relative width of the text outset (relative to font size).
        /// \param OutsetBias     The absolute width bias of the text outset (added to relative width).
        /// \param OutsetBlur     The blur amount of the text outset.
        /// \param InsetRoundness The roundness of the text inset.
        /// \param InsetThreshold The threshold for the text inset.
        ZY_INLINE constexpr FontEffect(
            Color  OutsetColor,
            Real32 OutsetOffset,
            Real32 OutsetWidth,
            Real32 OutsetBias,
            Real32 OutsetBlur,
            Real32 InsetRoundness,
            Real32 InsetThreshold)
            : mOutsetColor    { OutsetColor },
              mOutsetOffset   { OutsetOffset },
              mOutsetWidth    { OutsetWidth },
              mOutsetBias     { OutsetBias },
              mOutsetBlur     { OutsetBlur },
              mInsetRoundness { InsetRoundness },
              mInsetThreshold { InsetThreshold }
        {
        }

        /// \brief Gets the outset color value.
        ///
        /// \return The outset color.
        ZY_INLINE constexpr Color GetOutsetColor() const
        {
            return mOutsetColor;
        }

        /// \brief Gets the outset offset value.
        ///
        /// \return The outset offset.
        ZY_INLINE constexpr Real32 GetOutsetOffset() const
        {
            return mOutsetOffset;
        }

        /// \brief Gets the relative outset width value.
        ///
        /// \return The relative outset width.
        ZY_INLINE constexpr Real32 GetOutsetWidthRelative() const
        {
            return mOutsetWidth;
        }

        /// \brief Gets the absolute outset width value.
        ///
        /// \return The absolute outset width.
        ZY_INLINE constexpr Real32 GetOutsetWidthAbsolute() const
        {
            return mOutsetBias;
        }

        /// \brief Gets the outset blur value.
        ///
        /// \return The outset blur.
        ZY_INLINE constexpr Real32 GetOutsetBlur() const
        {
            return mOutsetBlur;
        }

        /// \brief Gets the inset roundness value.
        ///
        /// \return The inset roundness.
        ZY_INLINE constexpr Real32 GetInsetRoundness() const
        {
            return mInsetRoundness;
        }

        /// \brief Gets the inset threshold value.
        ///
        /// \return The inset threshold.
        ZY_INLINE constexpr Real32 GetInsetThreshold() const
        {
            return mInsetThreshold;
        }

    public:

        /// \brief Creates an outline text effect with specified parameters.
        ///
        /// \param Width    The width of the outline.
        /// \param Softness The softness of the outline.
        /// \param Tint     The color tint of the outline.
        /// \return A text effect with the specified parameters.
        ZY_INLINE constexpr static FontEffect Outline(Real32 Width, Real32 Softness, Color Tint)
        {
            return FontEffect(Tint, 0.0f, Width, 0.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a shadow text effect with specified parameters.
        ///
        /// \param Softness The softness of the shadow.
        /// \param Tint     The color tint of the shadow.
        /// \return A text effect with the specified parameters.
        ZY_INLINE constexpr static FontEffect Shadow(Real32 Softness, Color Tint)
        {
            return FontEffect(Tint, -1.0f / 2.0f, 1.0f / 4.0f, 2.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a bold text effect with specified thickness.
        ///
        /// 0.02 = Slight bold
        /// 0.05 = Medium bold
        /// 0.10 = Heavy bold
        /// 0.15 = Extreme bold
        /// 0.20 = Maximum bold
        ///
        /// \param Thickness The thickness of the bold effect.
        /// \return A text effect with the specified parameters.
        ZY_INLINE constexpr static FontEffect Bold(Real32 Thickness)
        {
            return FontEffect(Color::Transparent(), 0.0f, 0.0f, Thickness, 0.0f, 1.0f, 0.5f - Thickness);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Color  mOutsetColor;
        Real32 mOutsetOffset;
        Real32 mOutsetWidth;
        Real32 mOutsetBias;
        Real32 mOutsetBlur;
        Real32 mInsetRoundness;
        Real32 mInsetThreshold;
    };
}