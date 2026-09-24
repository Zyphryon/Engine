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

    Vector4 Frustum::GetPlane(Side Plane) const
    {
        const UInt32 Index = ZyEnum::Cast(Plane);
        const UInt32 Lane  = Index % 4;

        ZY_ALIGN(16) Real32 Coefficients[4][4];

        for (UInt32 Row = 0; Row < 4; ++Row)
        {
            mQuartets[Index / 4][Row].Store(Coefficients[Row]);
        }
        return Vector4(Coefficients[0][Lane], Coefficients[1][Lane], Coefficients[2][Lane], Coefficients[3][Lane]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(Vector3 Point) const
    {
        return Test(Sphere(Point, 0.0f));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(ConstRef<Sphere> Volume) const
    {
        const Vector3 Center = Volume.GetCenter();
        const Vector4 X(Center.GetX());
        const Vector4 Y(Center.GetY());
        const Vector4 Z(Center.GetZ());
        const Vector4 Reach(-Volume.GetRadius());

        Vector4 Outside;

        for (ConstRef<Quartet> Group : mQuartets)
        {
            Outside |= Measure(Group, X, Y, Z) < Reach;
        }
        return !Outside.IsAnyTrue();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Frustum::Test(ConstRef<Box> Volume) const
    {
        const Vector3 Minimum = Volume.GetMinimum();
        const Vector3 Maximum = Volume.GetMaximum();

        const Vector4 MinimumX(Minimum.GetX());
        const Vector4 MinimumY(Minimum.GetY());
        const Vector4 MinimumZ(Minimum.GetZ());
        const Vector4 MaximumX(Maximum.GetX());
        const Vector4 MaximumY(Maximum.GetY());
        const Vector4 MaximumZ(Maximum.GetZ());
        const Vector4 Zero = Vector4::Zero();

        Vector4 Outside;

        for (ConstRef<Quartet> Group : mQuartets)
        {
            const Vector4 X = Vector4::Select(MaximumX, MinimumX, Group[0] < Zero);
            const Vector4 Y = Vector4::Select(MaximumY, MinimumY, Group[1] < Zero);
            const Vector4 Z = Vector4::Select(MaximumZ, MinimumZ, Group[2] < Zero);

            Outside |= Measure(Group, X, Y, Z) < Zero;
        }
        return !Outside.IsAnyTrue();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Frustum Frustum::FromMatrix(ConstRef<Matrix4x4> ViewProjection)
    {
        Frustum Result;

        // Left, right, bottom and top add or take away x, then y, which a sign per lane turns into one sum.
        const Vector4 Signs(1.0f, -1.0f, 1.0f, -1.0f);

        // Every plane is a sum of the matrix's rows, so its coefficient on one axis comes from that axis's column.
        for (UInt32 Axis = 0; Axis < 4; ++Axis)
        {
            const Vector4 Column = ViewProjection.GetColumn(Axis);

            // Clip space keeps -w <= x <= w and -w <= y <= w, and the depth runs from zero to w rather than -w.
            Result.mQuartets[0][Axis] = Vector4::SplatW(Column) + Vector4::Swizzle<0, 0, 1, 1>(Column) * Signs;

            // Near and far fill half of the second group, and the rest holds a plane every point lies a unit inside of.
            const Real32 Open = (Axis == 3 ? 1.0f : 0.0f);

            ZY_ALIGN(16) Real32 Lanes[4];
            Column.Store(Lanes);

            Result.mQuartets[1][Axis] = Vector4(Lanes[2], Lanes[3] - Lanes[2], Open, Open);
        }

        Normalize(Result.mQuartets[0]);
        Normalize(Result.mQuartets[1]);
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Frustum::Normalize(Ref<Quartet> Group)
    {
        const Vector4 Length  = Vector4::Sqrt(Group[0] * Group[0] + Group[1] * Group[1] + Group[2] * Group[2]);
        const Vector4 Divisor = Vector4::Select(Length, Vector4(1.0f), Length <= Vector4(kEpsilon<Real32>));

        for (Ref<Vector4> Coefficient : Group)
        {
            Coefficient = Coefficient / Divisor;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector4 Frustum::Measure(ConstRef<Quartet> Group, Vector4 X, Vector4 Y, Vector4 Z)
    {
        return (Group[0] * X + Group[1] * Y) + (Group[2] * Z + Group[3]);
    }
}