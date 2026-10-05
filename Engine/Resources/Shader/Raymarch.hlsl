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

/// Billow noise from a stack of lattice slices, eased across a slice and between slices.
float ZyRaymarchLattice(
    Texture2DArray<float> Lattice, SamplerState Wrap, float2 Plane, float Height, float Texel, float Slices)
{
    const float2 Cell   = floor(Plane);
    const float2 Offset = Plane - Cell;
    const float2 Uv     = (Cell + Offset * Offset * (3.0 - 2.0 * Offset) + 0.5) * Texel;
    const float  Layer  = floor(Height);
    const float  Rise   = Height - Layer;
    const float  Lower  = Layer - Slices * floor(Layer / Slices);
    const float  Upper  = (Lower + 1.0 < Slices) ? Lower + 1.0 : 0.0;

    return lerp(Lattice.SampleLevel(Wrap, float3(Uv, Lower), 0.0),
                Lattice.SampleLevel(Wrap, float3(Uv, Upper), 0.0), Rise * Rise * (3.0 - 2.0 * Rise));
}

/// The first full-size texel of the 2x2 block a half-size texel covers.
int2 ZyRaymarchBlock(float2 Pixel, float2 Full)
{
    return int2(Pixel * (Full / max(floor(Full * 0.5), 1.0)) - 0.5);
}

/// The depth a half-size texel keeps, farthest on a checkerboard and nearest elsewhere.
float ZyRaymarchMeasure(float2 Pixel, float4 Reaches)
{
    const bool Far = ((uint(Pixel.x) + uint(Pixel.y)) & 1u) == 0u;

    return Far ? max(max(Reaches.x, Reaches.y), max(Reaches.z, Reaches.w))
               : min(min(Reaches.x, Reaches.y), min(Reaches.z, Reaches.w));
}

/// Depth-aware upsample of a half-size medium onto a full-size pixel.
float4 ZyRaymarchSettle(Texture2D Medium, Texture2D<float> Reaches, float2 Pixel, float2 Full, float Reach, float Reject)
{
    uint Across;
    uint Down;
    Medium.GetDimensions(Across, Down);

    const float2 At     = Pixel * (float2(Across, Down) / Full) - 0.5;
    const float2 Corner = floor(At);
    const float2 Blend  = At - Corner;
    const int2   Last   = int2(Across, Down) - 1;

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
        const float  Gap    = abs(Reaches.Load(Tap) - Reach);
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