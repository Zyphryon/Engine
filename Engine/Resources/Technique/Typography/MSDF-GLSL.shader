// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Uniforms
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

layout(std140, binding = 0) uniform cb_Scene
{
    mat4 u_Camera;
};

layout(std140, binding = 2) uniform cb_Material
{
    vec2 u_Range;
};

struct PackedFontParameters
{
    vec4  u_OutsetTint;
    float u_OutsetOffset;
    float u_OutsetWidth;
    float u_OutsetBias;
    float u_OutsetBlur;
    float u_InsetRoundness;
    float u_InsetThreshold;
};

layout(std140, binding = 3) uniform cb_Object
{
    PackedFontParameters u_Parameters[64];
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex Codec
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef VERTEX_SHADER

in vec4 a_Transform0;
in vec4 a_Transform1;
in vec4 a_Frame;
in vec4 a_Local;
in vec4 a_Color;

out vec4 v_Color;
out vec2 v_Texture;
flat out uint v_Effect;

vec2 TessellateRect(int VertexID)
{
    vec2 kUnitRectCorners[4] = vec2[](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0)
    );
    return kUnitRectCorners[VertexID];
}

void main()
{
    vec2 Corner   = TessellateRect(gl_VertexID);
    vec2 Local    = a_Local.xy + Corner * a_Local.zw;
    vec2 Position = vec2(
        dot(Local, a_Transform0.xy) + a_Transform0.z,
        dot(Local, a_Transform1.xy) + a_Transform1.z
    );

    gl_Position = u_Camera * vec4(Position, a_Transform0.w, 1.0);

    v_Texture = mix(a_Frame.xy, a_Frame.zw, Corner);
    v_Color   = a_Color;
    v_Effect  = floatBitsToUint(a_Transform1.w);
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment Codec
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

layout(location = 0) out vec4 out_Color;

uniform sampler2D s_Albedo;

in vec4 v_Color;
in vec2 v_Texture;
flat in uint v_Effect;

float Median(vec3 Color)
{
    return max(min(Color.r, Color.g), min(max(Color.r, Color.g), Color.b));
}

float Spread(vec2 Coordinates, vec2 Unit)
{
    return max(dot(Unit, vec2(1.0) / fwidth(Coordinates)) * 0.5, 1.0);
}

void main()
{
    PackedFontParameters Font = u_Parameters[v_Effect];

    vec4  Sample       = texture(s_Albedo, v_Texture);
    float DistanceSDF  = Sample.a;
    float DistanceMSDF = Median(Sample.rgb);

    // Convert atlas distance into normalized screen-space range.
    float Scale = Spread(v_Texture, u_Range);

    // Interpolated distance field depending on rounded vs sharp style.
    float StrokeDistance = mix(DistanceMSDF, DistanceSDF, Font.u_InsetRoundness);
    float StrokeBase     = StrokeDistance - Font.u_InsetThreshold;

    // Convert distance to alpha coverage.
    float InnerStrokeA = Scale * StrokeBase + 0.5 + Font.u_OutsetOffset;
    float OuterStrokeA = Scale * (StrokeBase + Font.u_OutsetWidth) + 0.5 + Font.u_OutsetOffset + Font.u_OutsetBias;

    vec4 InnerColor    = v_Color;
    vec4 OuterColor    = Font.u_OutsetTint;
    float InnerOpacity = clamp(InnerStrokeA, 0.0, 1.0);
    float OuterOpacity = clamp(OuterStrokeA, 0.0, 1.0);

    // Optional: Apply Blur Effect
    float BlurStart  = Font.u_OutsetWidth + Font.u_OutsetBias / Scale;
    float BlurEnd    = BlurStart * (1.0 - Font.u_OutsetBlur);
    float BlurDist   = Font.u_InsetThreshold - DistanceSDF - Font.u_OutsetOffset / Scale;
    float BlurFactor = mix(1.0, smoothstep(BlurStart, BlurEnd, BlurDist), step(0.0001, Font.u_OutsetBlur));
    OuterColor.a *= BlurFactor;

    // Apply Gamma Correction.
    InnerOpacity = pow(InnerOpacity, 1.0 / 2.2);
    OuterOpacity = pow(OuterOpacity, 1.0 / 2.2);

    out_Color = InnerColor * InnerOpacity + OuterColor * (OuterOpacity - InnerOpacity);
}

#endif