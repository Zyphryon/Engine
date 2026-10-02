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

/// The taps the blocker search reads.
#define ZY_SHADOW_SEARCH 12

/// The taps the filter reads.
#define ZY_SHADOW_TAPS   16

/// \brief Reads how much light reaches a point through a depth map, sharp where the point touches what blocks it and
///        softer the further below it the point lies.
///
/// \param Depths  The map, read as plain depth.
/// \param Point   The sampler the plain depth is read through, with no filtering.
/// \param Map     The map, read through a comparison.
/// \param Compare The sampler that compares a reference against the map, passing where the reference is nearer.
/// \param Uv      The point's coordinate on the map.
/// \param Depth   The point's depth on the map, over the map's range.
/// \param Texel   The size of one texel, as a share of the map.
/// \param Spread  How far the penumbra widens, as a share of the map, per unit of depth between blocker and point.
/// \param Turn    The angle the taps are carried around by, as its cosine and sine.
///
/// \return The share of light that reaches the point, from none at zero to all of it at one.
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

    // The search reaches as far as the widest penumbra a blocker right at the map's near end would cast.
    const float Search = clamp(Spread * Depth, Least * 2.0, Most);

    float Found = 0.0;
    float Count = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_SHADOW_SEARCH; ++Tap)
    {
        const float Stored = Depths.SampleLevel(Point, Uv + ZySpiral(Tap, ZY_SHADOW_SEARCH, Turn) * Search, 0).r;

        if (Stored < Depth)
        {
            Found += Stored;
            Count += 1.0;
        }
    }

    if (Count == 0.0)
    {
        return 1.0;
    }

    // The penumbra never reaches past the search, so a search blocked all round leaves the filter nothing to find.
    if (Count == float(ZY_SHADOW_SEARCH))
    {
        return 0.0;
    }

    const float Penumbra = clamp((Depth - Found / Count) * Spread, Least, Most);

    float Lit = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += Map.SampleCmpLevelZero(Compare, Uv + ZySpiral(Tap, ZY_SHADOW_TAPS, Turn) * Penumbra, Depth);
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED