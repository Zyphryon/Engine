// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_FONT_INCLUDED
#define ZY_FONT_INCLUDED

#include "Embedded://Shader/Packing.hlsl"

/// \brief Represents the effect a run of text is drawn with, laid out exactly as `Render::FontEffect` is.
struct ZyFontEffect
{
    /// The color of the outset, packed as the four bytes \ref ZyUnpackTint reads.
    uint  OutsetTint;

    /// How far the outset is pushed out past the stroke, in pixels of screen coverage.
    float OutsetOffset;

    /// How wide the outset runs, as a share of the field's range, so it grows with the text.
    float OutsetWidth;

    /// How much wider still the outset runs, in pixels of screen coverage.
    float OutsetBias;

    /// How much of the outset's width is faded away, over zero through one.
    float OutsetBlur;

    /// How much the smooth field is read over the corner-true one, over zero through one.
    float InsetRoundness;

    /// The level of the field the stroke's edge stands at.
    float InsetThreshold;
};

/// \brief Reads the corner-true distance the three channels of a multi-channel field reconstruct.
///
/// \param Sample The three channels the field holds.
///
/// \return The distance the channels agree on.
float ZyFontMedian(float3 Sample)
{
    return max(min(Sample.r, Sample.g), min(max(Sample.r, Sample.g), Sample.b));
}

/// \brief Measures how many pixels of the screen one unit of the field spans where a glyph is read.
///
/// \note The measure is taken through screen derivatives, so this belongs to the fragment stage alone. It never
///       drops below one, so text smaller than the field can resolve still reads a pixel wide.
///
/// \param Texture The point in the atlas the glyph is read at.
/// \param Range   The width of the field's range, in atlas units along each axis.
///
/// \return The count of pixels one unit of the field covers.
float ZyFontSpread(float2 Texture, float2 Range)
{
    return max(dot(Range, 1.0 / fwidth(Texture)) * 0.5, 1.0);
}

/// \brief Shades one pixel of a glyph, laying the outset under the stroke the way the effect asks.
///
/// \note The color comes back premultiplied, so it blends with one against one minus source alpha.
///
/// \param Effect The effect the run is drawn with.
/// \param Sample The texel of the atlas, with the smooth field in alpha and the corner-true one in color.
/// \param Spread The count of pixels one unit of the field covers, as \ref ZyFontSpread measures it.
/// \param Tint   The color the stroke itself is drawn in.
///
/// \return The pixel's color, premultiplied by its coverage.
float4 ZyFontShade(ZyFontEffect Effect, float4 Sample, float Spread, float4 Tint)
{
    const float Smooth = Sample.a;
    const float Sharp  = ZyFontMedian(Sample.rgb);

    // Rounded art reads better off the smooth field and sharp art off the median, so the style mixes them.
    const float Distance = lerp(Sharp, Smooth, Effect.InsetRoundness) - Effect.InsetThreshold;

    const float Inner = saturate(Spread * Distance + 0.5 + Effect.OutsetOffset);
    const float Outer = saturate(Spread * (Distance + Effect.OutsetWidth) + 0.5 + Effect.OutsetOffset + Effect.OutsetBias);

    // The blur fades the outset over the far share of its width, measured back in the field's own units.
    const float BlurStart  = Effect.OutsetWidth + Effect.OutsetBias / Spread;
    const float BlurEnd    = BlurStart * (1.0 - Effect.OutsetBlur);
    const float BlurDepth  = Effect.InsetThreshold - Smooth - Effect.OutsetOffset / Spread;
    const float BlurFactor = lerp(1.0, 1.0 - smoothstep(BlurEnd, BlurStart, BlurDepth), step(0.0001, Effect.OutsetBlur));

    const float4 Outset = ZyUnpackTint(Effect.OutsetTint) * BlurFactor;

    return Tint * Inner + Outset * max(Outer - Inner, 0.0);
}

#endif // ZY_FONT_INCLUDED