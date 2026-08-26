// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_NOISE_INCLUDED
#define ZY_NOISE_INCLUDED

/// \brief Scatters the bits of a whole number into another that holds no order against it.
///
/// \param Value The number to scatter.
///
/// \return The scattered number.
uint ZyScatter(uint Value)
{
    Value ^= Value >> 16u;
    Value *= 0x7FEB352Du;
    Value ^= Value >> 15u;
    Value *= 0x846CA68Bu;
    Value ^= Value >> 16u;

    return Value;
}

/// \brief Scatters one number into another that holds no order against it.
///
/// \param Position The number to scatter.
///
/// \return The scattered number, over zero through one.
float ZyHash11(float Position)
{
    float Scattered = fract(Position * 0.1031);

    Scattered *= Scattered + 33.33;
    Scattered *= Scattered + Scattered;

    return fract(Scattered);
}

/// \brief Scatters a point on a plane into a number that holds no order against it.
///
/// \param Position The point to scatter.
///
/// \return The scattered number, over zero through one.
float ZyHash21(vec2 Position)
{
    vec3 Scattered = fract(Position.xyx * 0.1031);

    Scattered += dot(Scattered, Scattered.yzx + 33.33);

    return fract((Scattered.x + Scattered.y) * Scattered.z);
}

/// \brief Scatters a point in space into a number that holds no order against it.
///
/// \param Position The point to scatter.
///
/// \return The scattered number, over zero through one.
float ZyHash31(vec3 Position)
{
    vec3 Scattered = fract(Position * 0.1031);

    Scattered += dot(Scattered, Scattered.zyx + 31.32);

    return fract((Scattered.x + Scattered.y) * Scattered.z);
}

/// \brief Reads a field that scatters every whole point and eases between them.
///
/// \param Position The point to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyValueNoise(vec2 Position)
{
    vec2 Cell   = floor(Position);
    vec2 Offset = fract(Position);
    vec2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    float Lower = mix(ZyHash21(Cell + vec2(0.0, 0.0)), ZyHash21(Cell + vec2(1.0, 0.0)), Weight.x);
    float Upper = mix(ZyHash21(Cell + vec2(0.0, 1.0)), ZyHash21(Cell + vec2(1.0, 1.0)), Weight.x);

    return mix(Lower, Upper, Weight.y);
}

/// \brief Reads the interleaved gradient field, which spreads its values evenly over any small neighbourhood.
///
/// \param Position The pixel to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyGradientNoise(vec2 Position)
{
    return fract(52.9829189 * fract(dot(Position, vec2(0.06711056, 0.00583715))));
}

/// \brief Reads the interleaved gradient field over a point in space rather than a point on the screen.
///
/// \param Position The point to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyGradientNoise(vec3 Position)
{
    return fract(52.9829189 * fract(dot(Position, vec3(0.06711056, 0.00583715, 0.00278233))));
}

/// \brief Spreads a tap over a disc, evenly and without ever landing twice in the same place.
///
/// \note Turning by the golden angle each step is what keeps the spiral from settling into arms.
///
/// \param Index    The tap to place, counted from zero.
/// \param Count    The count of taps the disc is spread over.
/// \param Rotation The turn the whole spiral is carried around by, which breaks its pattern up per pixel.
///
/// \return The tap, inside the unit disc.
vec2 ZySpiral(float Index, float Count, float Rotation)
{
    float Theta = Index * 2.39996322973 + Rotation;

    return vec2(cos(Theta), sin(Theta)) * sqrt((Index + 0.5) / Count);
}

/// \brief Measures the offset that breaks up the banding a target shows once it quantizes a color.
///
/// \param Position The pixel the color is written at.
/// \param Levels   The count of steps the target writes a channel over.
///
/// \return The offset to add to the color, either side of zero.
float ZyDither(vec2 Position, float Levels)
{
    return (ZyGradientNoise(Position) - 0.5) / Levels;
}

/// \brief Measures the offset that breaks up the banding an eight-bit target shows.
///
/// \param Position The pixel the color is written at.
///
/// \return The offset to add to the color, either side of zero.
float ZyDither(vec2 Position)
{
    return ZyDither(Position, 255.0);
}

#endif // ZY_NOISE_INCLUDED