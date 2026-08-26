// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_DEPTH_INCLUDED
#define ZY_DEPTH_INCLUDED

/// \brief Reads the distance a perspective depth stands for.
///
/// \param Depth The depth the buffer holds.
/// \param Near  The distance to the near plane the depth was written against.
/// \param Far   The distance to the far plane the depth was written against.
///
/// \return The distance from the eye, in the units the projection was built with.
float ZyLinearizeDepth(float Depth, float Near, float Far)
{
    return (Near * Far) / (Far - Depth * (Far - Near));
}

/// \brief Reads the share of the range a perspective depth stands for.
///
/// \param Depth The depth the buffer holds.
/// \param Near  The distance to the near plane the depth was written against.
/// \param Far   The distance to the far plane the depth was written against.
///
/// \return The distance from the eye, over zero at the near plane through one at the far plane.
float ZyLinearizeDepth01(float Depth, float Near, float Far)
{
    return (ZyLinearizeDepth(Depth, Near, Far) - Near) / (Far - Near);
}

/// \brief Places the point a texture coordinate and a depth name back into the space an inverse names.
///
/// \param Uv      The texture coordinate the point was read at.
/// \param Depth   The depth the buffer holds there.
/// \param Inverse The inverse of the matrix the point was carried to clip space by.
///
/// \return The point, in the space the inverse leads to.
float3 ZyPositionFromDepth(float2 Uv, float Depth, float4x4 Inverse)
{
    const float4 Clip     = float4(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0, Depth, 1.0);
    const float4 Position = mul(Inverse, Clip);

    return Position.xyz / Position.w;
}

#endif // ZY_DEPTH_INCLUDED