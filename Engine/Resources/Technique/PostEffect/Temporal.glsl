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
    vec4 u_Shift;
    vec4 u_Texel;
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

const float kSpread = 1.25;

vec3 Encode(vec3 Color)
{
    return vec3(
        dot(Color, vec3( 0.25, 0.5,  0.25)),
        dot(Color, vec3( 0.5,  0.0, -0.5)),
        dot(Color, vec3(-0.25, 0.5, -0.25)));
}

vec3 Decode(vec3 Tone)
{
    return vec3(Tone.x + Tone.y - Tone.z, Tone.x + Tone.z, Tone.x - Tone.y - Tone.z);
}

// What was kept is read through a Catmull-Rom filter in five taps, so sliding it by part of a texel never softens it.
vec3 ReadKept(vec2 Uv)
{
    vec2 Spot   = Uv / u_Texel.xy;
    vec2 Middle = floor(Spot - 0.5) + 0.5;
    vec2 Part   = Spot - Middle;

    vec2 W0  = Part * (-0.5 + Part * (1.0 - 0.5 * Part));
    vec2 W1  = 1.0 + Part * Part * (-2.5 + 1.5 * Part);
    vec2 W2  = Part * (0.5 + Part * (2.0 - 1.5 * Part));
    vec2 W3  = Part * Part * (-0.5 + 0.5 * Part);
    vec2 W12 = W1 + W2;

    vec2 T0  = (Middle - 1.0) * u_Texel.xy;
    vec2 T3  = (Middle + 2.0) * u_Texel.xy;
    vec2 T12 = (Middle + W2 / W12) * u_Texel.xy;

    vec4 Sum = vec4(0.0);

    Sum += vec4(textureLod(t_Level, vec2(T12.x, T0.y),  0.0).rgb, 1.0) * (W12.x * W0.y);
    Sum += vec4(textureLod(t_Level, vec2(T0.x,  T12.y), 0.0).rgb, 1.0) * (W0.x  * W12.y);
    Sum += vec4(textureLod(t_Level, vec2(T12.x, T12.y), 0.0).rgb, 1.0) * (W12.x * W12.y);
    Sum += vec4(textureLod(t_Level, vec2(T3.x,  T12.y), 0.0).rgb, 1.0) * (W3.x  * W12.y);
    Sum += vec4(textureLod(t_Level, vec2(T12.x, T3.y),  0.0).rgb, 1.0) * (W12.x * W3.y);

    return max(Sum.rgb / Sum.a, vec3(0.0));
}

void main()
{
    vec3 Now = Encode(textureLod(t_Scene, v_Texture, 0.0).rgb);

    // What was kept is held to the spread of colours the image has around the pixel, so nothing that left it lingers.
    vec3 Sum    = vec3(0.0);
    vec3 Square = vec3(0.0);

    for (int Y = -1; Y <= 1; ++Y)
    {
        for (int X = -1; X <= 1; ++X)
        {
            vec3 Near = Encode(textureLod(t_Scene, v_Texture + vec2(float(X), float(Y)) * u_Texel.xy, 0.0).rgb);

            Sum    += Near;
            Square += Near * Near;
        }
    }

    vec3 Mean   = Sum / 9.0;
    vec3 Spread = sqrt(max(Square / 9.0 - Mean * Mean, vec3(0.0))) * kSpread;

    vec2 Uv     = (v_Texture - 0.5) * u_Texel.zw + 0.5 + u_Shift.xy * 0.5;
    bool Inside = all(greaterThanEqual(Uv, vec2(0.0))) && all(lessThanEqual(Uv, vec2(1.0)));

    vec3 Away   = Encode(ReadKept(Uv)) - Mean;
    vec3 Reach  = abs(Away) / max(Spread, vec3(1e-5));
    vec3 Kept   = Mean + Away / max(max(Reach.x, max(Reach.y, Reach.z)), 1.0);

    out_Color = vec4(Decode(mix(Kept, Now, Inside ? u_Shift.z : 1.0)), 1.0);
}

#endif // FRAGMENT_SHADER