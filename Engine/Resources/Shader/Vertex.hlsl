// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_VERTEX_INCLUDED
#define ZY_VERTEX_INCLUDED

/// A corner of the 0..1 rectangle, drawn as a 4-vertex strip.
float2 ZyEmitRect(uint VertexID)
{
    const float2 kCorners[4] = {
        float2(0.0, 0.0),
        float2(1.0, 0.0),
        float2(0.0, 1.0),
        float2(1.0, 1.0)
    };

    return kCorners[VertexID];
}

/// A corner of the -1..1 rectangle, drawn as a 4-vertex strip.
float2 ZyEmitQuad(uint VertexID)
{
    return ZyEmitRect(VertexID) * 2.0 - 1.0;
}

/// The outline of the 0..1 rectangle, drawn as a 5-vertex line strip.
float2 ZyEmitRectOutline(uint VertexID)
{
    const float2 kCorners[5] = {
        float2(0.0, 0.0),
        float2(1.0, 0.0),
        float2(1.0, 1.0),
        float2(0.0, 1.0),
        float2(0.0, 0.0)
    };

    return kCorners[VertexID];
}

/// The edges of the -1..1 rectangle, drawn as an 8-vertex line list.
float2 ZyEmitQuadEdges(uint VertexID)
{
    const float2 kEdges[8] = {
        float2(-1.0, -1.0),
        float2( 1.0, -1.0),
        float2( 1.0, -1.0),
        float2( 1.0,  1.0),
        float2( 1.0,  1.0),
        float2(-1.0,  1.0),
        float2(-1.0,  1.0),
        float2(-1.0, -1.0)
    };

    return kEdges[VertexID];
}

/// A corner of the full-screen triangle, drawn with no buffer, as clip xy and texture uv.
float4 ZyEmitScreen(uint VertexID)
{
    const float2 Corner = float2(float((VertexID << 1) & 2), float(VertexID & 2));

    return float4(Corner * 2.0 - 1.0, Corner.x, 1.0 - Corner.y);
}

/// The edges of the -1..1 box, drawn as a 24-vertex line list.
float3 ZyEmitBox(uint VertexID)
{
    const float3 kEdges[24] = {
        float3(-1.0, -1.0, -1.0),
        float3( 1.0, -1.0, -1.0),
        float3(-1.0,  1.0, -1.0),
        float3( 1.0,  1.0, -1.0),
        float3(-1.0, -1.0,  1.0),
        float3( 1.0, -1.0,  1.0),
        float3(-1.0,  1.0,  1.0),
        float3( 1.0,  1.0,  1.0),
        float3(-1.0, -1.0, -1.0),
        float3(-1.0,  1.0, -1.0),
        float3( 1.0, -1.0, -1.0),
        float3( 1.0,  1.0, -1.0),
        float3(-1.0, -1.0,  1.0),
        float3(-1.0,  1.0,  1.0),
        float3( 1.0, -1.0,  1.0),
        float3( 1.0,  1.0,  1.0),
        float3(-1.0, -1.0, -1.0),
        float3(-1.0, -1.0,  1.0),
        float3( 1.0, -1.0, -1.0),
        float3( 1.0, -1.0,  1.0),
        float3(-1.0,  1.0, -1.0),
        float3(-1.0,  1.0,  1.0),
        float3( 1.0,  1.0, -1.0),
        float3( 1.0,  1.0,  1.0)
    };

    return kEdges[VertexID];
}

/// The wireframe of a -1..1 cylinder along Y, drawn as a line list.
float3 ZyEmitCylinder(uint VertexID, int Segments)
{
    const int   Line = int(VertexID / 2u);
    const int   End  = int(VertexID & 1u);
    const float Turn = 6.28318530718 / float(Segments);

    if (Line < 2 * Segments)
    {
        const float Theta = Turn * float((Line % Segments) + End);
        return float3(cos(Theta), (Line / Segments) == 0 ? -1.0 : 1.0, sin(Theta));
    }

    const float Theta = 1.57079632679 * float(Line - 2 * Segments);
    return float3(cos(Theta), End == 0 ? -1.0 : 1.0, sin(Theta));
}

/// The solid -1..1 box, drawn as a 14-vertex strip.
float3 ZyEmitBoxSolid(uint VertexID)
{
    const float3 kStrip[14] = {
        float3(-1.0,  1.0,  1.0),
        float3( 1.0,  1.0,  1.0),
        float3(-1.0, -1.0,  1.0),
        float3( 1.0, -1.0,  1.0),
        float3( 1.0, -1.0, -1.0),
        float3( 1.0,  1.0,  1.0),
        float3( 1.0,  1.0, -1.0),
        float3(-1.0,  1.0,  1.0),
        float3(-1.0,  1.0, -1.0),
        float3(-1.0, -1.0,  1.0),
        float3(-1.0, -1.0, -1.0),
        float3( 1.0, -1.0, -1.0),
        float3(-1.0,  1.0, -1.0),
        float3( 1.0,  1.0, -1.0)
    };

    return kStrip[VertexID];
}

#endif // ZY_VERTEX_INCLUDED