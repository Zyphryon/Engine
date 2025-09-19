// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// Parameters

cbuffer cb_Global : register(b0)
{
    float4x4 u_Camera;
};

// Attributes

struct vs_Input
{
    uint   VertexID : SV_VertexID;
    float3 Center   : POSITION;
    float4 Color    : COLOR;
    float2 Data0    : CUSTOM0;
    float2 Data1    : CUSTOM1;
};

struct ps_Input
{
    float4 Position  : SV_POSITION;
    float4 Color     : COLOR0;

#ifdef USE_SDF
    float2 Local     : CUSTOM0;
#endif

#if   defined(SHAPE_RING) || defined(SHAPE_LINE) || defined(SHAPE_ROUNDED_RECT)
    float  Thickness : CUSTOM1;
#endif
};

// VS Main

float2 TessellateRect(uint VertexID)
{
    const float2 kUnitRectCorners[4] = {
        float2(-1,-1),
        float2( 1,-1),
        float2(-1, 1),
        float2( 1, 1)
    };
    return kUnitRectCorners[VertexID];
}

ps_Input vertex(vs_Input Input)
{
    float2 Offset;
    float2 Local = TessellateRect(Input.VertexID);

    ps_Input Result;

#if   defined(SHAPE_CIRCLE) || defined(SHAPE_RING)

    #ifdef SHAPE_RING
        Result.Thickness = Input.Data0.y * 0.5;

        Local *= 1.0 + Result.Thickness;
    #endif

    Offset = Local * float2(Input.Data0.xx);

#elif defined(SHAPE_RECT) || defined(SHAPE_ROUNDED_RECT)

    Offset = Local * Input.Data0;

    #ifdef SHAPE_ROUNDED_RECT
        Result.Thickness = Input.Data1.x;
    #endif

#elif defined(SHAPE_LINE)

    Result.Thickness = Input.Data1.y * 0.5;

    Offset = Input.Data0 * (Local.x * Input.Data1.x) + float2(-Input.Data0.y, Input.Data0.x) * (Local.y * Result.Thickness);

    Local *= 1.0 + Result.Thickness;

#endif

    Result.Position = mul(u_Camera, float4(Input.Center + float3(Offset, 0.0), 1.f));
    Result.Color    = Input.Color;

#ifdef USE_SDF
    Result.Local    = Local;
#endif

    return Result;
}

// PS Main

float sdCircle(float2 Point, float Radius)
{
    return sqrt(dot(Point, Point)) - Radius;
}

float sdRing(float2 Point, float Radius, float Thickness)
{
    return abs(length(Point) - Radius) - Thickness;
}

float sdLine(float2 Point, float Thickness)
{
    return abs(Point.y) - Thickness;
}

float sdRoundedRect(float2 Point, float Radius)
{
    float2 q = abs(Point) - 1.0 + Radius;
    return length(max(q, 0.0)) - Radius + min(max(q.x, q.y), 0.0);
}

float4 fragment(ps_Input Input) : SV_Target
{
    float4 Result = Input.Color;

#ifdef USE_SDF

    float Distance = 0.0f;

    #if   defined(SHAPE_CIRCLE)
        Distance = sdCircle(Input.Local, 1.0);
    #elif defined(SHAPE_RING)
        Distance = sdRing(Input.Local, 1.0, Input.Thickness);
    #elif defined(SHAPE_LINE)
        Distance = sdLine(Input.Local, normalizedThickness);
    #elif defined(SHAPE_ROUNDED_RECT)
        Distance = sdRoundedRect(Input.Local, Input.Thickness);
    #endif

    #if defined(USE_AA)
        float Antialias = fwidth(Distance);
        Result.a *= 1.0 - smoothstep(-Antialias, Antialias, Distance);
    #else
        Result.a *= step(Distance, 0.5);
    #endif

    #if defined(SHAPE_CIRCLE) || defined(SHAPE_RING) || defined(SHAPE_ROUNDED_RECT)
        clip(Result.a - 0.001);
    #endif

#endif

    Result.rgb *= Result.a;
    return Result;
}
