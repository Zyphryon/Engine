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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex Codec
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef   VERTEX_SHADER

in vec3   a_Center;
in vec4   a_Color;
in vec4   a_Data;

out vec4  v_Color;

#ifdef USE_SDF
out vec2  v_Local;
#endif

#if defined(SHAPE_RING) || defined(SHAPE_LINE) || defined(SHAPE_ROUNDED_RECT)
out float v_Thickness;
#endif

vec2 TessellateRect(int VertexID)
{
    const vec2 kUnitRectCorners[4] = vec2[4](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2(-1.0,  1.0),
        vec2( 1.0,  1.0)
    );
    return kUnitRectCorners[VertexID];
}

void main()
{
    vec2 Offset;
    vec2 Local = TessellateRect(gl_VertexID);

#if   defined(SHAPE_CIRCLE) || defined(SHAPE_RING)

    #ifdef SHAPE_RING
        v_Thickness = a_Data.y * 0.5;

        Local *= 1.0 + v_Thickness;
    #endif

    Offset = Local * a_Data.xx;

#elif defined(SHAPE_RECT) || defined(SHAPE_ROUNDED_RECT)

    Offset = Local * a_Data.xy;

    #ifdef SHAPE_ROUNDED_RECT
        v_Thickness = a_Data.z;
    #endif

#elif defined(SHAPE_LINE)

    v_Thickness = a_Data.w * 0.5;

    Offset = a_Data.xy * (Local.x * a_Data.z) + vec2(-a_Data.y, a_Data.x) * (Local.y * v_Thickness);

    Local *= 1.0 + v_Thickness;   // TODO: Fix Thickness

#endif

    gl_Position = u_Camera * vec4(a_Center + vec3(Offset, 0.0), 1.0);
    v_Color = a_Color;

#ifdef USE_SDF
    v_Local = Local;
#endif
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment Codec
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

in vec4  v_Color;

#ifdef USE_SDF
in vec2  v_Local;
#endif

#if defined(SHAPE_RING) || defined(SHAPE_LINE) || defined(SHAPE_ROUNDED_RECT)
in float v_Thickness;
#endif

layout(location = 0) out vec4 out_Color;

float sdCircle(vec2 Point, float Radius)
{
    return length(Point) - Radius;
}

float sdRing(vec2 Point, float Radius, float Thickness)
{
    return abs(length(Point) - Radius) - Thickness;
}

float sdLine(vec2 Point, float Thickness)
{
    return abs(Point.y) - Thickness;
}

float sdRoundedRect(vec2 Point, float Radius)
{
    vec2 q = abs(Point) - 1.0 + Radius;
    return length(max(q, 0.0)) - Radius + min(max(q.x, q.y), 0.0);
}

void main()
{
    vec4  Result = v_Color;

#ifdef USE_SDF

    float Distance = 0.0;

    #if   defined(SHAPE_CIRCLE)
        Distance = sdCircle(v_Local, 1.0);
    #elif defined(SHAPE_RING)
        Distance = sdRing(v_Local, 1.0, v_Thickness);
    #elif defined(SHAPE_LINE)
        Distance = sdLine(v_Local, v_Thickness);
    #elif defined(SHAPE_ROUNDED_RECT)
        Distance = sdRoundedRect(v_Local, v_Thickness);
    #endif

    #if defined(USE_AA)
        float Antialias = fwidth(Distance);
        Result.a *= smoothstep(Antialias, -Antialias, Distance);
    #else
        Result.a *= step(Distance, 0.0);
    #endif

    #if defined(SHAPE_CIRCLE) || defined(SHAPE_RING) || defined(SHAPE_ROUNDED_RECT)
        if (Result.a - 0.001 < 0.0)
        {
            discard;
        }
    #endif

#endif

    out_Color = vec4(Result.rgb * Result.a, Result.a);
}

#endif // FRAGMENT_SHADER