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
#define ZY_SHADOW_TAPS   16

/// Taps the filter reads when the penumbra is only a couple of texels wide.
#define ZY_SHADOW_NARROW 4

/// Soft shadow from a depth map: sharp at contact, softer the further the blocker.
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

    float Lit = 0.0;

    // A narrow penumbra needs only a few taps.
    [branch]
    if (Penumbra <= Least * 2.0)
    {
        [unroll]
        for (int Tap = 0; Tap < ZY_SHADOW_NARROW; ++Tap)
        {
            Lit += Map.SampleCmpLevelZero(Compare, Uv + ZySpiral(Tap, ZY_SHADOW_NARROW, Turn) * Penumbra, Depth);
        }
        return Lit / float(ZY_SHADOW_NARROW);
    }

    [unroll]
    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += Map.SampleCmpLevelZero(Compare, Uv + ZySpiral(Tap, ZY_SHADOW_TAPS, Turn) * Penumbra, Depth);
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED