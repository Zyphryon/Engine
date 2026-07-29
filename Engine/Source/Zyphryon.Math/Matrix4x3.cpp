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

#include "Matrix4x3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x3 Matrix4x3::CreateDirection(Vector3 Eye, Vector3 Direction, Vector3 Up)
    {
        ZY_ASSERT(Direction.IsNormalized(), "Direction must be normalized");
        ZY_ASSERT(Up.IsNormalized(), "Up must be normalized");
        ZY_ASSERT(!Up.IsParallel(Direction), "Up vector is parallel to Direction");

        const Vector3 Forward = -Direction;
        const Vector3 Right   = Vector3::Normalize(Vector3::Cross(Up, Forward));
        const Vector3 Upward  = Vector3::Cross(Forward, Right);

        return Matrix4x3(
            Vector4(Right.GetX(),   Right.GetY(),   Right.GetZ(),   -Vector3::Dot(Right, Eye)),
            Vector4(Upward.GetX(),  Upward.GetY(),  Upward.GetZ(),  -Vector3::Dot(Upward, Eye)),
            Vector4(Forward.GetX(), Forward.GetY(), Forward.GetZ(), -Vector3::Dot(Forward, Eye)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x3 Matrix4x3::Inverse(ConstRef<Matrix4x3> Matrix)
    {
        const Vector3 AxisX = Matrix.GetBasisX();
        const Vector3 AxisY = Matrix.GetBasisY();
        const Vector3 AxisZ = Matrix.GetBasisZ();

        const Vector3 CofactorX = Vector3::Cross(AxisY, AxisZ);
        const Vector3 CofactorY = Vector3::Cross(AxisZ, AxisX);
        const Vector3 CofactorZ = Vector3::Cross(AxisX, AxisY);

        const Real32 Determinant = Vector3::Dot(AxisX, CofactorX);

        ZY_ASSERT(Abs(Determinant) > kTolerance<Real32> * AxisX.GetLength() * AxisY.GetLength() * AxisZ.GetLength(),
            "Cannot invert a matrix whose basis is singular");

        const Real32  Reciprocal = 1.0f / Determinant;
        const Vector3 RowX       = CofactorX * Reciprocal;
        const Vector3 RowY       = CofactorY * Reciprocal;
        const Vector3 RowZ       = CofactorZ * Reciprocal;

        const Vector3 Offset = Matrix.GetTranslation();

        return Matrix4x3(
            Vector4(RowX.GetX(), RowX.GetY(), RowX.GetZ(), -Vector3::Dot(RowX, Offset)),
            Vector4(RowY.GetX(), RowY.GetY(), RowY.GetZ(), -Vector3::Dot(RowY, Offset)),
            Vector4(RowZ.GetX(), RowZ.GetY(), RowZ.GetZ(), -Vector3::Dot(RowZ, Offset)));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x3 Matrix4x3::Lerp(ConstRef<Matrix4x3> Start, ConstRef<Matrix4x3> End, Real32 Percentage)
    {
        return Matrix4x3(
            Start.mColumns[0] + (End.mColumns[0] - Start.mColumns[0]) * Percentage,
            Start.mColumns[1] + (End.mColumns[1] - Start.mColumns[1]) * Percentage,
            Start.mColumns[2] + (End.mColumns[2] - Start.mColumns[2]) * Percentage);
    }
}