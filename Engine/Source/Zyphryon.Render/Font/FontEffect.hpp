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

namespace ZyRender
{
    /// \brief Represents the effect parameters for text rendering.
    class ZY_ALIGN(16) FontEffect final
    {
    public:

        /// \brief Constructs a text effect with default parameters.
        ZY_INLINE constexpr FontEffect()
            : mOutsetColor    { },
              mOutsetOffset   { 0.0f },
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
        /// \param OutsetOffset   The distance the outset is pushed past the stroke, in screen pixels.
        /// \param OutsetWidth    The width of the outset, as a share of the field range.
        /// \param OutsetBias     The absolute width bias of the text outset (added to relative width).
        /// \param OutsetBlur     The share of the outset that fades out, from `0` to `1`.
        /// \param InsetRoundness The blend from the sharp field at `0` to the smooth field at `1`.
        /// \param InsetThreshold The field level the stroke's edge sits at.
        ZY_INLINE constexpr FontEffect(
            IntColor8 OutsetColor,
            Real32    OutsetOffset,
            Real32    OutsetWidth,
            Real32    OutsetBias,
            Real32    OutsetBlur,
            Real32    InsetRoundness,
            Real32    InsetThreshold)
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
        /// \return The color the outset is drawn in.
        ZY_INLINE constexpr IntColor8 GetOutsetColor() const
        {
            return mOutsetColor;
        }

        /// \brief Gets the outset offset value.
        ///
        /// \return The distance the outset is pushed past the stroke, in screen pixels.
        ZY_INLINE constexpr Real32 GetOutsetOffset() const
        {
            return mOutsetOffset;
        }

        /// \brief Gets the relative outset width value.
        ///
        /// \return The outset width, as a share of the field range.
        ZY_INLINE constexpr Real32 GetOutsetWidthRelative() const
        {
            return mOutsetWidth;
        }

        /// \brief Gets the absolute outset width value.
        ///
        /// \return The extra outset width, in screen pixels.
        ZY_INLINE constexpr Real32 GetOutsetWidthAbsolute() const
        {
            return mOutsetBias;
        }

        /// \brief Gets the outset blur value.
        ///
        /// \return The share of the outset that fades out, from `0` to `1`.
        ZY_INLINE constexpr Real32 GetOutsetBlur() const
        {
            return mOutsetBlur;
        }

        /// \brief Gets the inset roundness value.
        ///
        /// \return The blend from the sharp field at `0` to the smooth field at `1`.
        ZY_INLINE constexpr Real32 GetInsetRoundness() const
        {
            return mInsetRoundness;
        }

        /// \brief Gets the inset threshold value.
        ///
        /// \return The field level the stroke's edge sits at.
        ZY_INLINE constexpr Real32 GetInsetThreshold() const
        {
            return mInsetThreshold;
        }

    public:

        /// \brief Creates an outline text effect with specified parameters.
        ///
        /// \param Width    The width of the outline, as a share of the field range.
        /// \param Softness The share of the outline that fades out, from `0` to `1`.
        /// \param Tint     The color tint of the outline.
        /// \return An effect that outlines the stroke.
        ZY_INLINE static constexpr FontEffect Outline(Real32 Width, Real32 Softness, IntColor8 Tint)
        {
            return FontEffect(Tint, 0.0f, Width, 0.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a shadow text effect with specified parameters.
        ///
        /// \param Softness The share of the shadow that fades out, from `0` to `1`.
        /// \param Tint     The color tint of the shadow.
        /// \return An effect that casts a shadow behind the stroke.
        ZY_INLINE static constexpr FontEffect Shadow(Real32 Softness, IntColor8 Tint)
        {
            return FontEffect(Tint, -1.0f / 2.0f, 1.0f / 4.0f, 2.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a bold text effect with specified thickness.
        ///
        /// \param Thickness The thickness of the bold, from `0.02` (slight) to `0.20` (heaviest).
        /// \return An effect that thickens the stroke.
        ZY_INLINE static constexpr FontEffect Bold(Real32 Thickness)
        {
            return FontEffect(IntColor8::Transparent(), 0.0f, 0.0f, Thickness, 0.0f, 1.0f, 0.5f - Thickness);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        IntColor8 mOutsetColor;
        Real32    mOutsetOffset;
        Real32    mOutsetWidth;
        Real32    mOutsetBias;
        Real32    mOutsetBlur;
        Real32    mInsetRoundness;
        Real32    mInsetThreshold;
    };
}