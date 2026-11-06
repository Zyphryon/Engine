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
    float4 u_Settle;
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

Texture2D    t_Normal    : register(t0);
SamplerState s_Normal    : register(s0);
Texture2D    t_Reach     : register(t1);
SamplerState s_Reach     : register(s1);
Texture2D    t_Occlusion : register(t2);
SamplerState s_Occlusion : register(s2);

float4 main(fs_Input Input) : SV_Target0
{
    float2 Size;
    t_Reach.GetDimensions(Size.x, Size.y);

    float2 Full;
    t_Normal.GetDimensions(Full.x, Full.y);

    const float  Along = t_Reach.SampleLevel(s_Reach, Input.Texture, 0).r;
    const float2 Texel = 2.0 / Full;

#if ZY_TIER >= 3
    const float2 Corner = floor(Input.Position.xy);
    const float4 Lower  = t_Reach.GatherRed(s_Reach, Corner / Size);
    const float4 Right  = t_Reach.GatherRed(s_Reach, (Corner + float2(2.0, 0.0)) / Size);
    const float4 Upper  = t_Reach.GatherRed(s_Reach, (Corner + float2(0.0, 2.0)) / Size);
    const float4 Both   = t_Reach.GatherRed(s_Reach, (Corner + 2.0) / Size);

    const float Reaches[16] =
    {
        Lower.w, Lower.z, Right.w, Right.z,
        Lower.x, Lower.y, Right.x, Right.y,
        Upper.w, Upper.z, Both.w,  Both.z,
        Upper.x, Upper.y, Both.x,  Both.y
    };
#endif

    float4 Total  = 0.0;
    float  Weight = 0.0;

    [unroll]
    for (int Y = 0; Y < 4; ++Y)
    {
        [unroll]
        for (int X = 0; X < 4; ++X)
        {
            const float2 Uv = Input.Texture + (float2(X, Y) - 1.5) * Texel;

#if ZY_TIER >= 3
            const float Reach = Reaches[Y * 4 + X];
#else
            const float Reach = t_Reach.SampleLevel(s_Reach, Uv, 0).r;
#endif
            const float Near  = saturate(1.0 - abs(Reach - Along) / u_Settle.y);

            Total  += t_Occlusion.SampleLevel(s_Occlusion, Uv, 0) * Near;
            Weight += Near;
        }
    }
    const float4 Settled = Total / max(Weight, 1e-4);
    const float3 Open    = normalize(Settled.rgb * 2.0 - 1.0 + 1e-6);

    return float4(Open * 0.5 + 0.5, lerp(1.0, Settled.a, u_Settle.x));
}

#endif // FRAGMENT_SHADER