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
vec2 ZyEmitRect(int VertexID)
{
    const vec2 kCorners[4] = vec2[4](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(0.0, 1.0),
        vec2(1.0, 1.0));

    return kCorners[VertexID];
}

/// A corner of the -1..1 rectangle, drawn as a 4-vertex strip.
vec2 ZyEmitQuad(int VertexID)
{
    return ZyEmitRect(VertexID) * 2.0 - 1.0;
}

/// The outline of the 0..1 rectangle, drawn as a 5-vertex line strip.
vec2 ZyEmitRectOutline(int VertexID)
{
    const vec2 kCorners[5] = vec2[5](
        vec2(0.0, 0.0),
        vec2(1.0, 0.0),
        vec2(1.0, 1.0),
        vec2(0.0, 1.0),
        vec2(0.0, 0.0));

    return kCorners[VertexID];
}

/// The edges of the -1..1 rectangle, drawn as an 8-vertex line list.
vec2 ZyEmitQuadEdges(int VertexID)
{
    const vec2 kEdges[8] = vec2[8](
        vec2(-1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2( 1.0, -1.0),
        vec2( 1.0,  1.0),
        vec2( 1.0,  1.0),
        vec2(-1.0,  1.0),
        vec2(-1.0,  1.0),
        vec2(-1.0, -1.0));

    return kEdges[VertexID];
}

/// A corner of the full-screen triangle, drawn with no buffer, as clip xy and texture uv.
vec4 ZyEmitScreen(int VertexID)
{
    vec2 Corner = vec2(float((VertexID << 1) & 2), float(VertexID & 2));

    return vec4(Corner * 2.0 - 1.0, Corner.x, Corner.y);
}

/// The edges of the -1..1 box, drawn as a 24-vertex line list.
vec3 ZyEmitBox(int VertexID)
{
    const vec3 kEdges[24] = vec3[24](
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3( 1.0,  1.0,  1.0));

    return kEdges[VertexID];
}

/// The wireframe of a -1..1 cylinder along Y, drawn as a line list.
vec3 ZyEmitCylinder(int VertexID, int Segments)
{
    int   Line = VertexID / 2;
    int   End  = VertexID & 1;
    float Turn = 6.28318530718 / float(Segments);

    if (Line < 2 * Segments)
    {
        float Theta = Turn * float((Line % Segments) + End);
        return vec3(cos(Theta), (Line / Segments) == 0 ? -1.0 : 1.0, sin(Theta));
    }

    float Theta = 1.57079632679 * float(Line - 2 * Segments);
    return vec3(cos(Theta), End == 0 ? -1.0 : 1.0, sin(Theta));
}

/// The solid -1..1 box, drawn as a 14-vertex strip.
vec3 ZyEmitBoxSolid(int VertexID)
{
    const vec3 kStrip[14] = vec3[14](
        vec3(-1.0,  1.0,  1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3( 1.0, -1.0,  1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3( 1.0,  1.0,  1.0),
        vec3( 1.0,  1.0, -1.0),
        vec3(-1.0,  1.0,  1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3(-1.0, -1.0,  1.0),
        vec3(-1.0, -1.0, -1.0),
        vec3( 1.0, -1.0, -1.0),
        vec3(-1.0,  1.0, -1.0),
        vec3( 1.0,  1.0, -1.0));

    return kStrip[VertexID];
}

#endif // ZY_VERTEX_INCLUDED