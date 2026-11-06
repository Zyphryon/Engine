// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.glsl"
#include "Embedded://Shader/Packing.glsl"

layout(std140, binding = 1) uniform cb_Pass
{
    mat4 u_Inverse;
    vec4 u_Toward;
    vec4 u_Eye;
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

layout(binding = 0) uniform sampler2D t_Normal;
layout(binding = 1) uniform highp sampler2D t_Reach;

in vec2 v_Texture;

layout(location = 0) out vec4 out_Occlusion;

vec3 Locate(vec2 Clip, float Along)
{
#if defined(HORIZON_PERSPECTIVE)
    vec4 Near  = u_Inverse * vec4(Clip, 0.0, 1.0);
    vec3 Start = Near.xyz / Near.w;
    vec3 Sight = Start - u_Eye.xyz;

    return Start + Sight * (Along / dot(Sight, u_Toward.xyz));
#else
    vec3 Start = (u_Inverse * vec4(Clip, 0.0, 1.0)).xyz;

    return Start + u_Toward.xyz * Along;
#endif
}

vec3 Fetch(vec2 Pixel)
{
    // A sample stands where its texel's middle does, so a flat ground never reads as steps that close it in.
    vec2 Uv = (floor(Pixel) + 0.5) / vec2(textureSize(t_Reach, 0));

    return Locate(vec2(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0), textureLod(t_Reach, Uv, 0.0).r);
}

#define ZY_OCCLUSION_READ(Pixel) Fetch(Pixel)

#include "Embedded://Shader/Occlusion.glsl"

void main()
{
    vec2  Size  = vec2(textureSize(t_Reach, 0));
    float Along = textureLod(t_Reach, v_Texture, 0.0).r;

    if (Along >= ZY_OCCLUSION_OPEN)
    {
        out_Occlusion = vec4(0.5, 1.0, 0.5, 1.0);
        return;
    }

    vec2 Pixel  = v_Texture * Size;
    vec3 Center = Fetch(Pixel);
    vec3 Normal = normalize(ZyDecodeNormalMap(textureLod(t_Normal, v_Texture, 0.0).rgb));

#if defined(HORIZON_PERSPECTIVE)
    vec2 Uv     = (floor(Pixel) + 0.5) / Size;
    vec2 Clip   = vec2(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0);
    vec3 Across = Locate(Clip + vec2(2.0 / Size.x, 0.0), Along) - Center;
    vec3 Down   = Locate(Clip - vec2(0.0, 2.0 / Size.y), Along) - Center;
    vec3 Viewer = normalize(u_Eye.xyz - Center);
#else
    vec3 Across = (u_Inverse * vec4(2.0 / Size.x, 0.0, 0.0, 0.0)).xyz;
    vec3 Down   = (u_Inverse * vec4(0.0, -2.0 / Size.y, 0.0, 0.0)).xyz;
    vec3 Viewer = -u_Toward.xyz;
#endif

    vec3  Bent;
    float Visible = ZyHorizonOcclusion(
        Center, Normal, Viewer, Pixel, Across, Down, u_Toward.w, ZyOcclusionNoise(gl_FragCoord.xy), Bent);

    out_Occlusion = vec4(Bent * 0.5 + 0.5, Visible);
}

#endif // FRAGMENT_SHADER