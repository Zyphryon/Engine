// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.glsl"

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

const float kKey   = 0.18;
const float kShare = 0.5;

#if defined(EXPOSURE_ADAPT)

float Adapt()
{
    ivec2 Size  = textureSize(t_Scene, 0);
    vec2  Total = vec2(0.0);

    for (int Y = 0; Y < Size.y; ++Y)
    {
        for (int X = 0; X < Size.x; ++X)
        {
            Total += texelFetch(t_Scene, ivec2(X, Y), 0).rg;
        }
    }

    float Mean   = Total.x / max(Total.y, 0.0001);
    float Target = clamp(kShare * (log2(kKey) - Mean), u_Filter.z, u_Filter.w);
    float Last   = texelFetch(t_Level, ivec2(0, 0), 0).r;

    if (!(abs(Last) < 64.0))
    {
        Last = Target;
    }
    return mix(Last, Target, Target < Last ? u_Filter.x : u_Filter.y);
}

#elif !defined(EXPOSURE_KEEP)

vec2 Measure(vec2 Uv)
{
    float Total = 0.0;

    for (int Y = 0; Y < 4; ++Y)
    {
        for (int X = 0; X < 4; ++X)
        {
            vec2 Offset = (vec2(X, Y) - 1.5) * 0.25 * u_Filter.xy;
            vec3 Color  = textureLod(t_Scene, Uv + Offset, 0.0).rgb;

            Total += log2(max(dot(Color, vec3(0.2126, 0.7152, 0.0722)), 0.0001));
        }
    }

    vec2  Middle = (Uv - 0.5) * 2.0;
    float Weight = exp(-2.0 * dot(Middle, Middle));

    return vec2(Total * (1.0 / 16.0) * Weight, Weight);
}

#endif

void main()
{
#if defined(EXPOSURE_ADAPT)
    out_Color = vec4(Adapt(), 0.0, 0.0, 1.0);
#elif defined(EXPOSURE_KEEP)
    out_Color = texelFetch(t_Scene, ivec2(0, 0), 0);
#else
    out_Color = vec4(Measure(v_Texture), 0.0, 1.0);
#endif
}

#endif // FRAGMENT_SHADER