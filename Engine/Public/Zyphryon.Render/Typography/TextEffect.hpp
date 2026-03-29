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
    /// \brief Represents text effect parameters for text rendering.
    class ZYPHRYON_ALIGN(16) TextEffect final
    {
    public:

        /// \brief Constructs a text effect with default parameters.
        ZYPHRYON_INLINE constexpr TextEffect()
            : mOutsetOffset        { 0.0f },
              mOutsetWidthRelative { 0.0f },
              mOutsetWidthAbsolute { 0.0f },
              mOutsetBlur          { 0.0f },
              mInsetRoundness      { 1.0f },
              mInsetThreshold      { 0.5f }
        {
        }

        /// \brief Constructs a text effect with specified parameters.
        ///
        /// \param OutsetColor          The color of the text outset.
        /// \param OutsetOffset         The offset of the text outset.
        /// \param OutsetWidthRelative  The relative width of the text outset.
        /// \param OutsetWidthAbsolute  The absolute width of the text outset.
        /// \param OutsetBlur           The blur amount of the text outset.
        /// \param InsetRoundness       The roundness of the text inset.
        /// \param InsetThreshold       The threshold for the text inset.
        ZYPHRYON_INLINE constexpr TextEffect(
            Color  OutsetColor,
            Real32 OutsetOffset,
            Real32 OutsetWidthRelative,
            Real32 OutsetWidthAbsolute,
            Real32 OutsetBlur,
            Real32 InsetRoundness,
            Real32 InsetThreshold)
            : mOutsetColor         { OutsetColor },
              mOutsetOffset        { OutsetOffset },
              mOutsetWidthRelative { OutsetWidthRelative },
              mOutsetWidthAbsolute { OutsetWidthAbsolute },
              mOutsetBlur          { OutsetBlur },
              mInsetRoundness      { InsetRoundness },
              mInsetThreshold      { InsetThreshold }
        {
        }

        /// \brief Gets the outset color value.
        ///
        /// \return The outset color.
        ZYPHRYON_INLINE constexpr Color GetOutsetColor() const
        {
            return mOutsetColor;
        }

        /// \brief Gets the outset offset value.
        ///
        /// \return The outset offset.
        ZYPHRYON_INLINE constexpr Real32 GetOutsetOffset() const
        {
            return mOutsetOffset;
        }

        /// \brief Gets the relative outset width value.
        ///
        /// \return The relative outset width.
        ZYPHRYON_INLINE constexpr Real32 GetOutsetWidthRelative() const
        {
            return mOutsetWidthRelative;
        }

        /// \brief Gets the absolute outset width value.
        ///
        /// \return The absolute outset width.
        ZYPHRYON_INLINE constexpr Real32 GetOutsetWidthAbsolute() const
        {
            return mOutsetWidthAbsolute;
        }

        /// \brief Gets the outset blur value.
        ///
        /// \return The outset blur.
        ZYPHRYON_INLINE constexpr Real32 GetOutsetBlur() const
        {
            return mOutsetBlur;
        }

        /// \brief Gets the inset roundness value.
        ///
        /// \return The inset roundness.
        ZYPHRYON_INLINE constexpr Real32 GetInsetRoundness() const
        {
            return mInsetRoundness;
        }

        /// \brief Gets the inset threshold value.
        ///
        /// \return The inset threshold.
        ZYPHRYON_INLINE constexpr Real32 GetInsetThreshold() const
        {
            return mInsetThreshold;
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mOutsetColor);
            Archive.SerializeObject(mOutsetOffset);
            Archive.SerializeObject(mOutsetWidthRelative);
            Archive.SerializeObject(mOutsetWidthAbsolute);
            Archive.SerializeObject(mOutsetBlur);
            Archive.SerializeObject(mInsetRoundness);
            Archive.SerializeObject(mInsetThreshold);
        }

    public:

        /// \brief Creates an outline text effect with specified parameters.
        ///
        /// \param Width    The width of the outline.
        /// \param Softness The softness of the outline.
        /// \param Tint     The color tint of the outline.
        ZYPHRYON_INLINE constexpr static TextEffect Outline(Real32 Width, Real32 Softness, ConstRef<Color> Tint)
        {
            return TextEffect(Tint, 0.0f, Width, 0.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a shadow text effect with specified parameters.
        ///
        /// \param Width    The width of the shadow.
        /// \param Softness The softness of the shadow.
        /// \param Tint     The color tint of the shadow.
        ZYPHRYON_INLINE constexpr static TextEffect Shadow(Real32 Width, Real32 Softness, ConstRef<Color> Tint)
        {
            return TextEffect(Tint, 0.25f, Width, 0.0f, Softness, 1.0f, 0.5f);
        }

        /// \brief Creates a bold text effect with specified thickness.
        ///
        /// \note Recommended values:
        ///       0.02 = Slight bold
        ///       0.05 = Medium bold
        ///       0.10 = Heavy bold
        ///       0.15 = Extreme bold
        ///       0.20 = Maximum bold (may cause artifacts)
        ///
        /// \param Thickness The thickness of the bold effect.
        ZYPHRYON_INLINE constexpr static TextEffect Bold(Real32 Thickness)
        {
            return TextEffect(Color(0, 0, 0, 0), 0.0f, 0.0f, Thickness, 0.0f, 1.0f, 0.5f - Thickness);
        }

        ZYPHRYON_INLINE constexpr static TextEffect Glow(Real32 Size, Real32 Softness, ConstRef<Color> Tint)
        {
            return TextEffect(
                Tint,     // glow color
                0.0f,     // no offset (centered)
                Size,     // outward expansion
                0.0f,
                Softness, // blur = softness
                1.0f,     // keep sharp glyph base
                0.5f      // normal threshold
            );
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Color  mOutsetColor;
        Real32 mOutsetOffset;
        Real32 mOutsetWidthRelative;
        Real32 mOutsetWidthAbsolute;
        Real32 mOutsetBlur;
        Real32 mInsetRoundness;
        Real32 mInsetThreshold;
    };
}