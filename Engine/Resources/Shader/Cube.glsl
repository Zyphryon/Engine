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
#include "Embedded://Shader/Shadow.glsl"

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

/// Picks one face's tile from six kept as tile plus one in 7-bit fields, +X -X +Y in x and -Y +Z -Z in y; -1 for none.
float ZyCubePick(vec2 Tiles, int Face)
{
    uint Packed = uint(Face < 3 ? Tiles.x : Tiles.y);
    uint Shift  = uint(Face < 3 ? Face : Face - 3) * 7u;

    return float((Packed >> Shift) & 127u) - 1.0;
}

/// Finds a point's atlas coordinate, depth and tile bounds on a face given its tile; false if the face has none.
bool ZyCubeLocateFace(
    vec4 Atlas, vec3 Reach, vec3 Normal, float Far, int Face, float Tile,
    out vec2 Uv, out float Depth, out vec2 Least, out vec2 Most, out float Distance)
{
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

    vec3  Size  = abs(Reach);
    float Grain = 2.0 * ZyMax3(Size.x, Size.y, Size.z) / (Scale * Atlas.w);
    vec3  View  = ZyCubeView(Reach + Normal * (Grain * ZY_CUBE_LIFT), Face);
    vec2  Mid   = ZyCubeTile(Tile, Per);
    vec2  Clip  = View.xy * Scale / (View.z * Per) + Mid;
    vec2  Heart = vec2(Mid.x * 0.5 + 0.5, 0.5 - Mid.y * 0.5);

    Uv       = vec2(Clip.x * 0.5 + 0.5, 0.5 - Clip.y * 0.5);
    Least    = Heart - (0.5 / Per - Texel);
    Most     = Heart + (0.5 / Per - Texel);
    Depth    = Far / (Far - Near) - Far * Near / (Far - Near) / max(View.z - ZY_CUBE_BIAS, Near);
    Distance = View.z;
    return true;
}

/// Finds a point's atlas coordinate, depth and tile bounds from tiles packed as ZyCubePick reads them.
bool ZyCubeLocate(
    vec4 Atlas, vec3 World, vec3 Normal, vec4 Light, vec2 Tiles,
    out vec2 Uv, out float Depth, out vec2 Least, out vec2 Most, out float Distance)
{
    vec3  Reach = World - Light.xyz;
    int   Face  = ZyCubeFace(Reach);
    float Tile  = ZyCubePick(Tiles, Face);

    return ZyCubeLocateFace(Atlas, Reach, Normal, Light.w, Face, Tile, Uv, Depth, Least, Most, Distance);
}

/// Small fixed filter about a point, the tent of ZyShadowSpread kept inside its tile.
float ZyCubeSpread(sampler2DShadow Map, vec2 Uv, float Depth, vec2 Least, vec2 Most, float Texel)
{
    float Half = Texel * 0.5;

    return ZyShadowSpread(Map, clamp(Uv, Least + Half, Most - Half), Depth, Texel);
}

/// Soft lamp shadow from the cube atlas, wider with source size and distance past the blocker.
float ZyCubeShadow(
    sampler2D Depths, sampler2DShadow Map, vec4 Atlas,
    vec3 World, vec3 Normal, vec4 Light, float Source, vec2 Tiles, vec2 Spin)
{
    vec2  Uv;
    float Depth;
    vec2  Least;
    vec2  Most;
    float Distance;

    if (!ZyCubeLocate(Atlas, World, Normal, Light, Tiles, Uv, Depth, Least, Most, Distance))
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

#if ZY_TIER >= 3

    const float Taps = float(ZY_CUBE_GATHER * 4);

    for (int Probe = 0; Probe < ZY_CUBE_GATHER; ++Probe)
    {
        vec2 At      = clamp(Uv + ZySpiral(Probe, float(ZY_CUBE_GATHER), Spin) * Search, Least, Most);
        vec4 Stored  = textureGather(Depths, At);
        vec4 Blocked = vec4(lessThan(Stored, vec4(Depth)));

        Found += dot(B / (Stored - A), Blocked);
        Count += dot(Blocked, vec4(1.0));
    }

#else

    const float Taps = float(ZY_CUBE_SEARCH);

    for (int Probe = 0; Probe < ZY_CUBE_SEARCH; ++Probe)
    {
        vec2  At     = clamp(Uv + ZySpiral(Probe, float(ZY_CUBE_SEARCH), Spin) * Search, Least, Most);
        float Stored = textureLod(Depths, At, 0.0).r;

        if (Stored < Depth)
        {
            Found += B / (Stored - A);
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
        return ZyCubeSpread(Map, Uv, Depth, Least, Most, Texel);
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