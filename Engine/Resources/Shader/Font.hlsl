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

/// The effect a run of text is drawn with, laid out to match `Render::FontEffect`.
struct ZyFontEffect
{
    /// The outset color, packed as ZyUnpackTint reads it.
    uint  OutsetTint;

    /// How far the outset is pushed past the stroke, in screen pixels.
    float OutsetOffset;

    /// The outset width as a share of the field range, so it grows with the text.
    float OutsetWidth;

    /// Extra outset width, in screen pixels.
    float OutsetBias;

    /// How much of the outset fades out, from 0 to 1.
    float OutsetBlur;

    /// The blend from the sharp field at 0 to the smooth field at 1.
    float InsetRoundness;

    /// The field level the stroke's edge sits at.
    float InsetThreshold;
};

/// The median of the three channels, which gives the corner-true distance.
float ZyFontMedian(float3 Sample)
{
    return max(min(Sample.r, Sample.g), min(max(Sample.r, Sample.g), Sample.b));
}

/// Screen pixels one field unit spans here, at least one; fragment stage only.
float ZyFontSpread(float2 Texture, float2 Range)
{
    return max(dot(Range, 1.0 / fwidth(Texture)) * 0.5, 1.0);
}

/// Shades one glyph pixel with the outset under the stroke, as premultiplied color.
float4 ZyFontShade(ZyFontEffect Effect, float4 Sample, float Spread, float4 Tint)
{
    const float Smooth = Sample.a;
    const float Sharp  = ZyFontMedian(Sample.rgb);

    // Mix the smooth field, better for rounded art, with the median, better for sharp art.
    const float Distance = lerp(Sharp, Smooth, Effect.InsetRoundness) - Effect.InsetThreshold;

    const float Inner = saturate(Spread * Distance + 0.5 + Effect.OutsetOffset);
    const float Outer = saturate(Spread * (Distance + Effect.OutsetWidth) + 0.5 + Effect.OutsetOffset + Effect.OutsetBias);

    // Fade the outset over the outer part of its width, in field units.
    const float BlurStart  = Effect.OutsetWidth + Effect.OutsetBias / Spread;
    const float BlurEnd    = BlurStart * (1.0 - Effect.OutsetBlur);
    const float BlurDepth  = Effect.InsetThreshold - Smooth - Effect.OutsetOffset / Spread;
    const float BlurFactor = lerp(1.0, 1.0 - smoothstep(BlurEnd, BlurStart, BlurDepth), step(0.0001, Effect.OutsetBlur));

    const float4 Outset = ZyUnpackTint(Effect.OutsetTint) * BlurFactor;

    return Tint * Inner + Outset * max(Outer - Inner, 0.0);
}

#endif // ZY_FONT_INCLUDED