// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.hlsl"
#include "Embedded://Shader/Color.hlsl"
#include "Embedded://Shader/Noise.hlsl"
#include "Embedded://Shader/Tonemap.hlsl"
#include "Embedded://Shader/Antialias.hlsl"

cbuffer cb_Pass : register(b1)
{
    float4 u_Grading;   // X = Exposure, Y = Saturation, Z = Vignette, W = Grain
    float4 u_Lift;      // RGB = Lift,   W = The grain's frame, or below zero when nothing past the curve is graded
    float4 u_Gamma;     // RGB = Gamma,  W = The strength of the halo
    float4 u_Gain;      // RGB = Gain,   W = The levels each channel is stepped to
    float4 u_Lens;      // XY = A texel of the scene, Z = Aberration in pixels
};

struct fs_Input
{
    float4 Position : SV_POSITION;
    float2 Texture  : TEXCOORD0;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef VERTEX_SHADER

fs_Input main(uint VertexID : SV_VertexID)
{
    const float4 Screen = ZyEmitScreen(VertexID);

    fs_Input Result;

    Result.Position = float4(Screen.xy, 0.0, 1.0);
    Result.Texture  = Screen.zw;

    return Result;
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

Texture2D    t_Scene : register(t0);
SamplerState s_Scene : register(s0);

Texture2D    t_Bloom : register(t1);
SamplerState s_Bloom : register(s1);

// GT7 is the curve the image is authored against; ACES is the variant a scene can ask for instead.
#if defined(ENABLE_TONEMAP_ACES)
    #define Tonemap ZyTonemapAces
#else
    #define Tonemap ZyTonemapGt
#endif // ENABLE_TONEMAP_ACES

float Grain(float2 Pixel, uint Frame)
{
    const int2 Cell = int2(Pixel);
    const uint Seed = ZyScatter(uint(Cell.x) * 73856093u ^ uint(Cell.y) * 19349663u ^ Frame * 83492791u);

    return float(Seed & 0xFFFFu) * (1.0 / 65535.0) - 0.5;
}

// A 4x4 ordered matrix that dithers a stepped gradient, each cell half a step in so the nudges centre on nought.
static const float kBayer[16] =
{
     0.5 / 16.0,  8.5 / 16.0,  2.5 / 16.0, 10.5 / 16.0,
    12.5 / 16.0,  4.5 / 16.0, 14.5 / 16.0,  6.5 / 16.0,
     3.5 / 16.0, 11.5 / 16.0,  1.5 / 16.0,  9.5 / 16.0,
    15.5 / 16.0,  7.5 / 16.0, 13.5 / 16.0,  5.5 / 16.0,
};

float3 Palette(float3 Color, float Levels, float2 Pixel)
{
    const int2  Cell  = int2(Pixel) & 3;
    const float Steps = Levels - 1.0;

    return floor(Color * Steps + 0.5 + (kBayer[Cell.y * 4 + Cell.x] - 0.5)) / Steps;
}

float4 main(fs_Input Input) : SV_Target
{
#if defined(ENABLE_ANTIALIAS_FXAA)
    float3 Scene = ZyAntialiasFXAA(t_Scene, s_Scene, Input.Texture, u_Lens.xy, 0.25);
#else
    float3 Scene = t_Scene.SampleLevel(s_Scene, Input.Texture, 0).rgb;
#endif

    // Red and blue part from green toward the corners, as a lens that bends each colour by its own amount.
    if (u_Lens.z > 0.0)
    {
        const float2 Shift = (Input.Texture - 0.5) * 2.0 * u_Lens.z * u_Lens.xy;

        Scene.r = t_Scene.SampleLevel(s_Scene, Input.Texture + Shift, 0).r;
        Scene.b = t_Scene.SampleLevel(s_Scene, Input.Texture - Shift, 0).b;
    }

    // The halo is scene referred, so it joins before exposure carries the pair into the curve.
    float3 Lit = Scene;

    if (u_Gamma.w > 0.0)
    {
        Lit += t_Bloom.SampleLevel(s_Bloom, Input.Texture, 0).rgb * u_Gamma.w;
    }

    float3 Color = Tonemap(Lit * u_Grading.x);

    // Saturation and vignette hold only over light, so both are taken before the image is encoded.
    const float Luma = ZyLuminance(Color);
    Color = lerp(float3(Luma, Luma, Luma), Color, u_Grading.y);

    const float2 Offset   = Input.Texture - 0.5;
    const float  Vignette = 1.0 - u_Grading.z * saturate(dot(Offset, Offset) * 2.0);

    Color *= Vignette;

    // A grade that changes nothing past the curve hands the curve's image back as it stands.
    if (u_Lift.w < 0.0)
    {
#if defined(ENABLE_ENCODED_OUTPUT)
        return float4(ZyToGamma(saturate(Color)), 1.0);
#else
        return float4(saturate(Color), 1.0);
#endif // ENABLE_ENCODED_OUTPUT
    }

    // The rest is judged by eye, so it works on the encoded image, where even steps look even.
    float3 Shown = ZyToGamma(saturate(Color));

    // The bands are graded on the image the curve handed back, which is the range they are authored against.
    Shown = pow(max(Shown * u_Gain.rgb + u_Lift.rgb, 0.0), u_Gamma.rgb);

    // Grain shows most at a halfway exposure of the encoded luma, so the corners the vignette darkened grain less.
    const float Exposed = saturate(ZyLuminance(Shown));
    const float Density = sqrt(saturate(4.0 * Exposed * (1.0 - Exposed)));

    // The grain is stirred by the frame it is handed, which keeps it from reading as dirt on the lens.
    Shown += Grain(Input.Position.xy, uint(u_Lift.w)) * (u_Grading.w * Density);
    Shown  = saturate(Shown);

    // The palette is what the display carried, so the steps land last, over the grain that dithers them.
    if (u_Gain.w > 1.0)
    {
        Shown = Palette(Shown, u_Gain.w, Input.Position.xy);
    }

    // The target encodes as it stores, so the image is handed back as light, unless it stores what it is handed.
#if defined(ENABLE_ENCODED_OUTPUT)
    return float4(Shown, 1.0);
#else
    return float4(ZyToLinear(Shown), 1.0);
#endif // ENABLE_ENCODED_OUTPUT
}

#endif // FRAGMENT_SHADER