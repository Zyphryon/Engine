// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_NOISE_INCLUDED
#define ZY_NOISE_INCLUDED

#include "Embedded://Shader/Math.glsl"

/// Taps in the precomputed spiral.
#define ZY_SPIRAL_TAPS 32

/// Precomputed spiral tap directions, so a pixel only has to rotate them.
const vec2 ZY_SPIRAL_TURN[ZY_SPIRAL_TAPS] = vec2[ZY_SPIRAL_TAPS](
    vec2( 1.000000,  0.000000), vec2(-0.737369,  0.675490), vec2( 0.087426, -0.996171), vec2( 0.608439,  0.793601),
    vec2(-0.984713, -0.174182), vec2( 0.843755, -0.536728), vec2(-0.259604,  0.965715), vec2(-0.460907, -0.887448),
    vec2( 0.939321,  0.343039), vec2(-0.924346,  0.381556), vec2( 0.423846, -0.905734), vec2( 0.299284,  0.954164),
    vec2(-0.865211, -0.501408), vec2( 0.976676, -0.214719), vec2(-0.575129,  0.818062), vec2(-0.128511, -0.991708),
    vec2( 0.764649,  0.644447), vec2(-0.999146,  0.041318), vec2( 0.708829, -0.705380), vec2(-0.046191,  0.998933),
    vec2(-0.640709, -0.767784), vec2( 0.991069,  0.133347), vec2(-0.820858,  0.571132), vec2( 0.219481, -0.975617),
    vec2( 0.497181,  0.867647), vec2(-0.952693, -0.303935), vec2( 0.907791, -0.419423), vec2(-0.386061,  0.922473),
    vec2(-0.338452, -0.940984), vec2( 0.885189,  0.465231), vec2(-0.966970,  0.254890), vec2( 0.540838, -0.841127));

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
    float Scattered = fract(Position * 0.1031);

    Scattered *= Scattered + 33.33;
    Scattered *= Scattered + Scattered;

    return fract(Scattered);
}

/// Hashes a 2D point to 0..1.
float ZyHash21(vec2 Position)
{
    vec3 Scattered = fract(Position.xyx * 0.1031);

    Scattered += dot(Scattered, Scattered.yzx + 33.33);

    return fract((Scattered.x + Scattered.y) * Scattered.z);
}

/// Hashes a 3D point to 0..1.
float ZyHash31(vec3 Position)
{
    vec3 Scattered = fract(Position * 0.1031);

    Scattered += dot(Scattered, Scattered.zyx + 31.32);

    return fract((Scattered.x + Scattered.y) * Scattered.z);
}

/// Value noise in 0..1, hashed at whole points and eased between them.
float ZyValueNoise(vec2 Position)
{
    vec2 Cell   = floor(Position);
    vec2 Offset = fract(Position);
    vec2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    float Lower = mix(ZyHash21(Cell + vec2(0.0, 0.0)), ZyHash21(Cell + vec2(1.0, 0.0)), Weight.x);
    float Upper = mix(ZyHash21(Cell + vec2(0.0, 1.0)), ZyHash21(Cell + vec2(1.0, 1.0)), Weight.x);

    return mix(Lower, Upper, Weight.y);
}

/// Value noise in 0..1, read from a wrapping lattice texture with one texel per whole point.
float ZyValueNoise(sampler2D Lattice, vec2 Position, float Texel)
{
    vec2 Cell   = floor(Position);
    vec2 Offset = Position - Cell;
    vec2 Weight = Offset * Offset * (3.0 - 2.0 * Offset);

    return textureLod(Lattice, (Cell + Weight + 0.5) * Texel, 0.0).r;
}

/// Interleaved gradient noise in 0..1, even over any small patch of pixels.
float ZyGradientNoise(vec2 Position)
{
    return fract(52.9829189 * fract(dot(Position, vec2(0.06711056, 0.00583715))));
}

/// Interleaved gradient noise in 0..1, over a point in space.
float ZyGradientNoise(vec3 Position)
{
    return fract(52.9829189 * fract(dot(Position, vec3(0.06711056, 0.00583715, 0.00278233))));
}

/// Places a tap on a golden-angle spiral inside the unit disc.
vec2 ZySpiral(float Index, float Count, float Rotation)
{
    float Theta = Index * 2.39996322973 + Rotation;

    return vec2(cos(Theta), sin(Theta)) * sqrt((Index + 0.5) / Count);
}

/// Places a tap from the precomputed spiral, rotated by an angle's cosine and sine.
vec2 ZySpiral(int Index, float Count, vec2 Turn)
{
    return ZyRotate(ZY_SPIRAL_TURN[Index], Turn) * sqrt((float(Index) + 0.5) / Count);
}

/// Dither offset that hides banding in a target with the given levels per channel.
float ZyDither(vec2 Position, float Levels)
{
    return (ZyGradientNoise(Position) - 0.5) / Levels;
}

/// Dither offset that hides banding in an 8-bit target.
float ZyDither(vec2 Position)
{
    return ZyDither(Position, 255.0);
}

#endif // ZY_NOISE_INCLUDED