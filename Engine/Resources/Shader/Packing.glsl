// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PACKING_INCLUDED
#define ZY_PACKING_INCLUDED

/// \brief Reads the four bytes a color was packed into, lowest byte first as red.
///
/// \param Packed The four bytes the color was packed into.
///
/// \return The color the bytes stand for, each channel over zero through one.
vec4 ZyUnpackTint(uint Packed)
{
    uvec4 Bytes = uvec4(Packed, Packed >> 8u, Packed >> 16u, Packed >> 24u) & 0xFFu;

    return vec4(Bytes) * (1.0 / 255.0);
}

/// \brief Writes a color back into the four bytes \ref ZyUnpackTint reads it from.
///
/// \param Color The color to write, each channel over zero through one.
///
/// \return The four bytes the color packs into.
uint ZyPackTint(vec4 Color)
{
    uvec4 Bytes = uvec4(clamp(Color, 0.0, 1.0) * 255.0 + 0.5);

    return Bytes.x | (Bytes.y << 8u) | (Bytes.z << 16u) | (Bytes.w << 24u);
}

/// \brief Folds a direction onto the octahedron that carries it in two channels instead of three.
///
/// \note The direction has to be unit length, since the fold divides by the distance it walks along the axes.
///
/// \param Normal The direction to fold, at unit length.
///
/// \return The point on the octahedron, over negative one through one on both axes.
vec2 ZyEncodeOctahedral(vec3 Normal)
{
    vec3 Projected = Normal / (abs(Normal.x) + abs(Normal.y) + abs(Normal.z));
    vec2 Wrapped   = (1.0 - abs(Projected.yx)) * (2.0 * step(0.0, Projected.xy) - 1.0);

    return (Projected.z >= 0.0) ? Projected.xy : Wrapped;
}

/// \brief Unfolds the direction a point on the octahedron carries.
///
/// \param Encoded The point on the octahedron, over negative one through one on both axes.
///
/// \return The direction the point carries, at unit length.
vec3 ZyDecodeOctahedral(vec2 Encoded)
{
    vec3 Normal = vec3(Encoded, 1.0 - abs(Encoded.x) - abs(Encoded.y));

    Normal.xy -= max(-Normal.z, 0.0) * (2.0 * step(0.0, Normal.xy) - 1.0);

    return normalize(Normal);
}

/// \brief Reads the direction a normal map stores over zero through one.
///
/// \param Texel The texel the normal map holds.
///
/// \return The direction in tangent space, over negative one through one on every axis.
vec3 ZyDecodeNormalMap(vec3 Texel)
{
    return Texel * 2.0 - 1.0;
}

/// \brief Reads the direction a two channel normal map stores, standing the third axis back up.
///
/// \note This is what a map compressed to two channels needs.
///
/// \param Texel The two channels the normal map holds.
///
/// \return The direction in tangent space, over negative one through one on every axis.
vec3 ZyDecodeNormalMap(vec2 Texel)
{
    vec2 Tangent = Texel * 2.0 - 1.0;

    return vec3(Tangent, sqrt(max(1.0 - dot(Tangent, Tangent), 0.0)));
}

#endif // ZY_PACKING_INCLUDED