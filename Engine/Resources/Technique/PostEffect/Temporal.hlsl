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
    float4 u_Shift;
    float4 u_Texel;
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

static const float kSpread = 1.25;

float3 Encode(float3 Color)
{
    return float3(
        dot(Color, float3( 0.25, 0.5,  0.25)),
        dot(Color, float3( 0.5,  0.0, -0.5)),
        dot(Color, float3(-0.25, 0.5, -0.25)));
}

float3 Decode(float3 Tone)
{
    return float3(Tone.x + Tone.y - Tone.z, Tone.x + Tone.z, Tone.x - Tone.y - Tone.z);
}

// What was kept is read through a Catmull-Rom filter in five taps, so sliding it by part of a texel never softens it.
float3 ReadKept(float2 Uv)
{
    const float2 Spot   = Uv / u_Texel.xy;
    const float2 Middle = floor(Spot - 0.5) + 0.5;
    const float2 Part   = Spot - Middle;

    const float2 W0  = Part * (-0.5 + Part * (1.0 - 0.5 * Part));
    const float2 W1  = 1.0 + Part * Part * (-2.5 + 1.5 * Part);
    const float2 W2  = Part * (0.5 + Part * (2.0 - 1.5 * Part));
    const float2 W3  = Part * Part * (-0.5 + 0.5 * Part);
    const float2 W12 = W1 + W2;

    const float2 T0  = (Middle - 1.0) * u_Texel.xy;
    const float2 T3  = (Middle + 2.0) * u_Texel.xy;
    const float2 T12 = (Middle + W2 / W12) * u_Texel.xy;

    float4 Sum = 0.0;

    Sum += float4(t_Level.SampleLevel(s_Level, float2(T12.x, T0.y),  0).rgb, 1.0) * (W12.x * W0.y);
    Sum += float4(t_Level.SampleLevel(s_Level, float2(T0.x,  T12.y), 0).rgb, 1.0) * (W0.x  * W12.y);
    Sum += float4(t_Level.SampleLevel(s_Level, float2(T12.x, T12.y), 0).rgb, 1.0) * (W12.x * W12.y);
    Sum += float4(t_Level.SampleLevel(s_Level, float2(T3.x,  T12.y), 0).rgb, 1.0) * (W3.x  * W12.y);
    Sum += float4(t_Level.SampleLevel(s_Level, float2(T12.x, T3.y),  0).rgb, 1.0) * (W12.x * W3.y);

    return max(Sum.rgb / Sum.a, 0.0);
}

float4 main(fs_Input Input) : SV_Target0
{
    const float3 Now = Encode(t_Scene.SampleLevel(s_Scene, Input.Texture, 0).rgb);

    // What was kept is held to the spread of colours the image has around the pixel, so nothing that left it lingers.
    float3 Sum    = 0.0;
    float3 Square = 0.0;

    for (int Y = -1; Y <= 1; ++Y)
    {
        for (int X = -1; X <= 1; ++X)
        {
            const float2 Uv   = Input.Texture + float2(X, Y) * u_Texel.xy;
            const float3 Near = Encode(t_Scene.SampleLevel(s_Scene, Uv, 0).rgb);

            Sum    += Near;
            Square += Near * Near;
        }
    }

    const float3 Mean   = Sum / 9.0;
    const float3 Spread = sqrt(max(Square / 9.0 - Mean * Mean, 0.0)) * kSpread;

    const float2 Uv     = (Input.Texture - 0.5) * u_Texel.zw + 0.5 + u_Shift.xy * float2(0.5, -0.5);
    const bool   Inside = all(Uv >= 0.0) && all(Uv <= 1.0);

    const float3 Away   = Encode(ReadKept(Uv)) - Mean;
    const float3 Reach  = abs(Away) / max(Spread, 1e-5);
    const float3 Kept   = Mean + Away / max(max(Reach.x, max(Reach.y, Reach.z)), 1.0);

    return float4(Decode(lerp(Kept, Now, Inside ? u_Shift.z : 1.0)), 1.0);
}

#endif // FRAGMENT_SHADER