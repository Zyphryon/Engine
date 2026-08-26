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
const mat3 ZY_ACES_INPUT = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777);

/// The matrix that carries the tonemapped color back out into linear sRGB.
const mat3 ZY_ACES_OUTPUT = mat3(
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
vec3 ZyTonemapAces(vec3 Color)
{
    Color = Color * ZY_ACES_INPUT;

    vec3 Numerator   = Color * (Color + 0.0245786) - 0.000090537;
    vec3 Denominator = Color * (0.983729 * Color + 0.4329510) + 0.238081;

    return clamp((Numerator / Denominator) * ZY_ACES_OUTPUT, 0.0, 1.0);
}

/// \brief Rolls a color of light off through a cheap approximation of the ACES filmic curve.
///
/// \param Color The color of light to roll off.
///
/// \return The tonemapped color, over zero through one.
vec3 ZyTonemapAcesFast(vec3 Color)
{
    return clamp((Color * (2.51 * Color + 0.03)) / (Color * (2.43 * Color + 0.59) + 0.14), 0.0, 1.0);
}

/// \brief Rolls one channel of light off through the Gran Turismo curve.
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
    float Length   = ((Peak - LinearStart) * LinearLength) / Contrast;
    float ToeEnd   = LinearStart + Length;
    float ToeSlope = LinearStart + Contrast * Length;
    float Falloff  = -((Contrast * Peak) / (Peak - ToeSlope)) / Peak;

    float ToeWeight      = 1.0 - smoothstep(0.0, LinearStart, Channel);
    float ShoulderWeight = step(ToeEnd, Channel);
    float LinearWeight   = 1.0 - ToeWeight - ShoulderWeight;

    float Toe      = LinearStart * pow(max(Channel, 0.0) / LinearStart, BlackTightness) + Pedestal;
    float Linear   = LinearStart + Contrast * (Channel - LinearStart);
    float Shoulder = Peak - (Peak - ToeSlope) * exp(Falloff * (Channel - ToeEnd));

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
vec3 ZyTonemapGt(
    vec3  Color,
    float Peak,
    float Contrast,
    float LinearStart,
    float LinearLength,
    float BlackTightness,
    float Pedestal)
{
    return vec3(
        ZyTonemapGt(Color.r, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal),
        ZyTonemapGt(Color.g, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal),
        ZyTonemapGt(Color.b, Peak, Contrast, LinearStart, LinearLength, BlackTightness, Pedestal));
}

/// \brief Rolls a color of light off through the Gran Turismo curve, shaped as the engine defaults it.
///
/// \param Color The color of light to roll off.
///
/// \return The tonemapped color.
vec3 ZyTonemapGt(vec3 Color)
{
    return ZyTonemapGt(Color,
        ZY_GT_PEAK,
        ZY_GT_CONTRAST,
        ZY_GT_LINEAR_START,
        ZY_GT_LINEAR_LENGTH,
        ZY_GT_BLACK_TIGHTNESS,
        ZY_GT_PEDESTAL);
}

#endif // ZY_TONEMAP_INCLUDED