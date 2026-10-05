// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_CUBE_INCLUDED
#define ZY_CUBE_INCLUDED

#include "Embedded://Shader/Noise.glsl"

/// How far a receiver is pushed along its normal before the lookup, in face texels.
#define ZY_CUBE_LIFT   1.5

/// Depth bias against what the face recorded, in world units.
#define ZY_CUBE_BIAS   0.03

/// The widest a penumbra spreads, in texels.
#define ZY_CUBE_WIDEST 16.0

/// 2x2 gathers the blocker search makes, where gather is available.
#define ZY_CUBE_GATHER 3

/// Single taps the blocker search reads, where gather is not available.
#define ZY_CUBE_SEARCH 8

/// Taps the filter reads.
#define ZY_CUBE_TAPS   12

/// Taps the small fixed filter reads.
#define ZY_CUBE_SPREAD 4

/// Picks the cube face a direction from the light lands on.
int ZyCubeFace(vec3 Reach)
{
    vec3 Size = abs(Reach);

    if (Size.x >= Size.y && Size.x >= Size.z)
    {
        return Reach.x >= 0.0 ? 0 : 1;
    }
    if (Size.y >= Size.z)
    {
        return Reach.y >= 0.0 ? 2 : 3;
    }
    return Reach.z >= 0.0 ? 4 : 5;
}

/// Turns a direction from the light into a face's right, up and depth.
vec3 ZyCubeView(vec3 Reach, int Face)
{
    const vec3 kRight[6] = vec3[6](
        vec3( 0.0, 0.0, -1.0), vec3(0.0, 0.0, 1.0), vec3(1.0, 0.0, 0.0),
        vec3( 1.0, 0.0,  0.0), vec3(1.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0));
    const vec3 kUp[6]    = vec3[6](
        vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, -1.0),
        vec3(0.0, 0.0, 1.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 1.0,  0.0));
    const vec3 kAhead[6] = vec3[6](
        vec3(1.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0),
        vec3(0.0, -1.0, 0.0), vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, -1.0));

    return vec3(dot(Reach, kRight[Face]), dot(Reach, kUp[Face]), dot(Reach, kAhead[Face]));
}

/// The centre of an atlas tile, in clip space.
vec2 ZyCubeTile(float Tile, float Per)
{
    float Row    = floor((Tile + 0.5) / Per);
    float Column = Tile - Row * Per;

    return (vec2(Column, Row) * 2.0 + 1.0) / Per - 1.0;
}

/// Places a point seen from a light into the atlas tile of one of its faces.
vec4 ZyCubePlace(vec3 World, vec4 Lamp, int Face, float Tile, vec4 Atlas, out vec3 Seen)
{
    vec3  View = ZyCubeView(World - Lamp.xyz, Face);
    float Near = Atlas.z;
    float Far  = Lamp.w;

    Seen = vec3(View.xy * Atlas.y, View.z);

    return vec4(Seen.xy / Atlas.x + ZyCubeTile(Tile, Atlas.x) * View.z, (View.z - Near) * Far / (Far - Near), View.z);
}

/// Distances to the face's four edges, for clipping the tile.
vec4 ZyCubeClip(vec3 Seen)
{
    return Seen.z - vec4(Seen.x, -Seen.x, Seen.y, -Seen.y);
}

/// Finds a point's atlas coordinate, depth and tile bounds; false if its face has no tile.
bool ZyCubeLocate(
    vec4 Atlas, vec3 World, vec3 Normal, vec4 Light, vec4 Faces0, vec2 Faces1,
    out vec2 Uv, out float Depth, out vec2 Least, out vec2 Most, out float Distance)
{
    vec3  Reach    = World - Light.xyz;
    int   Face     = ZyCubeFace(Reach);
    float Tiles[6] = float[6](Faces0.x, Faces0.y, Faces0.z, Faces0.w, Faces1.x, Faces1.y);
    float Tile     = Tiles[Face];

    Uv       = vec2(0.0);
    Depth    = 0.0;
    Least    = vec2(0.0);
    Most     = vec2(0.0);
    Distance = 0.0;

    if (Tile < 0.0)
    {
        return false;
    }

    float Per   = Atlas.x;
    float Scale = Atlas.y;
    float Near  = Atlas.z;
    float Texel = 1.0 / (Per * Atlas.w);
    float Far   = Light.w;

    float Grain = 2.0 * ZyCubeView(Reach, Face).z / (Scale * Atlas.w);
    vec3  View  = ZyCubeView(Reach + Normal * (Grain * ZY_CUBE_LIFT), Face);
    vec2  Mid   = ZyCubeTile(Tile, Per);

    Uv       = (View.xy * Scale / (View.z * Per) + Mid) * 0.5 + 0.5;
    Least    = Mid * 0.5 + 0.5 - (0.5 / Per - Texel);
    Most     = Mid * 0.5 + 0.5 + (0.5 / Per - Texel);
    Depth    = (Far / (Far - Near) - Far * Near / (Far - Near) / max(View.z - ZY_CUBE_BIAS, Near)) * 0.5 + 0.5;
    Distance = View.z;
    return true;
}

