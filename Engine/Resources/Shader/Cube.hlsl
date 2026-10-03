// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_CUBE_INCLUDED
#define ZY_CUBE_INCLUDED

#include "Embedded://Shader/Noise.hlsl"

/// How far a receiver is lifted along its normal before a light's face is read, in texels of that face.
#define ZY_CUBE_LIFT   1.5

/// How far a receiver is held in front of what a light's face recorded, in world units.
#define ZY_CUBE_BIAS   0.03

/// The widest a light's penumbra spreads, in texels.
#define ZY_CUBE_WIDEST 16.0

/// The 2x2 blocks the blocker search gathers.
#define ZY_CUBE_GATHER 3

/// The taps the filter reads.
#define ZY_CUBE_TAPS   12

/// The taps a fixed spread of comparisons reads.
#define ZY_CUBE_SPREAD 4

/// \brief Picks the face a direction from the light lands on, by its longest axis.
int ZyCubeFace(float3 Reach)
{
    const float3 Size = abs(Reach);

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

/// \brief Carries a direction from the light into a face's own right, up and depth.
float3 ZyCubeView(float3 Reach, int Face)
{
    const float3 kRight[6] =
    {
        float3( 0.0, 0.0, -1.0), float3(0.0, 0.0, 1.0), float3(1.0, 0.0, 0.0),
        float3( 1.0, 0.0,  0.0), float3(1.0, 0.0, 0.0), float3(-1.0, 0.0, 0.0)
    };
    const float3 kUp[6] =
    {
        float3(0.0, 1.0, 0.0), float3(0.0, 1.0, 0.0), float3(0.0, 0.0, -1.0),
        float3(0.0, 0.0, 1.0), float3(0.0, 1.0, 0.0), float3(0.0, 1.0,  0.0)
    };
    const float3 kAhead[6] =
    {
        float3(1.0, 0.0, 0.0), float3(-1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0),
        float3(0.0, -1.0, 0.0), float3(0.0, 0.0, 1.0), float3(0.0, 0.0, -1.0)
    };

    return float3(dot(Reach, kRight[Face]), dot(Reach, kUp[Face]), dot(Reach, kAhead[Face]));
}

/// \brief Gets the middle of a tile of the atlas, in its clip space.
float2 ZyCubeTile(float Tile, float Per)
{
    const float Row    = floor((Tile + 0.5) / Per);
    const float Column = Tile - Row * Per;

    return (float2(Column, Row) * 2.0 + 1.0) / Per - 1.0;
}

/// \brief Places a point seen from a light into the tile one of its faces was given.
///
/// \param Atlas Tiles per row, the share of a tile the face spans, the near plane and texels per tile.
/// \param Seen  The point on the face, kept within the tile while its x and y stay within its depth.
float4 ZyCubePlace(float3 World, float4 Lamp, int Face, float Tile, float4 Atlas, out float3 Seen)
{
    const float3 View = ZyCubeView(World - Lamp.xyz, Face);
    const float  Near = Atlas.z;
    const float  Far  = Lamp.w;

    Seen = float3(View.xy * Atlas.y, View.z);

    return float4(Seen.xy / Atlas.x + ZyCubeTile(Tile, Atlas.x) * View.z, (View.z - Near) * Far / (Far - Near), View.z);
}

/// \brief Gets how far a point lies within each side of its face, for the rasterizer to clip the tile against.
float4 ZyCubeClip(float3 Seen)
{
    return Seen.z - float4(Seen.x, -Seen.x, Seen.y, -Seen.y);
}

/// \brief Finds where a point lands on the face of a light it is seen from, and the part of the atlas that face holds.
///
/// \param Atlas    Tiles per row, the share of a tile a face spans, the near plane and texels per tile.
/// \param Uv       The point on the atlas.
/// \param Depth    The point's depth on the face, held in front of what the face recorded.
/// \param Least    The lowest coordinate a read may take and stay within the tile.
/// \param Most     The highest coordinate a read may take and stay within the tile.
/// \param Distance The point's distance from the light along the face's own depth, in world units.
/// \return `false` when the face was given no tile, which leaves the point lit.
bool ZyCubeLocate(
    float4 Atlas, float3 World, float3 Normal, float4 Light, float4 Faces0, float2 Faces1,
    out float2 Uv, out float Depth, out float2 Least, out float2 Most, out float Distance)
{
    const float3 Reach    = World - Light.xyz;
    const int    Face     = ZyCubeFace(Reach);
    const float  Tiles[6] = { Faces0.x, Faces0.y, Faces0.z, Faces0.w, Faces1.x, Faces1.y };
    const float  Tile     = Tiles[Face];

    Uv       = float2(0.0, 0.0);
    Depth    = 0.0;
    Least    = float2(0.0, 0.0);
    Most     = float2(0.0, 0.0);
    Distance = 0.0;

    if (Tile < 0.0)
    {
        return false;
    }

    const float Per   = Atlas.x;
    const float Scale = Atlas.y;
    const float Near  = Atlas.z;
    const float Texel = 1.0 / (Per * Atlas.w);
    const float Far   = Light.w;

    const float  Grain = 2.0 * ZyCubeView(Reach, Face).z / (Scale * Atlas.w);
    const float3 View  = ZyCubeView(Reach + Normal * (Grain * ZY_CUBE_LIFT), Face);
    const float2 Mid   = ZyCubeTile(Tile, Per);
    const float2 Clip  = View.xy * Scale / (View.z * Per) + Mid;
    const float2 Heart = float2(Mid.x * 0.5 + 0.5, 0.5 - Mid.y * 0.5);

    Uv       = float2(Clip.x * 0.5 + 0.5, 0.5 - Clip.y * 0.5);
    Least    = Heart - (0.5 / Per - Texel);
    Most     = Heart + (0.5 / Per - Texel);
    Depth    = Far / (Far - Near) - Far * Near / (Far - Near) / max(View.z - ZY_CUBE_BIAS, Near);
    Distance = View.z;
    return true;
}

/// \brief Reads a fixed spread of comparisons about a point of a light's face, kept within its tile.
///
/// \param Radius The spread, as a share of the atlas.
/// \param Spin   The angle the taps are carried around by, as its cosine and sine.
/// \return The share of the taps the point is lit through, from none at zero to all of them at one.
float ZyCubeSpread(
    Texture2D Map, SamplerComparisonState Compare, float2 Uv, float Depth, float2 Least, float2 Most,
    float Radius, float2 Spin)
{
    float Lit = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_CUBE_SPREAD; ++Tap)
    {
        Lit += Map.SampleCmpLevelZero(Compare, clamp(Uv + ZySpiral(Tap, ZY_CUBE_SPREAD, Spin) * Radius, Least, Most), Depth);
    }
    return Lit / float(ZY_CUBE_SPREAD);
}

