// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.hlsl"
#include "Embedded://Shader/Packing.hlsl"

cbuffer cb_Pass : register(b1)
{
    float4x4 u_Inverse;
    float4   u_Toward;
    float4   u_Eye;
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

Texture2D    t_Normal : register(t0);
SamplerState s_Normal : register(s0);
Texture2D    t_Reach  : register(t1);
SamplerState s_Reach  : register(s1);
Texture2D    t_Glow   : register(t2);
SamplerState s_Glow   : register(s2);

struct fs_Output
{
    float4 Occlusion : SV_Target0;
    float4 Bounce    : SV_Target1;
};

float3 Locate(float2 Clip, float Along)
{
#if defined(HORIZON_PERSPECTIVE)
    const float4 Near  = mul(u_Inverse, float4(Clip, 0.0, 1.0));
    const float3 Start = Near.xyz / Near.w;
    const float3 Sight = Start - u_Eye.xyz;

    return Start + Sight * (Along / dot(Sight, u_Toward.xyz));
#else
    const float3 Start = mul(u_Inverse, float4(Clip, 0.0, 1.0)).xyz;

    return Start + u_Toward.xyz * Along;
#endif
}

float3 Fetch(float2 Pixel)
{
    float2 Size;
    t_Reach.GetDimensions(Size.x, Size.y);

    // A sample stands where its texel's middle does, so a flat ground never reads as steps that close it in.
    const float2 Uv = (floor(Pixel) + 0.5) / Size;

    return Locate(float2(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0), t_Reach.SampleLevel(s_Reach, Uv, 0).r);
}

float3 Shine(float2 Pixel)
{
    float2 Size;
    t_Reach.GetDimensions(Size.x, Size.y);

    return t_Glow.SampleLevel(s_Glow, Pixel / Size, 0).rgb * u_Eye.w;
}

#define ZY_OCCLUSION_READ(Pixel) Fetch(Pixel)
#define ZY_OCCLUSION_GLOW(Pixel) Shine(Pixel)

#include "Embedded://Shader/Occlusion.hlsl"

fs_Output main(fs_Input Input)
{
    float2 Size;
    t_Reach.GetDimensions(Size.x, Size.y);

    const float Along = t_Reach.SampleLevel(s_Reach, Input.Texture, 0).r;

    fs_Output Result;

    if (Along >= ZY_OCCLUSION_OPEN)
    {
        Result.Occlusion = float4(0.5, 1.0, 0.5, 1.0);
        Result.Bounce    = float4(0.0, 0.0, 0.0, u_Eye.w);
        return Result;
    }

    const float2 Uv     = (floor(Input.Position.xy) + 0.5) / Size;
    const float2 Clip   = float2(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0);
    const float3 Center = Locate(Clip, Along);
    const float3 Normal = normalize(ZyDecodeNormalMap(t_Normal.SampleLevel(s_Normal, Input.Texture, 0).rgb));

#if defined(HORIZON_PERSPECTIVE)
    const float3 Across = Locate(Clip + float2(2.0 / Size.x, 0.0), Along) - Center;
    const float3 Down   = Locate(Clip - float2(0.0, 2.0 / Size.y), Along) - Center;
    const float3 Viewer = normalize(u_Eye.xyz - Center);
#else
    const float3 Across = mul(u_Inverse, float4(2.0 / Size.x, 0.0, 0.0, 0.0)).xyz;
    const float3 Down   = mul(u_Inverse, float4(0.0, -2.0 / Size.y, 0.0, 0.0)).xyz;
    const float3 Viewer = -u_Toward.xyz;
#endif

    float3      Bent;
    float3      Bounce;
    const float Visible = ZyHorizonOcclusion(
        Center, Normal, Viewer, Input.Position.xy, Across, Down, u_Toward.w,
        ZyOcclusionNoise(Input.Position.xy), Bent, Bounce);

    Result.Occlusion = float4(Bent * 0.5 + 0.5, Visible);
    Result.Bounce    = float4(Bounce, u_Eye.w);
    return Result;
}

#endif // FRAGMENT_SHADER