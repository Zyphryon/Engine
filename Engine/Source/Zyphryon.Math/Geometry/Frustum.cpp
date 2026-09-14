// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Frustum.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(Vector3 Point) const
    {
        const Vector4 Homogeneous(Point.GetX(), Point.GetY(), Point.GetZ(), 1.0f);

        for (ConstRef<Vector4> Plane : mPlanes)
        {
            if (Vector4::Dot(Plane, Homogeneous) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(ConstRef<Sphere> Volume) const
    {
        const Vector3 Center = Volume.GetCenter();
        const Vector4 Homogeneous(Center.GetX(), Center.GetY(), Center.GetZ(), 1.0f);

        for (ConstRef<Vector4> Plane : mPlanes)
        {
            if (Vector4::Dot(Plane, Homogeneous) < -Volume.GetRadius())
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(ConstRef<Box> Volume) const
    {
        const Vector3 Minimum = Volume.GetMinimum();
        const Vector3 Maximum = Volume.GetMaximum();

        for (ConstRef<Vector4> Plane : mPlanes)
        {
            const Vector4 Corner(
                Plane.GetX() >= 0.0f ? Maximum.GetX() : Minimum.GetX(),
                Plane.GetY() >= 0.0f ? Maximum.GetY() : Minimum.GetY(),
                Plane.GetZ() >= 0.0f ? Maximum.GetZ() : Minimum.GetZ(),
                1.0f);

            if (Vector4::Dot(Plane, Corner) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Frustum Frustum::FromMatrix(ConstRef<Matrix4x4> ViewProjection)
    {
        // A clip coordinate is a row of the matrix dotted with the point, and the rows are the transpose's columns.
        const Matrix4x4 Rows = Matrix4x4::Transpose(ViewProjection);

        const Vector4 R0 = Rows.GetColumn(0);
        const Vector4 R1 = Rows.GetColumn(1);
        const Vector4 R2 = Rows.GetColumn(2);
        const Vector4 R3 = Rows.GetColumn(3);

        // Clip space keeps -w <= x <= w and -w <= y <= w, and the depth runs from zero to w rather than -w.
        Frustum Result;
        Result.mPlanes[ZyEnum::Cast(Side::Left)]   = Vector4::Normalize3(R3 + R0);
        Result.mPlanes[ZyEnum::Cast(Side::Right)]  = Vector4::Normalize3(R3 - R0);
        Result.mPlanes[ZyEnum::Cast(Side::Bottom)] = Vector4::Normalize3(R3 + R1);
        Result.mPlanes[ZyEnum::Cast(Side::Top)]    = Vector4::Normalize3(R3 - R1);
        Result.mPlanes[ZyEnum::Cast(Side::Near)]   = Vector4::Normalize3(R2);
        Result.mPlanes[ZyEnum::Cast(Side::Far)]    = Vector4::Normalize3(R3 - R2);
        return Result;
    }
}