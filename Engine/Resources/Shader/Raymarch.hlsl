// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_RAYMARCH_INCLUDED
#define ZY_RAYMARCH_INCLUDED

/// Henyey-Greenstein phase, scaled so an even medium reads one.
float ZyRaymarchPhase(float Cosine, float Forward)
{
    const float Facing = 1.0 + Forward * Forward - 2.0 * Forward * Cosine;

    return (1.0 - Forward * Forward) / (Facing * sqrt(Facing));
}

/// One over a ray direction, each axis kept off zero for box clipping.
float3 ZyRaymarchInverse(float3 Sight)
{
    return 1.0 / ((abs(Sight) > 1e-6) ? Sight : 1e-6);
}

/// Clips a ray to a box, giving where it enters and leaves; false if it misses before Reach.
bool ZyRaymarchClip(float3 Start, float3 Inverse, float3 Least, float3 Most, float Reach, out float Enter, out float Leave)
{
    const float3 Low     = (Least - Start) * Inverse;
    const float3 High    = (Most  - Start) * Inverse;
    const float3 Nearer  = min(Low, High);
    const float3 Further = max(Low, High);

    Enter = max(max(max(Nearer.x, Nearer.y), Nearer.z), 0.0);
    Leave = min(min(min(Further.x, Further.y), Further.z), Reach);

    return Leave > Enter;
}

/// Clips a ray to an ellipsoid, giving where it enters and leaves; false if it misses before Reach.
bool ZyRaymarchEllipsoid(float3 Start, float3 Sight, float3 Middle, float3 Radii, float Reach, out float Enter, out float Leave)
{
    const float3 Origin = (Start - Middle) / Radii;
    const float3 Way    = Sight / Radii;
    const float  Square = dot(Way, Way);
    const float  Along  = dot(Origin, Way);
    const float  Spread = Along * Along - Square * (dot(Origin, Origin) - 1.0);
    const float  Root   = sqrt(max(Spread, 0.0));

    Enter = max((-Along - Root) / Square, 0.0);
    Leave = min((-Along + Root) / Square, Reach);

    return Spread > 0.0 && Leave > Enter;
}

/// Clips a ray to an upright elliptic cylinder of any height, giving where it enters and leaves.
bool ZyRaymarchCylinder(float3 Start, float3 Sight, float2 Middle, float2 Radii, float Reach, out float Enter, out float Leave)
{
    const float2 Origin = (Start.xz - Middle) / Radii;
    const float2 Way    = Sight.xz / Radii;
    const float  Square = dot(Way, Way);
    const float  Along  = dot(Origin, Way);
    const float  Inside = dot(Origin, Origin) - 1.0;

    // A ray running up the axis is inside for its whole length or never.
    if (Square < 1e-8)
    {
        Enter = 0.0;
        Leave = Reach;
        return Inside < 0.0;
    }

    const float Spread = Along * Along - Square * Inside;
    const float Root   = sqrt(max(Spread, 0.0));

    Enter = max((-Along - Root) / Square, 0.0);
    Leave = min((-Along + Root) / Square, Reach);

    return Spread > 0.0 && Leave > Enter;
}

/// Billow noise from a stack of lattice slices, eased across a slice and between slices; G holds the next slice's R.
float ZyRaymarchLattice(
    Texture2DArray<float2> Lattice, SamplerState Wrap, float2 Plane, float Height, float Texel, float Slices)
{
    const float2 Cell   = floor(Plane);
    const float2 Offset = Plane - Cell;
    const float2 Uv     = (Cell + Offset * Offset * (3.0 - 2.0 * Offset) + 0.5) * Texel;
    const float  Layer  = floor(Height);
    const float  Rise   = Height - Layer;
    const float  Lower  = Layer - Slices * floor(Layer / Slices);
    const float2 Pair   = Lattice.SampleLevel(Wrap, float3(Uv, Lower), 0.0);

    return lerp(Pair.x, Pair.y, Rise * Rise * (3.0 - 2.0 * Rise));
}

/// The size of the half-size target an output of the given size is measured into.
float2 ZyRaymarchHalf(float2 Full)
{
    return max(floor(Full * 0.5), 1.0);
}

/// The first full-size texel of the 2x2 block a half-size texel covers.
int2 ZyRaymarchBlock(float2 Pixel, float2 Full)
{
    return int2(Pixel * (Full / ZyRaymarchHalf(Full)) - 0.5);
}

/// The depth a half-size texel keeps, farthest on a checkerboard and nearest elsewhere.
float ZyRaymarchMeasure(float2 Pixel, float4 Reaches)
{
    const bool Far = ((uint(Pixel.x) + uint(Pixel.y)) & 1u) == 0u;

    return Far ? max(max(Reaches.x, Reaches.y), max(Reaches.z, Reaches.w))
               : min(min(Reaches.x, Reaches.y), min(Reaches.z, Reaches.w));
}

/// Whether the four texels of a half-size medium a full-size pixel settles from are empty; Linear is bilinear.
bool ZyRaymarchVacant(Texture2D Medium, SamplerState Linear, float2 Pixel, float2 Half, float2 Full)
{
    const float2 Corner = floor(Pixel * (Half / Full) - 0.5);

    // At the middle of the four a bilinear tap weighs each a quarter, so it reads empty only where all four are.
    return Medium.SampleLevel(Linear, (Corner + 1.0) / Half, 0.0).a <= 0.0;
}

/// Depth-aware upsample of a half-size medium onto a full-size pixel; Clamp is a clamping sampler for Reaches.
float4 ZyRaymarchSettle(
    Texture2D Medium, Texture2D<float> Reaches, SamplerState Clamp,
    float2 Pixel, float2 Half, float2 Full, float Reach, float Reject)
{
    const float2 At     = Pixel * (Half / Full) - 0.5;
    const float2 Corner = floor(At);
    const float2 Blend  = At - Corner;
    const int2   Last   = int2(Half) - 1;

#if ZY_TIER >= 3
    const float4 Gaps = abs(Reaches.GatherRed(Clamp, (Corner + 1.0) / Half).wzxy - Reach);
#endif

    float4 Sum     = 0.0;
    float  Total   = 0.0;
    float  Nearest = 1e30;
    float4 Closest = 0.0;

    [unroll]
    for (int Index = 0; Index < 4; ++Index)
    {
        const int2   Offset = int2(Index & 1, Index >> 1);
        const int3   Tap    = int3(clamp(int2(Corner) + Offset, 0, Last), 0);
        const float4 Light  = Medium.Load(Tap);
#if ZY_TIER >= 3
        const float  Gap    = Gaps[Index];
#else
        const float  Gap    = abs(Reaches.Load(Tap) - Reach);
#endif
        const float2 Share  = (Offset != 0) ? Blend : 1.0 - Blend;
        const float  Weight = Share.x * Share.y * saturate(1.0 - Gap * Reject);

        Sum   += Light * Weight;
        Total += Weight;

        if (Gap < Nearest)
        {
            Nearest = Gap;
            Closest = Light;
        }
    }
    return (Total > 1e-4) ? Sum / Total : Closest;
}

#endif // ZY_RAYMARCH_INCLUDED