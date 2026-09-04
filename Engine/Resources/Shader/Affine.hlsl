// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_AFFINE_INCLUDED
#define ZY_AFFINE_INCLUDED

/// \brief Represents the axes an instance is laid down along, and the point it stands at.
struct ZyAffine
{
    /// The direction the instance's own X travels, in the space it is placed into.
    float3 ColumnX;

    /// The direction the instance's own Y travels, in the space it is placed into.
    float3 ColumnY;

    /// The direction the instance's own Z travels, in the space it is placed into.
    float3 ColumnZ;

    /// The point the instance stands at.
    float3 Origin;
};

/// \brief Reads an affine out of the three rows an instance carries it as.
///
/// \note Each row holds one component of every column, which is the transposed layout a `Matrix4x3` is named in.
///
/// \param Row0 The first row of the affine.
/// \param Row1 The second row of the affine.
/// \param Row2 The third row of the affine.
///
/// \return The axes the instance is laid down along, and the point it stands at.
ZyAffine ZyReadAffine(float4 Row0, float4 Row1, float4 Row2)
{
    ZyAffine Result;

    Result.ColumnX = float3(Row0.x, Row1.x, Row2.x);
    Result.ColumnY = float3(Row0.y, Row1.y, Row2.y);
    Result.ColumnZ = float3(Row0.z, Row1.z, Row2.z);
    Result.Origin  = float3(Row0.w, Row1.w, Row2.w);

    return Result;
}

/// \brief Carries a point out of an instance's own space and into the one it is placed in.
///
/// \param Transform The axes the instance is laid down along, and the point it stands at.
/// \param Local     The point to carry, in the instance's own space.
///
/// \return The point, in the space the instance is placed in.
float3 ZyApplyAffine(ZyAffine Transform, float3 Local)
{
    return Transform.Origin + Local.x * Transform.ColumnX + Local.y * Transform.ColumnY + Local.z * Transform.ColumnZ;
}

#endif // ZY_AFFINE_INCLUDED