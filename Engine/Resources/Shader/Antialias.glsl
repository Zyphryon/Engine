// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_ANTIALIAS_INCLUDED
#define ZY_ANTIALIAS_INCLUDED

#include "Embedded://Shader/Color.glsl"

/// Least contrast an edge needs, as a share of its brightest side.
#ifndef ZY_FXAA_THRESHOLD
#define ZY_FXAA_THRESHOLD 0.125
#endif

/// Least contrast an edge needs in the dark, where the share alone would catch noise.
#ifndef ZY_FXAA_FLOOR
#define ZY_FXAA_FLOOR     0.0312
#endif

/// How far an edge thinner than a pixel is softened.
#ifndef ZY_FXAA_SUBPIXEL
#define ZY_FXAA_SUBPIXEL  0.5
#endif

/// Steps the search takes toward each end of an edge, one for each stride below.
#define ZY_FXAA_STEPS     12

/// Texels each step of the search covers.
const float ZY_FXAA_STRIDE[ZY_FXAA_STEPS] = float[ZY_FXAA_STEPS](1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0);

/// Brightness of a linear color as the eye judges an edge, squeezed so a highlight weighs as much as a shadow.
float ZyAntialiasLuma(vec3 Color)
{
    float Light = ZyLuminance(Color);
    return sqrt(Light / (1.0 + Light));
}

/// Brightness of the image at a point, for the edge search.
float ZyAntialiasRead(sampler2D Image, vec2 Uv)
{
    return ZyAntialiasLuma(textureLod(Image, Uv, 0.0).rgb);
}

/// FXAA: finds the edge through a pixel and reads the image across it, as far as the pixel lies along the edge.
vec3 ZyAntialiasFXAA(sampler2D Image, vec2 Uv, vec2 Texel)
{
    vec3 Middle = textureLod(Image, Uv, 0.0).rgb;

    float M = ZyAntialiasLuma(Middle);
    float N = ZyAntialiasRead(Image, Uv + vec2(0.0, -Texel.y));
    float S = ZyAntialiasRead(Image, Uv + vec2(0.0,  Texel.y));
    float W = ZyAntialiasRead(Image, Uv + vec2(-Texel.x, 0.0));
    float E = ZyAntialiasRead(Image, Uv + vec2( Texel.x, 0.0));

    float Lowest  = min(M, min(min(N, S), min(W, E)));
    float Highest = max(M, max(max(N, S), max(W, E)));
    float Range   = Highest - Lowest;

    if (Range < max(ZY_FXAA_FLOOR, Highest * ZY_FXAA_THRESHOLD))
    {
        return Middle;
    }

    float NW = ZyAntialiasRead(Image, Uv + vec2(-Texel.x, -Texel.y));
    float NE = ZyAntialiasRead(Image, Uv + vec2( Texel.x, -Texel.y));
    float SW = ZyAntialiasRead(Image, Uv + vec2(-Texel.x,  Texel.y));
    float SE = ZyAntialiasRead(Image, Uv + vec2( Texel.x,  Texel.y));

    // A pixel unlike all its neighbours is a sliver thinner than itself, softened by how far it stands out.
    float Around   = (2.0 * (N + S + W + E) + NW + NE + SW + SE) / 12.0;
    float Stands   = smoothstep(0.0, 1.0, clamp(abs(Around - M) / Range, 0.0, 1.0));
    float Subpixel = Stands * Stands * ZY_FXAA_SUBPIXEL;

    // The edge runs the way the image changes least.
    float Across = abs(NW + SW - 2.0 * W) + 2.0 * abs(N + S - 2.0 * M) + abs(NE + SE - 2.0 * E);
    float Down   = abs(NW + NE - 2.0 * N) + 2.0 * abs(W + E - 2.0 * M) + abs(SW + SE - 2.0 * S);
    bool  Level  = (Across >= Down);

    // The pixel is read toward the side that differs from it most.
    float Before  = Level ? N : W;
    float After   = Level ? S : E;
    bool  Steeper = abs(Before - M) >= abs(After - M);
    float Change  = 0.25 * max(abs(Before - M), abs(After - M));
    float Average = 0.5 * ((Steeper ? Before : After) + M);
    float Step    = (Level ? Texel.y : Texel.x) * (Steeper ? -1.0 : 1.0);

    vec2 Edge  = Uv + (Level ? vec2(0.0, Step * 0.5) : vec2(Step * 0.5, 0.0));
    vec2 Along = Level ? vec2(Texel.x, 0.0) : vec2(0.0, Texel.y);

    // Walks both ways along the edge until the brightness leaves it.
    vec2  Back      = Edge;
    vec2  Ahead     = Edge;
    float EndBack   = 0.0;
    float EndAhead  = 0.0;
    bool  DoneBack  = false;
    bool  DoneAhead = false;

    for (int Index = 0; Index < ZY_FXAA_STEPS; ++Index)
    {
        if (!DoneBack)
        {
            Back    -= Along * ZY_FXAA_STRIDE[Index];
            EndBack  = ZyAntialiasRead(Image, Back) - Average;
            DoneBack = abs(EndBack) >= Change;
        }
        if (!DoneAhead)
        {
            Ahead     += Along * ZY_FXAA_STRIDE[Index];
            EndAhead   = ZyAntialiasRead(Image, Ahead) - Average;
            DoneAhead  = abs(EndAhead) >= Change;
        }
    }

    float ToBack  = Level ? Uv.x - Back.x : Uv.y - Back.y;
    float ToAhead = Level ? Ahead.x - Uv.x : Ahead.y - Uv.y;
    bool  Nearer  = ToBack < ToAhead;

    // Only the end that turns the way the pixel does pulls it across, the further the nearer that end is.
    bool  Turns  = ((Nearer ? EndBack : EndAhead) < 0.0) != (M < Average);
    float Pull   = Turns ? 0.5 - min(ToBack, ToAhead) / (ToBack + ToAhead) : 0.0;
    float Offset = max(Pull, Subpixel) * Step;

    return textureLod(Image, Uv + (Level ? vec2(0.0, Offset) : vec2(Offset, 0.0)), 0.0).rgb;
}

