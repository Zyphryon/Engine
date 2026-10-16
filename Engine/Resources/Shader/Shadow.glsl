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

/// Finds where a point in a light's clip space falls on its map, and its depth there; false off the map.
bool ZyShadowLocate(vec3 Clip, out vec2 Uv, out float Depth)
{
    Uv    = vec2(Clip.x * 0.5 + 0.5, 0.5 - Clip.y * 0.5);
    Depth = Clip.z;

    return !(any(lessThan(Uv, vec2(0.0))) || any(greaterThan(Uv, vec2(1.0))) || Clip.z >= 1.0);
}

/// Finds where a point falls on the map an orthographic light's matrix carries it to; false off the map.
bool ZyShadowLocate(mat4 Light, vec3 World, out vec2 Uv, out float Depth)
{
    return ZyShadowLocate((Light * vec4(World, 1.0)).xyz, Uv, Depth);
}

/// Small fixed filter about a point, four filtered comparisons half a texel off it each way that read as a 3x3 tent.
float ZyShadowSpread(sampler2DShadow Map, vec2 Uv, float Depth, float Texel)
{
    float Half = Texel * 0.5;

    float Lit = textureLod(Map, vec3(Uv + vec2(-Half, -Half), Depth), 0.0)
              + textureLod(Map, vec3(Uv + vec2( Half, -Half), Depth), 0.0)
              + textureLod(Map, vec3(Uv + vec2(-Half,  Half), Depth), 0.0)
              + textureLod(Map, vec3(Uv + vec2( Half,  Half), Depth), 0.0);
    return Lit * 0.25;
}

/// Soft shadow from a depth map, sharp at contact and softer the further the blocker.
float ZyShadow(sampler2D Depths, sampler2DShadow Map, vec2 Uv, float Depth, float Texel, float Spread, vec2 Turn)
{
    float Least = Texel;
    float Most  = Texel * 48.0;

    // Search as wide as the penumbra a blocker at the map's near end would cast.
    float Search = clamp(Spread * Depth, Least * 2.0, Most);

    float Found = 0.0;
    float Count = 0.0;

#if ZY_TIER >= 3

    const float Taps = float(ZY_SHADOW_GATHER * 4);

    for (int Probe = 0; Probe < ZY_SHADOW_GATHER; ++Probe)
    {
        vec4 Stored  = textureGather(Depths, Uv + ZySpiral(Probe, float(ZY_SHADOW_GATHER), Turn) * Search);
        vec4 Blocked = vec4(lessThan(Stored, vec4(Depth)));

        Found += dot(Stored, Blocked);
        Count += dot(Blocked, vec4(1.0));
    }

#else

    const float Taps = float(ZY_SHADOW_SEARCH);

    for (int Probe = 0; Probe < ZY_SHADOW_SEARCH; ++Probe)
    {
        float Stored = textureLod(Depths, Uv + ZySpiral(Probe, float(ZY_SHADOW_SEARCH), Turn) * Search, 0.0).r;

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

    // A narrow penumbra needs only the few taps of the fixed filter.
    if (Penumbra <= Least * 2.0)
    {
        return ZyShadowSpread(Map, Uv, Depth, Texel);
    }

    float Lit = 0.0;

    for (int Tap = 0; Tap < ZY_SHADOW_TAPS; ++Tap)
    {
        Lit += textureLod(Map, vec3(Uv + ZySpiral(Tap, float(ZY_SHADOW_TAPS), Turn) * Penumbra, Depth), 0.0);
    }
    return Lit / float(ZY_SHADOW_TAPS);
}

#endif // ZY_SHADOW_INCLUDED