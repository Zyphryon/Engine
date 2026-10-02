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

/// The taps the blocker search reads.
#define ZY_CUBE_SEARCH 8

/// The taps the filter reads.
#define ZY_CUBE_TAPS   12

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

/// Returns how much of a light reaches a point past what stands in its way, softer the further the point lies
/// behind the blocker and the wider the light's source.
///
/// \param Atlas Tiles per row, the share of a tile a face spans, the near plane and texels per tile.
float ZyCubeShadow(
    Texture2D Depths, SamplerState Point, Texture2D Map, SamplerComparisonState Compare, float4 Atlas,
    float3 World, float3 Normal, float4 Light, float Source, float4 Faces0, float2 Faces1, float2 Pixel)
{
    const float3 Reach    = World - Light.xyz;
    const int    Face     = ZyCubeFace(Reach);
    const float  Tiles[6] = { Faces0.x, Faces0.y, Faces0.z, Faces0.w, Faces1.x, Faces1.y };
    const float  Tile     = Tiles[Face];

    if (Tile < 0.0)
    {
        return 1.0;
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
    const float2 Uv    = float2(Clip.x * 0.5 + 0.5, 0.5 - Clip.y * 0.5);
    const float2 Least = float2(Mid.x * 0.5 + 0.5, 0.5 - Mid.y * 0.5) - (0.5 / Per - Texel);
    const float2 Most  = float2(Mid.x * 0.5 + 0.5, 0.5 - Mid.y * 0.5) + (0.5 / Per - Texel);

    const float A     = Far / (Far - Near);
    const float B     = -Far * Near / (Far - Near);
    const float Depth = A + B / max(View.z - ZY_CUBE_BIAS, Near);

    // A world unit at the receiver's depth, as a share of the atlas.
    const float  Spread = Scale * 0.5 / (Per * View.z);
    const float  Turn   = ZyGradientNoise(Pixel) * 6.28318530718;
    const float2 Spin   = float2(cos(Turn), sin(Turn));
    const float  Search = clamp(Source * 0.5 * Spread, Texel * 1.5, Texel * ZY_CUBE_WIDEST);

    float Found = 0.0;
    float Count = 0.0;

    [unroll]
    for (int Tap = 0; Tap < ZY_CUBE_SEARCH; ++Tap)
    {
        const float2 At     = clamp(Uv + ZySpiral(Tap, ZY_CUBE_SEARCH, Spin) * Search, Least, Most);
        const float  Stored = Depths.SampleLevel(Point, At, 0).r;

        if (Stored < Depth)
        {
            Found += B / (Stored - A);
            Count += 1.0;
        }
    }

    if (Count == 0.0)
    {
        return 1.0;
    }

    const float Blocker  = max(Found / Count, Near);
    const float Penumbra = clamp(Source * 0.5 * (View.z - Blocker) / Blocker * Spread, Texel, Texel * ZY_CUBE_WIDEST);

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