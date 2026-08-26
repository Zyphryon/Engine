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
    float Clip = Depth * 2.0 - 1.0;

    return (2.0 * Near * Far) / (Far + Near - Clip * (Far - Near));
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

/// \brief Measures how much world one whole unit of clip depth spans.
///
/// \param Inverse The inverse of the matrix the point was carried to clip space by.
///
/// \return The world distance the depth range covers.
float ZyDepthSpan(mat4 Inverse)
{
    return length((Inverse * vec4(0.0, 0.0, 1.0, 0.0)).xyz);
}

/// \brief Reads the clip depth a value the depth buffer holds stands for.
///
/// \param Depth The depth the buffer holds, over zero through one.
///
/// \return The depth, in clip space.
float ZyClipDepth(float Depth)
{
    return Depth * 2.0 - 1.0;
}

/// \brief Places the point a texture coordinate and a depth name back into the space an inverse names.
///
/// \param Uv      The texture coordinate the point was read at.
/// \param Depth   The depth the buffer holds there.
/// \param Inverse The inverse of the matrix the point was carried to clip space by.
///
/// \return The point, in the space the inverse leads to.
vec3 ZyPositionFromDepth(vec2 Uv, float Depth, mat4 Inverse)
{
    vec4 Clip     = vec4(Uv * 2.0 - 1.0, ZyClipDepth(Depth), 1.0);
    vec4 Position = Inverse * Clip;

    return Position.xyz / Position.w;
}

#endif // ZY_DEPTH_INCLUDED