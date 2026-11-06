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
    vec4 u_Settle;
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
layout(binding = 2) uniform sampler2D t_Occlusion;

in vec2 v_Texture;

layout(location = 0) out vec4 out_Occlusion;

void main()
{
    vec2  Size  = vec2(textureSize(t_Reach, 0));
    float Along = textureLod(t_Reach, v_Texture, 0.0).r;
    vec2  Texel = 2.0 / vec2(textureSize(t_Normal, 0));

#if ZY_TIER >= 3
    vec2 Corner = floor(v_Texture * Size);
    vec4 Lower  = textureGather(t_Reach, Corner / Size);
    vec4 Right  = textureGather(t_Reach, (Corner + vec2(2.0, 0.0)) / Size);
    vec4 Upper  = textureGather(t_Reach, (Corner + vec2(0.0, 2.0)) / Size);
    vec4 Both   = textureGather(t_Reach, (Corner + 2.0) / Size);

    float Reaches[16] = float[16](
        Lower.w, Lower.z, Right.w, Right.z,
        Lower.x, Lower.y, Right.x, Right.y,
        Upper.w, Upper.z, Both.w,  Both.z,
        Upper.x, Upper.y, Both.x,  Both.y);
#endif

    vec4  Total  = vec4(0.0);
    float Weight = 0.0;

    for (int Y = 0; Y < 4; ++Y)
    {
        for (int X = 0; X < 4; ++X)
        {
            vec2 Uv = v_Texture + (vec2(X, Y) - 1.5) * Texel;

#if ZY_TIER >= 3
            float Reach = Reaches[Y * 4 + X];
#else
            float Reach = textureLod(t_Reach, Uv, 0.0).r;
#endif
            float Near  = clamp(1.0 - abs(Reach - Along) / u_Settle.y, 0.0, 1.0);

            Total  += textureLod(t_Occlusion, Uv, 0.0) * Near;
            Weight += Near;
        }
    }
    vec4 Settled = Total / max(Weight, 1e-4);
    vec3 Open    = normalize(Settled.rgb * 2.0 - 1.0 + 1e-6);

    out_Occlusion = vec4(Open * 0.5 + 0.5, mix(1.0, Settled.a, u_Settle.x));
}

#endif // FRAGMENT_SHADER