/// Contrast-adaptive sharpening: lifts fine detail by the room it has, leaving strong edges as they are.
vec3 ZyAntialiasSharpen(sampler2D Image, vec2 Uv, vec2 Texel, vec3 Middle, float Amount)
{
    vec3 N = textureLod(Image, Uv + vec2(0.0, -Texel.y), 0.0).rgb;
    vec3 S = textureLod(Image, Uv + vec2(0.0,  Texel.y), 0.0).rgb;
    vec3 W = textureLod(Image, Uv + vec2(-Texel.x, 0.0), 0.0).rgb;
    vec3 E = textureLod(Image, Uv + vec2( Texel.x, 0.0), 0.0).rgb;

    vec4  Cross = vec4(ZyAntialiasLuma(N), ZyAntialiasLuma(S), ZyAntialiasLuma(W), ZyAntialiasLuma(E));
    float M     = ZyAntialiasLuma(Middle);
    float Lo    = min(M, min(min(Cross.x, Cross.y), min(Cross.z, Cross.w)));
    float Hi    = max(M, max(max(Cross.x, Cross.y), max(Cross.z, Cross.w)));

    float Room   = sqrt(clamp(min(Lo, 1.0 - Hi) / max(Hi, 1e-4), 0.0, 1.0));
    float Weight = -Room / mix(8.0, 5.0, clamp(Amount, 0.0, 1.0));

    return max((Middle + (N + S + W + E) * Weight) / (1.0 + 4.0 * Weight), vec3(0.0));
}

#endif // ZY_ANTIALIAS_INCLUDED
