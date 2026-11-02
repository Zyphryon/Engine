// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.hlsl"

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

static const float kKey   = 0.18;
static const float kShare = 0.5;

#if defined(EXPOSURE_ADAPT)

float Adapt()
{
    uint Width;
    uint Height;
    t_Scene.GetDimensions(Width, Height);

    float2 Total = 0.0;

    [loop]
    for (uint Y = 0; Y < Height; ++Y)
    {
        [loop]
        for (uint X = 0; X < Width; ++X)
        {
            Total += t_Scene.Load(int3(X, Y, 0)).rg;
        }
    }

    const float Mean   = Total.x / max(Total.y, 0.0001);
    const float Target = clamp(kShare * (log2(kKey) - Mean), u_Filter.z, u_Filter.w);

    float Last = t_Level.Load(int3(0, 0, 0)).r;

    if (!(abs(Last) < 64.0))
    {
        Last = Target;
    }
    return lerp(Last, Target, Target < Last ? u_Filter.x : u_Filter.y);
}

#elif !defined(EXPOSURE_KEEP)

float2 Measure(float2 Uv)
{
    float Total = 0.0;

    [unroll]
    for (int Y = 0; Y < 4; ++Y)
    {
        [unroll]
        for (int X = 0; X < 4; ++X)
        {
            const float2 Offset = (float2(X, Y) - 1.5) * 0.25 * u_Filter.xy;
            const float3 Color  = t_Scene.SampleLevel(s_Scene, Uv + Offset, 0).rgb;

            Total += log2(max(dot(Color, float3(0.2126, 0.7152, 0.0722)), 0.0001));
        }
    }

    const float2 Middle = (Uv - 0.5) * 2.0;
    const float  Weight = exp(-2.0 * dot(Middle, Middle));

    return float2(Total * (1.0 / 16.0) * Weight, Weight);
}

#endif

float4 main(fs_Input Input) : SV_Target
{
#if defined(EXPOSURE_ADAPT)
    return float4(Adapt(), 0.0, 0.0, 1.0);
#elif defined(EXPOSURE_KEEP)
    return t_Scene.Load(int3(0, 0, 0));
#else
    return float4(Measure(Input.Texture), 0.0, 1.0);
#endif
}

#endif // FRAGMENT_SHADER