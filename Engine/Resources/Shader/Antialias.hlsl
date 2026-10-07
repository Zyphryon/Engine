// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_ANTIALIAS_INCLUDED
#define ZY_ANTIALIAS_INCLUDED

#include "Embedded://Shader/Color.hlsl"

/// Least contrast an edge needs, as a share of its brightest side.
#ifndef ZY_FXAA_THRESHOLD
#define ZY_FXAA_THRESHOLD 0.125
#endif

/// Least contrast an edge needs in the dark, where the share alone would catch noise.
#ifndef ZY_FXAA_FLOOR
#define ZY_FXAA_FLOOR     0.0625
#endif

/// How far an edge thinner than a pixel is softened.
#ifndef ZY_FXAA_SUBPIXEL
#define ZY_FXAA_SUBPIXEL  0.5
#endif

/// Steps the search takes toward each end of an edge, one for each stride below.
#define ZY_FXAA_STEPS     12

/// Texels each step of the search covers.
static const float ZY_FXAA_STRIDE[ZY_FXAA_STEPS] = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 2.0, 4.0, 8.0 };

/// Brightness of a linear color as the eye judges an edge, squeezed so a highlight weighs as much as a shadow.
float ZyAntialiasLuma(float3 Color)
{
    const float Light = ZyLuminance(Color);
    return sqrt(Light / (1.0 + Light));
}

/// Brightness of the image at a point, for the edge search.
float ZyAntialiasRead(Texture2D Image, SamplerState Linear, float2 Uv)
{
    return ZyAntialiasLuma(Image.SampleLevel(Linear, Uv, 0).rgb);
}

/// FXAA, then contrast-adaptive sharpening from the same four neighbours, from none at zero to the most at one.
float3 ZyAntialiasFXAA(Texture2D Image, SamplerState Linear, float2 Uv, float2 Texel, float Sharpen)
{
    const float3 Middle = Image.SampleLevel(Linear, Uv, 0).rgb;
    const float3 North  = Image.SampleLevel(Linear, Uv + float2(0.0, -Texel.y), 0).rgb;
    const float3 South  = Image.SampleLevel(Linear, Uv + float2(0.0,  Texel.y), 0).rgb;
    const float3 West   = Image.SampleLevel(Linear, Uv + float2(-Texel.x, 0.0), 0).rgb;
    const float3 East   = Image.SampleLevel(Linear, Uv + float2( Texel.x, 0.0), 0).rgb;

    const float M = ZyAntialiasLuma(Middle);
    const float N = ZyAntialiasLuma(North);
    const float S = ZyAntialiasLuma(South);
    const float W = ZyAntialiasLuma(West);
    const float E = ZyAntialiasLuma(East);

    const float Lowest  = min(M, min(min(N, S), min(W, E)));
    const float Highest = max(M, max(max(N, S), max(W, E)));
    const float Range   = Highest - Lowest;

    float3 Result = Middle;

    if (Range >= max(ZY_FXAA_FLOOR, Highest * ZY_FXAA_THRESHOLD))
    {
        const float NW = ZyAntialiasRead(Image, Linear, Uv + float2(-Texel.x, -Texel.y));
        const float NE = ZyAntialiasRead(Image, Linear, Uv + float2( Texel.x, -Texel.y));
        const float SW = ZyAntialiasRead(Image, Linear, Uv + float2(-Texel.x,  Texel.y));
        const float SE = ZyAntialiasRead(Image, Linear, Uv + float2( Texel.x,  Texel.y));

        // A pixel unlike all its neighbours is a sliver thinner than itself, softened by how far it stands out.
        const float Around   = (2.0 * (N + S + W + E) + NW + NE + SW + SE) / 12.0;
        const float Stands   = smoothstep(0.0, 1.0, saturate(abs(Around - M) / Range));
        const float Subpixel = Stands * Stands * ZY_FXAA_SUBPIXEL;

        // The edge runs the way the image changes least.
        const float Across = abs(NW + SW - 2.0 * W) + 2.0 * abs(N + S - 2.0 * M) + abs(NE + SE - 2.0 * E);
        const float Down   = abs(NW + NE - 2.0 * N) + 2.0 * abs(W + E - 2.0 * M) + abs(SW + SE - 2.0 * S);
        const bool  Level  = (Across >= Down);

        // The pixel is read toward the side that differs from it most.
        const float Before  = Level ? N : W;
        const float After   = Level ? S : E;
        const bool  Steeper = abs(Before - M) >= abs(After - M);
        const float Change  = 0.25 * max(abs(Before - M), abs(After - M));
        const float Average = 0.5 * ((Steeper ? Before : After) + M);
        const float Step    = (Level ? Texel.y : Texel.x) * (Steeper ? -1.0 : 1.0);

        const float2 Edge  = Uv + (Level ? float2(0.0, Step * 0.5) : float2(Step * 0.5, 0.0));
        const float2 Along = Level ? float2(Texel.x, 0.0) : float2(0.0, Texel.y);

        // Walks both ways along the edge until the brightness leaves it.
        float2 Back      = Edge;
        float2 Ahead     = Edge;
        float  EndBack   = 0.0;
        float  EndAhead  = 0.0;
        bool   DoneBack  = false;
        bool   DoneAhead = false;

        [unroll]
        for (int Index = 0; Index < ZY_FXAA_STEPS; ++Index)
        {
            if (!DoneBack)
            {
                Back    -= Along * ZY_FXAA_STRIDE[Index];
                EndBack  = ZyAntialiasRead(Image, Linear, Back) - Average;
                DoneBack = abs(EndBack) >= Change;
            }
            if (!DoneAhead)
            {
                Ahead     += Along * ZY_FXAA_STRIDE[Index];
                EndAhead   = ZyAntialiasRead(Image, Linear, Ahead) - Average;
                DoneAhead  = abs(EndAhead) >= Change;
            }
        }

        const float ToBack  = Level ? Uv.x - Back.x : Uv.y - Back.y;
        const float ToAhead = Level ? Ahead.x - Uv.x : Ahead.y - Uv.y;
        const bool  Nearer  = ToBack < ToAhead;

        // Only the end that turns the way the pixel does pulls it across, the further the nearer that end is.
        const bool  Turns  = ((Nearer ? EndBack : EndAhead) < 0.0) != (M < Average);
        const float Pull   = Turns ? 0.5 - min(ToBack, ToAhead) / (ToBack + ToAhead) : 0.0;
        const float Offset = max(Pull, Subpixel) * Step;

        Result = Image.SampleLevel(Linear, Uv + (Level ? float2(0.0, Offset) : float2(Offset, 0.0)), 0).rgb;
    }

    if (Sharpen <= 0.0)
    {
        return Result;
    }

    // Fine detail is lifted by the room it has before it clips, so strong edges are left as they are.
    const float R      = ZyAntialiasLuma(Result);
    const float Lo     = min(R, min(min(N, S), min(W, E)));
    const float Hi     = max(R, max(max(N, S), max(W, E)));
    const float Room   = sqrt(saturate(min(Lo, 1.0 - Hi) / max(Hi, 1e-4)));
    const float Weight = -Room * 0.2 * saturate(Sharpen);

    return max((Result + (North + South + West + East) * Weight) / (1.0 + 4.0 * Weight), 0.0);
}

#endif // ZY_ANTIALIAS_INCLUDED
