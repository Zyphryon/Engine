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

inline namespace ZyMath
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<SInt I, SInt J>
    static Vector4 Minors(Vector4 Col1, Vector4 Col2, Vector4 Col3)
    {
        const Vector4 Swap0A = Vector4::Shuffle<J, J, J, J>(Col3,   Col2);
        const Vector4 Swap0B = Vector4::Shuffle<I, I, I, I>(Col3,   Col2);

        const Vector4 Swap00 = Vector4::Shuffle<I, I, I, I>(Col2,   Col1);
        const Vector4 Swap01 = Vector4::Shuffle<0, 0, 0, 2>(Swap0A, Swap0A);
        const Vector4 Swap02 = Vector4::Shuffle<0, 0, 0, 2>(Swap0B, Swap0B);
        const Vector4 Swap03 = Vector4::Shuffle<J, J, J, J>(Col2,   Col1);

        return Swap00 * Swap01 - Swap02 * Swap03;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<SInt K>
    static Vector4 Spread(Vector4 Col0, Vector4 Col1)
    {
        const Vector4 Temp = Vector4::Shuffle<K, K, K, K>(Col1, Col0);
        return Vector4::Shuffle<0, 2, 2, 2>(Temp, Temp);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::Inverse(ConstRef<Matrix4x4> Matrix)
    {
        Vector4 Col0 = Matrix.mColumns[0];
        Vector4 Col1 = Matrix.mColumns[1];
        Vector4 Col2 = Matrix.mColumns[2];
        Vector4 Col3 = Matrix.mColumns[3];

        const Vector4 Fac0 = Minors<2, 3>(Col1, Col2, Col3);
        const Vector4 Fac1 = Minors<1, 3>(Col1, Col2, Col3);
        const Vector4 Fac2 = Minors<1, 2>(Col1, Col2, Col3);
        const Vector4 Fac3 = Minors<0, 3>(Col1, Col2, Col3);
        const Vector4 Fac4 = Minors<0, 2>(Col1, Col2, Col3);
        const Vector4 Fac5 = Minors<0, 1>(Col1, Col2, Col3);

        const Vector4 SignA(-1.0f, 1.0f, -1.0f, 1.0f);
        const Vector4 SignB( 1.0f,-1.0f,  1.0f,-1.0f);

        const Vector4 Vec0 = Spread<0>(Col0, Col1);
        const Vector4 Vec1 = Spread<1>(Col0, Col1);
        const Vector4 Vec2 = Spread<2>(Col0, Col1);
        const Vector4 Vec3 = Spread<3>(Col0, Col1);

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