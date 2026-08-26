// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_VERTEX_INCLUDED
#define ZY_VERTEX_INCLUDED

/// \brief Places one corner of a unit rectangle, drawn as a strip of four.
///
/// \param VertexID The corner to place, as `SV_VertexID` counts them.
///
/// \return The corner, over zero through one on both axes.
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

/// \brief Places one corner of a rectangle centred on the origin, drawn as a strip of four.
///
/// \param VertexID The corner to place, as `SV_VertexID` counts them.
///
/// \return The corner, over negative one through one on both axes.
float2 ZyEmitQuad(uint VertexID)
{
    return ZyEmitRect(VertexID) * 2.0 - 1.0;
}

/// \brief Traces the outline of a unit rectangle, drawn as a line strip of five.
///
/// \param VertexID The corner to trace, as `SV_VertexID` counts them.
///
/// \return The corner, over zero through one on both axes.
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

/// \brief Places one corner of the triangle that covers the screen, drawn as three vertices and no buffer.
///
/// \param VertexID The corner to place, as `SV_VertexID` counts them.
///
/// \return The clip position and the texture coordinate that reads through it.
float4 ZyEmitScreen(uint VertexID)
{
    const float2 Corner = float2(float((VertexID << 1) & 2), float(VertexID & 2));

    return float4(Corner * 2.0 - 1.0, Corner.x, 1.0 - Corner.y);
}

/// \brief Traces the twelve edges of a box centred on the origin, drawn as a line list of twenty-four.
///
/// \param VertexID The endpoint to trace, as `SV_VertexID` counts them.
///
/// \return The endpoint, over negative one through one on every axis.
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

/// \brief Fills a box centred on the origin, drawn as a single strip of fourteen.
///
/// \param VertexID The corner to fill, as `SV_VertexID` counts them.
///
/// \return The corner, over negative one through one on every axis.
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