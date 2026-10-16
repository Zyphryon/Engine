// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.hlsl"
#include "Embedded://Shader/Math.hlsl"

cbuffer cb_Pass : register(b1)
{
    float4 u_Filter;
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

Texture2D    t_Level : register(t1);
SamplerState s_Level : register(s1);

float3 Tap(float2 Uv, float2 Offset)
{
    return t_Scene.SampleLevel(s_Scene, Uv + Offset * u_Filter.zw, 0).rgb;
}

#if defined(BLOOM_UP)

// A three by three tent over the level below, one of its texels apart, which spreads it without a seam.
float3 Spread(float2 Uv)
{
    float3 Result = Tap(Uv, float2(0.0, 0.0)) * 4.0;

    Result += (Tap(Uv, float2(-1.0,  0.0)) + Tap(Uv, float2(1.0, 0.0))
            +  Tap(Uv, float2( 0.0, -1.0)) + Tap(Uv, float2(0.0, 1.0))) * 2.0;
    Result += (Tap(Uv, float2(-1.0, -1.0)) + Tap(Uv, float2(1.0, -1.0))
            +  Tap(Uv, float2(-1.0,  1.0)) + Tap(Uv, float2(1.0,  1.0)));

    return Result * (1.0 / 16.0);
}

#else

// Thirteen taps over the level above, weighted so a bright point lands in the half the same whichever way it moves.
float3 Halve(float2 Uv)
{
    const float3 Middle  = Tap(Uv, float2( 0.0,  0.0));
    const float3 Inner   = Tap(Uv, float2(-1.0, -1.0)) + Tap(Uv, float2(1.0, -1.0))
                         + Tap(Uv, float2(-1.0,  1.0)) + Tap(Uv, float2(1.0,  1.0));
    const float3 Corners = Tap(Uv, float2(-2.0, -2.0)) + Tap(Uv, float2(2.0, -2.0))
                         + Tap(Uv, float2(-2.0,  2.0)) + Tap(Uv, float2(2.0,  2.0));
    const float3 Sides   = Tap(Uv, float2( 0.0, -2.0)) + Tap(Uv, float2(0.0,  2.0))
                         + Tap(Uv, float2(-2.0,  0.0)) + Tap(Uv, float2(2.0,  0.0));

    return Middle * 0.125 + Inner * 0.125 + Corners * 0.03125 + Sides * 0.0625;
}

#endif // BLOOM_UP

#if defined(BLOOM_SIFT)

// Keeps what clears the threshold, eased in over the knee.
float3 Extract(float3 Color)
{
    const float Peak = ZyMax3(Color.r, Color.g, Color.b);
    const float Soft = clamp(Peak - u_Filter.x + u_Filter.y, 0.0, 2.0 * u_Filter.y);
    const float Gain = max(Peak - u_Filter.x, Soft * Soft / (4.0 * u_Filter.y + 0.0001));

    return Color * (Gain / max(Peak, 0.0001));
}

#endif // BLOOM_SIFT

float4 main(fs_Input Input) : SV_Target
{
#if defined(BLOOM_UP)
    const float3 Result = Spread(Input.Texture) + t_Level.SampleLevel(s_Level, Input.Texture, 0).rgb;
#elif defined(BLOOM_SIFT)
    const float3 Result = Extract(Halve(Input.Texture));
#else
    const float3 Result = Halve(Input.Texture);
#endif

    return float4(Result, 1.0);
}

#endif // FRAGMENT_SHADER