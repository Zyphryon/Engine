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

/// Taps in the precomputed spiral.
#define ZY_SPIRAL_TAPS 32

/// Precomputed spiral tap directions, so a pixel only has to rotate them.
static const float2 ZY_SPIRAL_TURN[ZY_SPIRAL_TAPS] = {
    float2( 1.000000,  0.000000), float2(-0.737369,  0.675490), float2( 0.087426, -0.996171), float2( 0.608439,  0.793601),
    float2(-0.984713, -0.174182), float2( 0.843755, -0.536728), float2(-0.259604,  0.965715), float2(-0.460907, -0.887448),
    float2( 0.939321,  0.343039), float2(-0.924346,  0.381556), float2( 0.423846, -0.905734), float2( 0.299284,  0.954164),
    float2(-0.865211, -0.501408), float2( 0.976676, -0.214719), float2(-0.575129,  0.818062), float2(-0.128511, -0.991708),
    float2( 0.764649,  0.644447), float2(-0.999146,  0.041318), float2( 0.708829, -0.705380), float2(-0.046191,  0.998933),
    float2(-0.640709, -0.767784), float2( 0.991069,  0.133347), float2(-0.820858,  0.571132), float2( 0.219481, -0.975617),
    float2( 0.497181,  0.867647), float2(-0.952693, -0.303935), float2( 0.907791, -0.419423), float2(-0.386061,  0.922473),
    float2(-0.338452, -0.940984), float2( 0.885189,  0.465231), float2(-0.966970,  0.254890), float2( 0.540838, -0.841127) };

/// Hashes an integer into a well-scattered one.
uint ZyScatter(uint Value)
{
    Value ^= Value >> 16u;
    Value *= 0x7FEB352Du;
    Value ^= Value >> 15u;
    Value *= 0x846CA68Bu;
    Value ^= Value >> 16u;

    return Value;
}

/// Hashes a number to 0..1.
float ZyHash11(float Position)
{
    float Scattered = frac(Position * 0.1031);

    Scattered *= Scattered + 33.33;
    Scattered *= Scattered + Scattered;

    return frac(Scattered);
}

/// Hashes a 2D point to 0..1.
float ZyHash21(float2 Position)
{
    float3 Scattered = frac(Position.xyx * 0.1031);

    Scattered += dot(Scattered, Scattered.yzx + 33.33);

    return frac((Scattered.x + Scattered.y) * Scattered.z);
}

/// Hashes a 3D point to 0..1.
float ZyHash31(float3 Position)
{
    float3 Scattered = frac(Position * 0.1031);

    Scattered += dot(Scattered, Scattered.zyx + 31.32);

    return frac((Scattered.x + Scattered.y) * Scattered.z);
}

/// Value noise in 0..1, hashed at whole points and eased between them.
float ZyValueNoise(float2 Position)
{
    const float2 Cell   = floor(Position);
    const float2 Offset = frac(Position);
    const float2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    const float Lower = lerp(ZyHash21(Cell + float2(0.0, 0.0)), ZyHash21(Cell + float2(1.0, 0.0)), Weight.x);
    const float Upper = lerp(ZyHash21(Cell + float2(0.0, 1.0)), ZyHash21(Cell + float2(1.0, 1.0)), Weight.x);

    return lerp(Lower, Upper, Weight.y);
}

/// Value noise in 0..1, read from a wrapping lattice texture with one texel per whole point.
float ZyValueNoise(Texture2D<float> Lattice, SamplerState Wrap, float2 Position, float Texel)
{
    const float2 Cell   = floor(Position);
    const float2 Offset = Position - Cell;
    const float2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    return Lattice.SampleLevel(Wrap, (Cell + Weight + 0.5) * Texel, 0.0);
}

/// Interleaved gradient noise in 0..1, even over any small patch of pixels.
float ZyGradientNoise(float2 Position)
{
    return frac(52.9829189 * frac(dot(Position, float2(0.06711056, 0.00583715))));
}

/// Interleaved gradient noise in 0..1, over a point in space.
float ZyGradientNoise(float3 Position)
{
    return frac(52.9829189 * frac(dot(Position, float3(0.06711056, 0.00583715, 0.00278233))));
}

/// Places a tap on a golden-angle spiral inside the unit disc.
float2 ZySpiral(float Index, float Count, float Rotation)
{
    const float Theta = Index * 2.39996322973 + Rotation;

    return float2(cos(Theta), sin(Theta)) * sqrt((Index + 0.5) / Count);
}

/// Places a tap from the precomputed spiral, rotated by an angle's cosine and sine.
float2 ZySpiral(int Index, float Count, float2 Turn)
{
    return ZyRotate(ZY_SPIRAL_TURN[Index], Turn) * sqrt((float(Index) + 0.5) / Count);
}

/// Dither offset that hides banding in a target with the given levels per channel.
float ZyDither(float2 Position, float Levels)
{
    return (ZyGradientNoise(Position) - 0.5) / Levels;
}

/// Dither offset that hides banding in an 8-bit target.
float ZyDither(float2 Position)
{
    return ZyDither(Position, 255.0);
}

#endif // ZY_NOISE_INCLUDED