/// Returns how much of a light reaches a point past what stands in its way, softer the further the point lies
/// behind the blocker and the wider the light's source.
///
/// \param Atlas Tiles per row, the share of a tile a face spans, the near plane and texels per tile.
float ZyCubeShadow(
    Texture2D Depths, SamplerState Point, Texture2D Map, SamplerComparisonState Compare, float4 Atlas,
    float3 World, float3 Normal, float4 Light, float Source, float4 Faces0, float2 Faces1, float2 Pixel)
{
    float2 Uv;
    float  Depth;
    float2 Least;
    float2 Most;
    float  Distance;

    if (!ZyCubeLocate(Atlas, World, Normal, Light, Faces0, Faces1, Uv, Depth, Least, Most, Distance))
    {
        return 1.0;
    }

    const float Per   = Atlas.x;
    const float Near  = Atlas.z;
    const float Texel = 1.0 / (Per * Atlas.w);
    const float Far   = Light.w;
    const float A     = Far / (Far - Near);
    const float B     = -Far * Near / (Far - Near);

    // A world unit at the receiver's depth, as a share of the atlas.
    const float  Spread = Atlas.y * 0.5 / (Per * Distance);
    const float  Turn   = ZyGradientNoise(Pixel) * 6.28318530718;
    const float2 Spin   = float2(cos(Turn), sin(Turn));
    const float  Search = clamp(Source * 0.5 * Spread, Texel * 1.5, Texel * ZY_CUBE_WIDEST);

    float Found = 0.0;
    float Count = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_CUBE_GATHER; ++Tap)
    {
        const float2 At      = clamp(Uv + ZySpiral(Tap, ZY_CUBE_GATHER, Spin) * Search, Least, Most);
        const float4 Stored  = Depths.GatherRed(Point, At);
        const float4 Blocked = float4(Stored < Depth);

        Found += dot(B / (Stored - A), Blocked);
        Count += dot(Blocked, 1.0);
    }

    if (Count == 0.0)
    {
        return 1.0;
    }

    // A search blocked all round is taken to lie deep in the shadow, which spares the filter where most pixels stand.
    if (Count == float(ZY_CUBE_GATHER * 4))
    {
        return 0.0;
    }

    const float Blocker  = max(Found / Count, Near);
    const float Penumbra = clamp(Source * 0.5 * (Distance - Blocker) / Blocker * Spread, Texel, Texel * ZY_CUBE_WIDEST);

    float Lit = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_CUBE_TAPS; ++Tap)
    {
        const float2 At = clamp(Uv + ZySpiral(Tap, ZY_CUBE_TAPS, Spin) * Penumbra, Least, Most);

        Lit += Map.SampleCmpLevelZero(Compare, At, Depth);
    }
    return Lit / float(ZY_CUBE_TAPS);
}

#endif // ZY_CUBE_INCLUDED