/// Small fixed filter about a point, kept inside its tile.
float ZyCubeSpread(sampler2DShadow Map, vec2 Uv, float Depth, vec2 Least, vec2 Most, float Radius, vec2 Spin)
{
    float Lit = 0.0;

    for (int Tap = 0; Tap < ZY_CUBE_SPREAD; ++Tap)
    {
        vec2 At = clamp(Uv + ZySpiral(Tap, float(ZY_CUBE_SPREAD), Spin) * Radius, Least, Most);

        Lit += textureLod(Map, vec3(At, Depth), 0.0);
    }
    return Lit / float(ZY_CUBE_SPREAD);
}

/// Soft lamp shadow from the cube atlas, wider with source size and distance past the blocker.
float ZyCubeShadow(
    sampler2D Depths, sampler2DShadow Map, vec4 Atlas,
    vec3 World, vec3 Normal, vec4 Light, float Source, vec4 Faces0, vec2 Faces1, vec2 Spin)
{
    vec2  Uv;
    float Depth;
    vec2  Least;
    vec2  Most;
    float Distance;

    if (!ZyCubeLocate(Atlas, World, Normal, Light, Faces0, Faces1, Uv, Depth, Least, Most, Distance))
    {
        return 1.0;
    }

    float Per   = Atlas.x;
    float Near  = Atlas.z;
    float Texel = 1.0 / (Per * Atlas.w);
    float Far   = Light.w;
    float A     = Far / (Far - Near);
    float B     = -Far * Near / (Far - Near);

    // One world unit at the receiver's depth, as a share of the atlas.
    float Spread = Atlas.y * 0.5 / (Per * Distance);
    float Search = clamp(Source * 0.5 * Spread, Texel * 1.5, Texel * ZY_CUBE_WIDEST);

    float Found = 0.0;
    float Count = 0.0;

#ifdef GL_ARB_texture_gather

    const float Taps = float(ZY_CUBE_GATHER * 4);

    for (int Tap = 0; Tap < ZY_CUBE_GATHER; ++Tap)
    {
        vec2 At      = clamp(Uv + ZySpiral(Tap, float(ZY_CUBE_GATHER), Spin) * Search, Least, Most);
        vec4 Stored  = textureGather(Depths, At);
        vec4 Blocked = vec4(lessThan(Stored, vec4(Depth)));

        Found += dot(B / (Stored * 2.0 - 1.0 - A), Blocked);
        Count += dot(Blocked, vec4(1.0));
    }

#else

    const float Taps = float(ZY_CUBE_SEARCH);

    for (int Tap = 0; Tap < ZY_CUBE_SEARCH; ++Tap)
    {
        vec2  At     = clamp(Uv + ZySpiral(Tap, float(ZY_CUBE_SEARCH), Spin) * Search, Least, Most);
        float Stored = textureLod(Depths, At, 0.0).r;

        if (Stored < Depth)
        {
            Found += B / (Stored * 2.0 - 1.0 - A);
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

    float Blocker  = max(Found / Count, Near);
    float Penumbra = clamp(Source * 0.5 * (Distance - Blocker) / Blocker * Spread, Texel, Texel * ZY_CUBE_WIDEST);

    // A narrow penumbra needs only the few taps of the fixed filter.
    if (Penumbra <= Texel * 2.0)
    {
        return ZyCubeSpread(Map, Uv, Depth, Least, Most, Penumbra, Spin);
    }

    float Lit = 0.0;

    for (int Tap = 0; Tap < ZY_CUBE_TAPS; ++Tap)
    {
        vec2 At = clamp(Uv + ZySpiral(Tap, float(ZY_CUBE_TAPS), Spin) * Penumbra, Least, Most);

        Lit += textureLod(Map, vec3(At, Depth), 0.0);
    }
    return Lit / float(ZY_CUBE_TAPS);
}

#endif // ZY_CUBE_INCLUDED
