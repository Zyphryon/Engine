// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_TONEMAP_INCLUDED
#define ZY_TONEMAP_INCLUDED

/// The matrix that carries linear sRGB into the space the ACES curve is fitted over.
static const float3x3 ZY_ACES_INPUT = float3x3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777);

/// The matrix that carries the tonemapped color back out into linear sRGB.
static const float3x3 ZY_ACES_OUTPUT = float3x3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602);

/// The brightness the GT curve rolls off towards.
#define ZY_GT_PEAK            1.00

/// The slope the GT curve holds through its straight section.
#define ZY_GT_CONTRAST        1.00

/// The point the GT curve leaves its toe and runs straight.
#define ZY_GT_LINEAR_START    0.22

/// The share of the range the GT curve runs straight over.
#define ZY_GT_LINEAR_LENGTH   0.40

/// How hard the GT curve pulls its toe towards black.
#define ZY_GT_BLACK_TIGHTNESS 1.33

/// The floor the GT curve lifts black off, for a display that cannot reach it.
#define ZY_GT_PEDESTAL        0.00

/// \brief Rolls a color of light off through the ACES filmic curve.
///
/// \param Color The color of light to roll off.
///
/// \return The tonemapped color, over zero through one.
float3 ZyTonemapAces(float3 Color)
{
    Color = mul(ZY_ACES_INPUT, Color);

    const float3 Numerator   = Color * (Color + 0.0245786) - 0.000090537;
    const float3 Denominator = Color * (0.983729 * Color + 0.4329510) + 0.238081;

    return saturate(mul(ZY_ACES_OUTPUT, Numerator / Denominator));
}

/// \brief Rolls a color of light off through a cheap approximation of the ACES filmic curve.
///
/// \param Color The color of light to roll off.
///
/// \return The tonemapped color, over zero through one.
float3 ZyTonemapAcesFast(float3 Color)
{
    return saturate((Color * (2.51 * Color + 0.03)) / (Color * (2.43 * Color + 0.59) + 0.14));
}

/// \brief Rolls one channel of light off through the Gran Turismo curve.
///
/// \note Hajime Uchimura's curve, built as a toe, a straight section and a shoulder blended by weights,
///       which is why every part of its shape is a parameter rather than a constant baked into a fit.
///
/// \param Channel        The channel of light to roll off.
/// \param Peak           The brightness the curve rolls off towards.
/// \param Contrast       The slope the curve holds through its straight section.
/// \param LinearStart    The point the curve leaves its toe and runs straight.
/// \param LinearLength   The share of the range the curve runs straight over.
/// \param BlackTightness The measure of how hard the curve pulls its toe towards black.
/// \param Pedestal       The floor the curve lifts black off.
///
/// \return The tonemapped channel.
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

/// \brief Rolls a color of light off through the Gran Turismo curve.
///
/// \param Color          The color of light to roll off.
/// \param Peak           The brightness the curve rolls off towards.
/// \param Contrast       The slope the curve holds through its straight section.
/// \param LinearStart    The point the curve leaves its toe and runs straight.
/// \param LinearLength   The share of the range the curve runs straight over.
/// \param BlackTightness The measure of how hard the curve pulls its toe towards black.
/// \param Pedestal       The floor the curve lifts black off.
///
/// \return The tonemapped color.
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

/// \brief Rolls a color of light off through the Gran Turismo curve, shaped as the engine defaults it.
///
/// \param Color The color of light to roll off.
///
/// \return The tonemapped color.
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