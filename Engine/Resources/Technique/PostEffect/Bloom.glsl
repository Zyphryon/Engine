// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.glsl"
#include "Embedded://Shader/Math.glsl"

layout(std140, binding = 1) uniform cb_Pass
{
    vec4 u_Filter;
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef VERTEX_SHADER

out vec2 v_Texture;

void main()
{
    vec4 Screen = ZyEmitScreen(gl_VertexID);

    gl_Position = vec4(Screen.xy, 0.0, 1.0);
    v_Texture   = Screen.zw;
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

layout(binding = 0) uniform sampler2D t_Scene;
layout(binding = 1) uniform sampler2D t_Level;

in vec2 v_Texture;

layout(location = 0) out vec4 out_Color;

vec3 Tap(vec2 Uv, vec2 Offset)
{
    return textureLod(t_Scene, Uv + Offset * u_Filter.zw, 0.0).rgb;
}

#if defined(BLOOM_UP)

// A three by three tent over the level below, one of its texels apart, which spreads it without a seam.
vec3 Spread(vec2 Uv)
{
    vec3 Result = Tap(Uv, vec2(0.0, 0.0)) * 4.0;

    Result += (Tap(Uv, vec2(-1.0,  0.0)) + Tap(Uv, vec2(1.0, 0.0))
            +  Tap(Uv, vec2( 0.0, -1.0)) + Tap(Uv, vec2(0.0, 1.0))) * 2.0;
    Result += (Tap(Uv, vec2(-1.0, -1.0)) + Tap(Uv, vec2(1.0, -1.0))
            +  Tap(Uv, vec2(-1.0,  1.0)) + Tap(Uv, vec2(1.0,  1.0)));

    return Result * (1.0 / 16.0);
}

#else

// Thirteen taps over the level above, weighted so a bright point lands in the half the same whichever way it moves.
vec3 Halve(vec2 Uv)
{
    vec3 Middle  = Tap(Uv, vec2( 0.0,  0.0));
    vec3 Inner   = Tap(Uv, vec2(-1.0, -1.0)) + Tap(Uv, vec2(1.0, -1.0))
                 + Tap(Uv, vec2(-1.0,  1.0)) + Tap(Uv, vec2(1.0,  1.0));
    vec3 Corners = Tap(Uv, vec2(-2.0, -2.0)) + Tap(Uv, vec2(2.0, -2.0))
                 + Tap(Uv, vec2(-2.0,  2.0)) + Tap(Uv, vec2(2.0,  2.0));
    vec3 Sides   = Tap(Uv, vec2( 0.0, -2.0)) + Tap(Uv, vec2(0.0,  2.0))
                 + Tap(Uv, vec2(-2.0,  0.0)) + Tap(Uv, vec2(2.0,  0.0));

    return Middle * 0.125 + Inner * 0.125 + Corners * 0.03125 + Sides * 0.0625;
}

#endif // BLOOM_UP

#if defined(BLOOM_SIFT)

// Keeps what clears the threshold, eased in over the knee.
vec3 Extract(vec3 Color)
{
    float Peak = ZyMax3(Color.r, Color.g, Color.b);
    float Soft = clamp(Peak - u_Filter.x + u_Filter.y, 0.0, 2.0 * u_Filter.y);
    float Gain = max(Peak - u_Filter.x, Soft * Soft / (4.0 * u_Filter.y + 0.0001));

    return Color * (Gain / max(Peak, 0.0001));
}

#endif // BLOOM_SIFT

void main()
{
#if defined(BLOOM_UP)
    vec3 Result = Spread(v_Texture) + textureLod(t_Level, v_Texture, 0.0).rgb;
#elif defined(BLOOM_SIFT)
    vec3 Result = Extract(Halve(v_Texture));
#else
    vec3 Result = Halve(v_Texture);
#endif

    out_Color = vec4(Result, 1.0);
}

#endif // FRAGMENT_SHADER