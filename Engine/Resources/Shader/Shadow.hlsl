// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_SHADOW_INCLUDED
#define ZY_SHADOW_INCLUDED

#include "Embedded://Shader/Noise.hlsl"

/// 2x2 gathers the blocker search makes, where gather is available.
#define ZY_SHADOW_GATHER 4

/// Single taps the blocker search reads, where gather is not available.
#define ZY_SHADOW_SEARCH 12

/// Taps the filter reads.
#ifndef ZY_SHADOW_TAPS
#define ZY_SHADOW_TAPS   16
#endif

/// Finds where a point in a light's clip space falls on its map, and its depth there; false off the map.
bool ZyShadowLocate(float3 Clip, out float2 Uv, out float Depth)
{
    Uv    = float2(Clip.x * 0.5 + 0.5, 0.5 - Clip.y * 0.5);
    Depth = Clip.z;

    return !(any(Uv < 0.0) || any(Uv > 1.0) || Clip.z >= 1.0);
}

/// Finds where a point falls on the map an orthographic light's matrix carries it to; false off the map.
bool ZyShadowLocate(float4x4 Light, float3 World, out float2 Uv, out float Depth)
{
    return ZyShadowLocate(mul(Light, float4(World, 1.0)).xyz, Uv, Depth);
}

/// Small fixed filter about a point, four filtered comparisons half a texel off it each way that read as a 3x3 tent.
float ZyShadowSpread(Texture2D Map, SamplerComparisonState Compare, float2 Uv, float Depth, float Texel)
{
    const float Half = Texel * 0.5;

    const float Lit = Map.SampleCmpLevelZero(Compare, Uv + float2(-Half, -Half), Depth)
                    + Map.SampleCmpLevelZero(Compare, Uv + float2( Half, -Half), Depth)
                    + Map.SampleCmpLevelZero(Compare, Uv + float2(-Half,  Half), Depth)
                    + Map.SampleCmpLevelZero(Compare, Uv + float2( Half,  Half), Depth);
    return Lit * 0.25;
}

/// Soft shadow from a depth map, sharp at contact and softer the further the blocker.
float ZyShadow(
    Texture2D              Depths,
    SamplerState           Point,
    Texture2D              Map,
    SamplerComparisonState Compare,
    float2                 Uv,
    float                  Depth,
    float                  Texel,
    float                  Spread,
    float2                 Turn)
{
    const float Least = Texel;
    const float Most  = Texel * 48.0;

    // Search as wide as the penumbra a blocker at the map's near end would cast.
    const float Search = clamp(Spread * Depth, Least * 2.0, Most);

    float Found = 0.0;
    float Count = 0.0;

#if ZY_TIER >= 3

    const float Taps = float(ZY_SHADOW_GATHER * 4);

    [unroll]
    for (int Probe = 0; Probe < ZY_SHADOW_GATHER; ++Probe)
    {
        const float4 Stored  = Depths.GatherRed(Point, Uv + ZySpiral(Probe, ZY_SHADOW_GATHER, Turn) * Search);
        const float4 Blocked = float4(Stored < Depth);

        Found += dot(Stored, Blocked);
        Count += dot(Blocked, 1.0);
    }

#else

    const float Taps = float(ZY_SHADOW_SEARCH);

    [unroll]
    for (int Probe = 0; Probe < ZY_SHADOW_SEARCH; ++Probe)
    {
        const float Stored = Depths.SampleLevel(Point, Uv + ZySpiral(Probe, ZY_SHADOW_SEARCH, Turn) * Search, 0.0).r;

        if (Stored < Depth)
        {
            Found += Stored;
            Count += 1.0;
        }
    }

#endif

    if (Count == 0.0)
    {
        return 1.0;
    }

    // Blocked all round means deep in shadow, so skip the filter.
    if (Count == Taps)
    {
        return 0.0;
    }

    const float Penumbra = clamp((Depth - Found / Count) * Spread, Least, Most);

    // A narrow penumbra needs only the few taps of the fixed filter.
    [branch]
    if (Penumbra <= Least * 2.0)
    {
        return ZyShadowSpread(Map, Compare, Uv, Depth, Texel);
    }

    float Lit = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += Map.SampleCmpLevelZero(Compare, Uv + ZySpiral(Tap, ZY_SHADOW_TAPS, Turn) * Penumbra, Depth);
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED