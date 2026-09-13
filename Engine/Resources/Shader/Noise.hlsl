// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_NOISE_INCLUDED
#define ZY_NOISE_INCLUDED

#include "Embedded://Shader/Math.hlsl"

/// The count of taps the spiral is laid out for ahead of time.
#define ZY_SPIRAL_TAPS 32

/// The direction of every tap of the spiral, laid once so a pixel turns them rather than placing each anew.
static const float2 ZY_SPIRAL_TURN[ZY_SPIRAL_TAPS] = {
    float2( 1.000000,  0.000000), float2(-0.737369,  0.675490), float2( 0.087426, -0.996171), float2( 0.608439,  0.793601),
    float2(-0.984713, -0.174182), float2( 0.843755, -0.536728), float2(-0.259604,  0.965715), float2(-0.460907, -0.887448),
    float2( 0.939321,  0.343039), float2(-0.924346,  0.381556), float2( 0.423846, -0.905734), float2( 0.299284,  0.954164),
    float2(-0.865211, -0.501408), float2( 0.976676, -0.214719), float2(-0.575129,  0.818062), float2(-0.128511, -0.991708),
    float2( 0.764649,  0.644447), float2(-0.999146,  0.041318), float2( 0.708829, -0.705380), float2(-0.046191,  0.998933),
    float2(-0.640709, -0.767784), float2( 0.991069,  0.133347), float2(-0.820858,  0.571132), float2( 0.219481, -0.975617),
    float2( 0.497181,  0.867647), float2(-0.952693, -0.303935), float2( 0.907791, -0.419423), float2(-0.386061,  0.922473),
    float2(-0.338452, -0.940984), float2( 0.885189,  0.465231), float2(-0.966970,  0.254890), float2( 0.540838, -0.841127) };

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
    float Scattered = frac(Position * 0.1031);

    Scattered *= Scattered + 33.33;
    Scattered *= Scattered + Scattered;

    return frac(Scattered);
}

/// \brief Scatters a point on a plane into a number that holds no order against it.
///
/// \param Position The point to scatter.
///
/// \return The scattered number, over zero through one.
float ZyHash21(float2 Position)
{
    float3 Scattered = frac(Position.xyx * 0.1031);

    Scattered += dot(Scattered, Scattered.yzx + 33.33);

    return frac((Scattered.x + Scattered.y) * Scattered.z);
}

/// \brief Scatters a point in space into a number that holds no order against it.
///
/// \param Position The point to scatter.
///
/// \return The scattered number, over zero through one.
float ZyHash31(float3 Position)
{
    float3 Scattered = frac(Position * 0.1031);

    Scattered += dot(Scattered, Scattered.zyx + 31.32);

    return frac((Scattered.x + Scattered.y) * Scattered.z);
}

/// \brief Reads a field that scatters every whole point and eases between them.
///
/// \param Position The point to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyValueNoise(float2 Position)
{
    const float2 Cell   = floor(Position);
    const float2 Offset = frac(Position);
    const float2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    const float Lower = lerp(ZyHash21(Cell + float2(0.0, 0.0)), ZyHash21(Cell + float2(1.0, 0.0)), Weight.x);
    const float Upper = lerp(ZyHash21(Cell + float2(0.0, 1.0)), ZyHash21(Cell + float2(1.0, 1.0)), Weight.x);

    return lerp(Lower, Upper, Weight.y);
}

/// \brief Reads the interleaved gradient field, which spreads its values evenly over any small neighbourhood.
///
/// \param Position The pixel to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyGradientNoise(float2 Position)
{
    return frac(52.9829189 * frac(dot(Position, float2(0.06711056, 0.00583715))));
}

/// \brief Reads the interleaved gradient field over a point in space rather than a point on the screen.
///
/// \param Position The point to read the field at.
///
/// \return The value the field carries there, over zero through one.
float ZyGradientNoise(float3 Position)
{
    return frac(52.9829189 * frac(dot(Position, float3(0.06711056, 0.00583715, 0.00278233))));
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
float2 ZySpiral(float Index, float Count, float Rotation)
{
    const float Theta = Index * 2.39996322973 + Rotation;

    return float2(cos(Theta), sin(Theta)) * sqrt((Index + 0.5) / Count);
}

/// \brief Spreads a tap over a disc the way \ref ZySpiral does, turned by an angle carried as its cosine and sine.
///
/// \param Index The tap to place, counted from zero, below \ref ZY_SPIRAL_TAPS.
/// \param Count The count of taps the disc is spread over.
/// \param Turn  The angle the whole spiral is carried around by, as its cosine and sine.
///
/// \return The tap, inside the unit disc.
float2 ZySpiral(int Index, float Count, float2 Turn)
{
    return ZyRotate(ZY_SPIRAL_TURN[Index], Turn) * sqrt((float(Index) + 0.5) / Count);
}

/// \brief Measures the offset that breaks up the banding a target shows once it quantizes a color.
///
/// \param Position The pixel the color is written at.
/// \param Levels   The count of steps the target writes a channel over.
///
/// \return The offset to add to the color, either side of zero.
float ZyDither(float2 Position, float Levels)
{
    return (ZyGradientNoise(Position) - 0.5) / Levels;
}

/// \brief Measures the offset that breaks up the banding an eight-bit target shows.
///
/// \param Position The pixel the color is written at.
///
/// \return The offset to add to the color, either side of zero.
float ZyDither(float2 Position)
{
    return ZyDither(Position, 255.0);
}

#endif // ZY_NOISE_INCLUDED