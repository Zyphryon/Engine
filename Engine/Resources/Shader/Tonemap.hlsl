// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_TONEMAP_INCLUDED
#define ZY_TONEMAP_INCLUDED

/// Carries linear sRGB into the space the ACES fit works in.
static const float3x3 ZY_ACES_INPUT = float3x3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777);

/// Carries the ACES fit's result back to linear sRGB.
static const float3x3 ZY_ACES_OUTPUT = float3x3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602);

/// Default GT peak brightness.
#define ZY_GT_PEAK            1.00

/// Default GT contrast, the slope of its straight section.
#define ZY_GT_CONTRAST        1.00

/// Default GT point where the toe ends and the straight section starts.
#define ZY_GT_LINEAR_START    0.22

/// Default GT length of the straight section, as a share of the range.
#define ZY_GT_LINEAR_LENGTH   0.40

/// Default GT pull of the toe towards black.
#define ZY_GT_BLACK_TIGHTNESS 1.33

/// Default GT black lift, for displays that cannot reach black.
#define ZY_GT_PEDESTAL        0.00

/// ACES filmic tonemap, to 0..1.
float3 ZyTonemapAces(float3 Color)
{
    Color = mul(ZY_ACES_INPUT, Color);

    const float3 Numerator   = Color * (Color + 0.0245786) - 0.000090537;
    const float3 Denominator = Color * (0.983729 * Color + 0.4329510) + 0.238081;

    return saturate(mul(ZY_ACES_OUTPUT, Numerator / Denominator));
}

/// Cheap approximation of the ACES filmic tonemap, to 0..1.
float3 ZyTonemapAcesFast(float3 Color)
{
    return saturate((Color * (2.51 * Color + 0.03)) / (Color * (2.43 * Color + 0.59) + 0.14));
}

/// Uchimura's Gran Turismo tonemap for one channel, blending a toe, a line and a shoulder.
float ZyTonemapGt(
    float Channel,
    float Peak,
    float Contrast,
    float LinearStart,
    float LinearLength,
    float BlackTightness,
    float Pedestal)
{
    const float Length   = ((Peak - LinearStart) * LinearLength) / Contrast;
    const float ToeEnd   = LinearStart + Length;
    const float ToeSlope = LinearStart + Contrast * Length;
    const float Falloff  = -((Contrast * Peak) / (Peak - ToeSlope)) / Peak;

    const float ToeWeight      = 1.0 - smoothstep(0.0, LinearStart, Channel);
    const float ShoulderWeight = step(ToeEnd, Channel);
    const float LinearWeight   = 1.0 - ToeWeight - ShoulderWeight;

    const float Toe      = LinearStart * pow(max(Channel, 0.0) / LinearStart, BlackTightness) + Pedestal;
    const float Linear   = LinearStart + Contrast * (Channel - LinearStart);
    const float Shoulder = Peak - (Peak - ToeSlope) * exp(Falloff * (Channel - ToeEnd));

    return Toe * ToeWeight + Linear * LinearWeight + Shoulder * ShoulderWeight;
}

/// Gran Turismo tonemap for a color.
float3 ZyTonemapGt(
    float3 Color,
    float  Peak,
    float  Contrast,
    float  LinearStart,
    float  LinearLength,
    float  BlackTightness,
    float  Pedestal)
{
    return float3(
        ZyTonemapGt(Color.r, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal),
        ZyTonemapGt(Color.g, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal),
        ZyTonemapGt(Color.b, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal));
}

/// Gran Turismo tonemap with the engine's default shape.
float3 ZyTonemapGt(float3 Color)
{
    return ZyTonemapGt(
        Color,
        ZY_GT_PEAK,
        ZY_GT_CONTRAST,
        ZY_GT_LINEAR_START,
        ZY_GT_LINEAR_LENGTH,
        ZY_GT_BLACK_TIGHTNESS,
        ZY_GT_PEDESTAL);
}

#endif // ZY_TONEMAP_INCLUDED