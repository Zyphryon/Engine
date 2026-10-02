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

/// \brief Carries a direction from the light into a face's own right, up and depth.
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

/// \brief Gets the middle of a tile of the atlas, in its clip space.
vec2 ZyCubeTile(float Tile, float Per)
{
    float Row    = floor((Tile + 0.5) / Per);
    float Column = Tile - Row * Per;

    return (vec2(Column, Row) * 2.0 + 1.0) / Per - 1.0;
}

/// \brief Places a point seen from a light into the tile one of its faces was given.
///
/// \param Atlas Tiles per row, the share of a tile the face spans, the near plane and texels per tile.
/// \param Seen  The point on the face, kept within the tile while its x and y stay within its depth.
vec4 ZyCubePlace(vec3 World, vec4 Lamp, int Face, float Tile, vec4 Atlas, out vec3 Seen)
{
    vec3  View = ZyCubeView(World - Lamp.xyz, Face);
    float Near = Atlas.z;
    float Far  = Lamp.w;

    Seen = vec3(View.xy * Atlas.y, View.z);

    return vec4(Seen.xy / Atlas.x + ZyCubeTile(Tile, Atlas.x) * View.z, (View.z - Near) * Far / (Far - Near), View.z);
}

/// \brief Gets how far a point lies within each side of its face, for the rasterizer to clip the tile against.
vec4 ZyCubeClip(vec3 Seen)
{
    return Seen.z - vec4(Seen.x, -Seen.x, Seen.y, -Seen.y);
}

/// Returns how much of a light reaches a point past what stands in its way, softer the further the point lies
/// behind the blocker and the wider the light's source.
///
/// \param Atlas Tiles per row, the share of a tile a face spans, the near plane and texels per tile.
float ZyCubeShadow(
    sampler2D Depths, sampler2DShadow Map, vec4 Atlas,
    vec3 World, vec3 Normal, vec4 Light, float Source, vec4 Faces0, vec2 Faces1, vec2 Pixel)
{
    vec3  Reach    = World - Light.xyz;
    int   Face     = ZyCubeFace(Reach);
    float Tiles[6] = float[6](Faces0.x, Faces0.y, Faces0.z, Faces0.w, Faces1.x, Faces1.y);
    float Tile     = Tiles[Face];

    if (Tile < 0.0)
    {
        return 1.0;
    }

    float Per   = Atlas.x;
    float Scale = Atlas.y;
    float Near  = Atlas.z;
    float Texel = 1.0 / (Per * Atlas.w);
    float Far   = Light.w;

    float Grain = 2.0 * ZyCubeView(Reach, Face).z / (Scale * Atlas.w);
    vec3  View  = ZyCubeView(Reach + Normal * (Grain * ZY_CUBE_LIFT), Face);
    vec2  Mid   = ZyCubeTile(Tile, Per);
    vec2  Uv    = (View.xy * Scale / (View.z * Per) + Mid) * 0.5 + 0.5;
    vec2  Least = Mid * 0.5 + 0.5 - (0.5 / Per - Texel);
    vec2  Most  = Mid * 0.5 + 0.5 + (0.5 / Per - Texel);

    float A     = Far / (Far - Near);
    float B     = -Far * Near / (Far - Near);
    float Depth = (A + B / max(View.z - ZY_CUBE_BIAS, Near)) * 0.5 + 0.5;

    // A world unit at the receiver's depth, as a share of the atlas.
    float Spread = Scale * 0.5 / (Per * View.z);
    float Turn   = ZyGradientNoise(Pixel) * 6.28318530718;
    vec2  Spin   = vec2(cos(Turn), sin(Turn));
    float Search = clamp(Source * 0.5 * Spread, Texel * 1.5, Texel * ZY_CUBE_WIDEST);

    float Found = 0.0;
    float Count = 0.0;

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

    if (Count == 0.0)
    {
        return 1.0;
    }

    float Blocker  = max(Found / Count, Near);
    float Penumbra = clamp(Source * 0.5 * (View.z - Blocker) / Blocker * Spread, Texel, Texel * ZY_CUBE_WIDEST);

    float Lit = 0.0;

    for (int Tap = 0; Tap < ZY_CUBE_TAPS; ++Tap)
    {
        vec2 At = clamp(Uv + ZySpiral(Tap, float(ZY_CUBE_TAPS), Spin) * Penumbra, Least, Most);

        Lit += texture(Map, vec3(At, Depth));
    }
    return Lit / float(ZY_CUBE_TAPS);
}

#endif // ZY_CUBE_INCLUDED