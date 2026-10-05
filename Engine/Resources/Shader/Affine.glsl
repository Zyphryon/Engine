// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_AFFINE_INCLUDED
#define ZY_AFFINE_INCLUDED

/// An instance's three axes and the point it stands at.
struct ZyAffine
{
    /// The instance's X axis, in the space it is placed in.
    vec3 ColumnX;

    /// The instance's Y axis, in the space it is placed in.
    vec3 ColumnY;

    /// The instance's Z axis, in the space it is placed in.
    vec3 ColumnZ;

    /// The point the instance stands at.
    vec3 Origin;
};

/// Reads an affine from the three transposed rows an instance carries.
ZyAffine ZyReadAffine(vec4 Row0, vec4 Row1, vec4 Row2)
{
    ZyAffine Result;

    Result.ColumnX = vec3(Row0.x, Row1.x, Row2.x);
    Result.ColumnY = vec3(Row0.y, Row1.y, Row2.y);
    Result.ColumnZ = vec3(Row0.z, Row1.z, Row2.z);
    Result.Origin  = vec3(Row0.w, Row1.w, Row2.w);

    return Result;
}

/// Carries a point from the instance's own space into the space it is placed in.
vec3 ZyApplyAffine(ZyAffine Transform, vec3 Local)
{
    return Transform.Origin + Local.x * Transform.ColumnX + Local.y * Transform.ColumnY + Local.z * Transform.ColumnZ;
}

#endif // ZY_AFFINE_INCLUDED