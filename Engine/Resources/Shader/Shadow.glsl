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

/// 2x2 gathers the blocker search makes, where gather is available.
#define ZY_SHADOW_GATHER 4

/// Single taps the blocker search reads, where gather is not available.
#define ZY_SHADOW_SEARCH 12

/// Taps the filter reads.
#ifndef ZY_SHADOW_TAPS
#define ZY_SHADOW_TAPS   16
#endif

/// Taps the filter reads when the penumbra is only a couple of texels wide.
#define ZY_SHADOW_NARROW 4

/// Soft shadow from a depth map: sharp at contact, softer the further the blocker.
float ZyShadow(sampler2D Depths, sampler2DShadow Map, vec2 Uv, float Depth, float Texel, float Spread, vec2 Turn)
{
    float Least = Texel;
    float Most  = Texel * 48.0;

    // Search as wide as the penumbra a blocker at the map's near end would cast.
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

    // Blocked all round means deep in shadow, so skip the filter.
    if (Count == Taps)
    {
        return 0.0;
    }

    float Penumbra = clamp((Depth - Found / Count) * Spread, Least, Most);

    float Lit = 0.0;

    // A narrow penumbra needs only a few taps.
    if (Penumbra <= Least * 2.0)
    {
        for (int Tap = 0; Tap < ZY_SHADOW_NARROW; ++Tap)
        {
            Lit += textureLod(Map, vec3(Uv + ZySpiral(Tap, float(ZY_SHADOW_NARROW), Turn) * Penumbra, Depth), 0.0);
        }
        return Lit / float(ZY_SHADOW_NARROW);
    }

    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += textureLod(Map, vec3(Uv + ZySpiral(Tap, float(ZY_SHADOW_TAPS), Turn) * Penumbra, Depth), 0.0);
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED