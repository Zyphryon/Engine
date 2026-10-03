// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_SHADOW_INCLUDED
#define ZY_SHADOW_INCLUDED

#include "Embedded://Shader/Noise.glsl"

/// The 2x2 blocks the blocker search gathers, where the driver can gather.
#define ZY_SHADOW_GATHER 4

/// The taps the blocker search reads one at a time, where it cannot.
#define ZY_SHADOW_SEARCH 12

/// The taps the filter reads.
#define ZY_SHADOW_TAPS   16

/// \brief Reads how much light reaches a point through a depth map, sharp where the point touches what blocks it and
///        softer the further below it the point lies.
///
/// \param Depths The map, read as plain depth with no filtering.
/// \param Map    The map, read through a comparison that passes where the reference is nearer.
/// \param Uv     The point's coordinate on the map.
/// \param Depth  The point's depth on the map, over the map's range.
/// \param Texel  The size of one texel, as a share of the map.
/// \param Spread How far the penumbra widens, as a share of the map, per unit of depth between blocker and point.
/// \param Turn   The angle the taps are carried around by, as its cosine and sine.
///
/// \return The share of light that reaches the point, from none at zero to all of it at one.
float ZyShadow(sampler2D Depths, sampler2DShadow Map, vec2 Uv, float Depth, float Texel, float Spread, vec2 Turn)
{
    float Least = Texel;
    float Most  = Texel * 48.0;

    // The search reaches as far as the widest penumbra a blocker right at the map's near end would cast.
    float Search = clamp(Spread * Depth, Least * 2.0, Most);

    float Found = 0.0;
    float Count = 0.0;

#ifdef GL_ARB_texture_gather

    const float Taps = float(ZY_SHADOW_GATHER * 4);

    for (int Tap = 0; Tap < ZY_SHADOW_GATHER; ++Tap)
    {
        vec4 Stored  = textureGather(Depths, Uv + ZySpiral(Tap, float(ZY_SHADOW_GATHER), Turn) * Search);
        vec4 Blocked = vec4(lessThan(Stored, vec4(Depth)));

        Found += dot(Stored, Blocked);
        Count += dot(Blocked, vec4(1.0));
    }

#else

    const float Taps = float(ZY_SHADOW_SEARCH);

    for (int Tap = 0; Tap < ZY_SHADOW_SEARCH; ++Tap)
    {
        float Stored = textureLod(Depths, Uv + ZySpiral(Tap, float(ZY_SHADOW_SEARCH), Turn) * Search, 0.0).r;

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

    // The penumbra never reaches past the search, so a search blocked all round leaves the filter nothing to find.
    if (Count == Taps)
    {
        return 0.0;
    }

    float Penumbra = clamp((Depth - Found / Count) * Spread, Least, Most);

    float Lit = 0.0;

    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += texture(Map, vec3(Uv + ZySpiral(Tap, float(ZY_SHADOW_TAPS), Turn) * Penumbra, Depth));
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED