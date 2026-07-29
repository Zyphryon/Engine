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

#include "Matrix4x4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::Inverse(ConstRef<Matrix4x4> Matrix)
    {
        Vector4 Col0 = Matrix.mColumns[0];
        Vector4 Col1 = Matrix.mColumns[1];
        Vector4 Col2 = Matrix.mColumns[2];
        Vector4 Col3 = Matrix.mColumns[3];

        Vector4 Fac0, Fac1, Fac2, Fac3, Fac4, Fac5;

        {
            const Vector4 Swp0a = Vector4::Shuffle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Shuffle<2, 2, 2, 2>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<3, 3, 3, 3>(Col2, Col1);

            Fac0 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        {
            const Vector4 Swp0a = Vector4::Shuffle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<1, 1, 1, 1>(Col3, Col2);

            const Vector4 Swp00 = Vector4::Shuffle<1, 1, 1, 1>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<3, 3, 3, 3>(Col2, Col1);

            Fac1 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        {
            const Vector4 Swp0a = Vector4::Shuffle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<1, 1, 1, 1>(Col3, Col2);

            const Vector4 Swp00 = Vector4::Shuffle<1, 1, 1, 1>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<2, 2, 2, 2>(Col2, Col1);

            Fac2 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        {
            const Vector4 Swp0a = Vector4::Shuffle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<0, 0, 0, 0>(Col3, Col2);

            const Vector4 Swp00 = Vector4::Shuffle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<3, 3, 3, 3>(Col2, Col1);

            Fac3 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        {
            const Vector4 Swp0a = Vector4::Shuffle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<0, 0, 0, 0>(Col3, Col2);

            const Vector4 Swp00 = Vector4::Shuffle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<2, 2, 2, 2>(Col2, Col1);

            Fac4 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        {
            const Vector4 Swp0a = Vector4::Shuffle<1, 1, 1, 1>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Shuffle<0, 0, 0, 0>(Col3, Col2);

            const Vector4 Swp00 = Vector4::Shuffle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Shuffle<0, 0, 0, 2>(Swp0a, Swp0a);
            const Vector4 Swp02 = Vector4::Shuffle<0, 0, 0, 2>(Swp0b, Swp0b);
            const Vector4 Swp03 = Vector4::Shuffle<1, 1, 1, 1>(Col2, Col1);

            Fac5 = Swp00 * Swp01 - Swp02 * Swp03;
        }

        const Vector4 SignA(-1.0f, 1.0f, -1.0f, 1.0f);
        const Vector4 SignB( 1.0f,-1.0f,  1.0f,-1.0f);

        const Vector4 Temp0 = Vector4::Shuffle<0, 0, 0, 0>(Col1, Col0);
        const Vector4 Vec0  = Vector4::Shuffle<0, 2, 2, 2>(Temp0, Temp0);

        const Vector4 Temp1 = Vector4::Shuffle<1, 1, 1, 1>(Col1, Col0);
        const Vector4 Vec1  = Vector4::Shuffle<0, 2, 2, 2>(Temp1, Temp1);

        const Vector4 Temp2 = Vector4::Shuffle<2, 2, 2, 2>(Col1, Col0);
        const Vector4 Vec2  = Vector4::Shuffle<0, 2, 2, 2>(Temp2, Temp2);

        const Vector4 Temp3 = Vector4::Shuffle<3, 3, 3, 3>(Col1, Col0);
        const Vector4 Vec3  = Vector4::Shuffle<0, 2, 2, 2>(Temp3, Temp3);

        const Vector4 Inv0 = SignB * ((Vec1 * Fac0) - (Vec2 * Fac1) + (Vec3 * Fac2));
        const Vector4 Inv1 = SignA * ((Vec0 * Fac0) - (Vec2 * Fac3) + (Vec3 * Fac4));
        const Vector4 Inv2 = SignB * ((Vec0 * Fac1) - (Vec1 * Fac3) + (Vec3 * Fac5));
        const Vector4 Inv3 = SignA * ((Vec0 * Fac2) - (Vec1 * Fac4) + (Vec2 * Fac5));

        const Vector4 Row0 = Vector4::Shuffle<0, 0, 0, 0>(Inv0, Inv1);
        const Vector4 Row1 = Vector4::Shuffle<0, 0, 0, 0>(Inv2, Inv3);
        const Vector4 Row2 = Vector4::Shuffle<0, 2, 0, 2>(Row0, Row1);

        const Real32 Determinant = Vector4::Dot(Col0, Row2);

        ZY_ASSERT(Abs(Determinant) > kTolerance<Real32> * Col0.GetLength() * Col1.GetLength() * Col2.GetLength(),
            "Cannot invert a singular matrix (determinant is negligible beside its scale)");

        const Real32 InvDet = 1.0f / Determinant;
        return Matrix4x4(Inv0 * InvDet, Inv1 * InvDet, Inv2 * InvDet, Inv3 * InvDet);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::Lerp(ConstRef<Matrix4x4> Start, ConstRef<Matrix4x4> End, Real32 Percentage)
    {
        return Matrix4x4(
            Start.mColumns[0] + (End.mColumns[0] - Start.mColumns[0]) * Percentage,
            Start.mColumns[1] + (End.mColumns[1] - Start.mColumns[1]) * Percentage,
            Start.mColumns[2] + (End.mColumns[2] - Start.mColumns[2]) * Percentage,
            Start.mColumns[3] + (End.mColumns[3] - Start.mColumns[3]) * Percentage);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreatePerspective(Angle FovY, Real32 Aspect, Real32 ZNear, Real32 ZFar)
    {
        ZY_ASSERT(!::IsAlmostZero(ZFar - ZNear), "Invalid perspective matrix: near/far planes too close");

        const Real32 ScaleY = 1.0f / Angle::Tangent(FovY * 0.5f);
        const Real32 ScaleX = ScaleY / Aspect;

        return Matrix4x4(
            ScaleX, 0.0f,   0.0f,                              0.0f,
            0.0f,   ScaleY, 0.0f,                              0.0f,
            0.0f,   0.0f,   ZFar / (ZFar - ZNear),             1.0f,
            0.0f,   0.0f, -(ZFar * ZNear) / (ZFar - ZNear),    0.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreateOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar)
    {
        ZY_ASSERT(!::IsAlmostZero(Right - Left), "Invalid orthographic matrix: width is zero");
        ZY_ASSERT(!::IsAlmostZero(Top - Bottom), "Invalid orthographic matrix: height is zero");
        ZY_ASSERT(!::IsAlmostZero(ZFar - ZNear), "Invalid orthographic matrix: depth is zero");

        const Real32 InvWidth  = 1.0f / (Right - Left);
        const Real32 InvHeight = 1.0f / (Top - Bottom);
        const Real32 InvDepth  = 1.0f / (ZFar - ZNear);

        return Matrix4x4(
            2.0f * InvWidth,            0.0f,                        0.0f,              0.0f,
            0.0f,                       2.0f * InvHeight,            0.0f,              0.0f,
            0.0f,                       0.0f,                        InvDepth,          0.0f,
            -(Right + Left) * InvWidth, -(Top + Bottom) * InvHeight, -ZNear * InvDepth, 1.0f);
    }
}