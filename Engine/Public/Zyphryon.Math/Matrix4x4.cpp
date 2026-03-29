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

    Real32 Matrix4x4::GetDeterminant(ConstRef<Matrix4x4> Matrix)
    {
        // TODO: SIMD
        ZYPHRYON_ALIGN(16) Real32 A[4];
        ZYPHRYON_ALIGN(16) Real32 B[4];
        ZYPHRYON_ALIGN(16) Real32 C[4];
        ZYPHRYON_ALIGN(16) Real32 D[4];

        Matrix.mColumns[0].Store(A);
        Matrix.mColumns[1].Store(B);
        Matrix.mColumns[2].Store(C);
        Matrix.mColumns[3].Store(D);

        const Real32 Sub00 = C[2] * D[3] - C[3] * D[2];
        const Real32 Sub01 = C[1] * D[3] - C[3] * D[1];
        const Real32 Sub02 = C[1] * D[2] - C[2] * D[1];
        const Real32 Sub03 = C[0] * D[3] - C[3] * D[0];
        const Real32 Sub04 = C[0] * D[2] - C[2] * D[0];
        const Real32 Sub05 = C[0] * D[1] - C[1] * D[0];

        const Real32 Cof0 = +(B[1] * Sub00 - B[2] * Sub01 + B[3] * Sub02);
        const Real32 Cof1 = -(B[0] * Sub00 - B[2] * Sub03 + B[3] * Sub04);
        const Real32 Cof2 = +(B[0] * Sub01 - B[1] * Sub03 + B[3] * Sub05);
        const Real32 Cof3 = -(B[0] * Sub02 - B[1] * Sub04 + B[2] * Sub05);

        return A[0] * Cof0 + A[1] * Cof1 + A[2] * Cof2 + A[3] * Cof3;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::Inverse(ConstRef<Matrix4x4> Matrix)
    {
        Vector4 Col0 = Matrix.mColumns[0];
        Vector4 Col1 = Matrix.mColumns[1];
        Vector4 Col2 = Matrix.mColumns[2];
        Vector4 Col3 = Matrix.mColumns[3];

        // Calculate cofactors
        Vector4 Fac0, Fac1, Fac2, Fac3, Fac4, Fac5;

        // Fac0: SubFactor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3]
        //       SubFactor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3]
        //       SubFactor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3]
        {
            const Vector4 Swp0a = Vector4::Swizzle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<2, 2, 2, 2>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<3, 3, 3, 3>(Col2, Col1);

            Fac0 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        // Fac1: SubFactor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3]
        //       SubFactor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3]
        //       SubFactor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3]
        {
            const Vector4 Swp0a = Vector4::Swizzle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<1, 1, 1, 1>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<1, 1, 1, 1>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<3, 3, 3, 3>(Col2, Col1);

            Fac1 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        // Fac2: SubFactor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2]
        //       SubFactor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2]
        //       SubFactor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2]
        {
            const Vector4 Swp0a = Vector4::Swizzle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<1, 1, 1, 1>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<1, 1, 1, 1>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<2, 2, 2, 2>(Col2, Col1);

            Fac2 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        // Fac3: SubFactor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3]
        //       SubFactor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3]
        //       SubFactor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3]
        {
            const Vector4 Swp0a = Vector4::Swizzle<3, 3, 3, 3>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<0, 0, 0, 0>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<3, 3, 3, 3>(Col2, Col1);

            Fac3 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        // Fac4: SubFactor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2]
        //       SubFactor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2]
        //       SubFactor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2]
        {
            const Vector4 Swp0a = Vector4::Swizzle<2, 2, 2, 2>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<0, 0, 0, 0>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<2, 2, 2, 2>(Col2, Col1);

            Fac4 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        // Fac5: SubFactor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1]
        //       SubFactor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1]
        //       SubFactor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1]
        {
            const Vector4 Swp0a = Vector4::Swizzle<1, 1, 1, 1>(Col3, Col2);
            const Vector4 Swp0b = Vector4::Swizzle<0, 0, 0, 0>(Col3, Col2);
            const Vector4 Swp00 = Vector4::Swizzle<0, 0, 0, 0>(Col2, Col1);
            const Vector4 Swp01 = Vector4::Swizzle<2, 0, 0, 0>(Swp0a);
            const Vector4 Swp02 = Vector4::Swizzle<2, 0, 0, 0>(Swp0b);
            const Vector4 Swp03 = Vector4::Swizzle<1, 1, 1, 1>(Col2, Col1);

            Fac5 = (Swp00 * Swp01) - (Swp02 * Swp03);
        }

        const Vector4 SignA = Vector4(1.0f, -1.0f, 1.0f, -1.0f);
        const Vector4 SignB = Vector4(-1.0f, 1.0f, -1.0f, 1.0f);

        const Vector4 Vec0 = Vector4::Swizzle<2, 2, 2, 0>(Vector4::Swizzle<0, 0, 0, 0>(Col1, Col0));
        const Vector4 Vec1 = Vector4::Swizzle<2, 2, 2, 0>(Vector4::Swizzle<1, 1, 1, 1>(Col1, Col0));
        const Vector4 Vec2 = Vector4::Swizzle<2, 2, 2, 0>(Vector4::Swizzle<2, 2, 2, 2>(Col1, Col0));
        const Vector4 Vec3 = Vector4::Swizzle<2, 2, 2, 0>(Vector4::Swizzle<3, 3, 3, 3>(Col1, Col0));

        const Vector4 Inv0 = SignB * (Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
        const Vector4 Inv1 = SignA * (Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
        const Vector4 Inv2 = SignB * (Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
        const Vector4 Inv3 = SignA * (Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

        const Vector4 Row0 = Vector4::Swizzle<0, 0, 0, 0>(Inv0, Inv1);
        const Vector4 Row1 = Vector4::Swizzle<0, 0, 0, 0>(Inv2, Inv3);
        const Vector4 Row2 = Vector4::Swizzle<2, 0, 2, 0>(Row0, Row1);

        const Real32 Determinant = Vector4::Dot(Col0, Row2);

        const Real32 InvDet = 1.0f / Determinant;
        return Matrix4x4(Inv0 * InvDet, Inv1 * InvDet, Inv2 * InvDet, Inv3 * InvDet);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::InverseAffine(ConstRef<Matrix4x4> Matrix)
    {
        const Real32 Len0 = Vector4::Dot3(Matrix.mColumns[0], Matrix.mColumns[0]);
        const Real32 Len1 = Vector4::Dot3(Matrix.mColumns[1], Matrix.mColumns[1]);
        const Real32 Len2 = Vector4::Dot3(Matrix.mColumns[2], Matrix.mColumns[2]);
        LOG_ASSERT(!Base::IsAlmostZero(Len0) &&
                   !Base::IsAlmostZero(Len1) &&
                   !Base::IsAlmostZero(Len2), "Invalid affine matrix: zero-length basis vector");

        const Vector4 Col0 = Matrix.mColumns[0] * (Len0 < kEpsilon<Real32> ? 1.0f : 1.0f / Len0);
        const Vector4 Col1 = Matrix.mColumns[1] * (Len1 < kEpsilon<Real32> ? 1.0f : 1.0f / Len1);
        const Vector4 Col2 = Matrix.mColumns[2] * (Len2 < kEpsilon<Real32> ? 1.0f : 1.0f / Len2);

        Vector4 Row0 = Vector4::SplatX(Col0);
        Row0         = Vector4::Blend<0b0010>(Row0, Vector4::SplatX(Col1));
        Row0         = Vector4::Blend<0b0100>(Row0, Vector4::SplatX(Col2));
        Row0         = Vector4::Blend<0b1000>(Row0, Vector4(0.0f));

        Vector4 Row1 = Vector4::SplatY(Col0);
        Row1         = Vector4::Blend<0b0010>(Row1, Vector4::SplatY(Col1));
        Row1         = Vector4::Blend<0b0100>(Row1, Vector4::SplatY(Col2));
        Row1         = Vector4::Blend<0b1000>(Row1, Vector4(0.0f));

        Vector4 Row2 = Vector4::SplatZ(Col0);
        Row2         = Vector4::Blend<0b0010>(Row2, Vector4::SplatZ(Col1));
        Row2         = Vector4::Blend<0b0100>(Row2, Vector4::SplatZ(Col2));
        Row2         = Vector4::Blend<0b1000>(Row2, Vector4(0.0f));

        const Real32 TranslateX = -Vector4::Dot3(Col0, Matrix.mColumns[3]);
        const Real32 TranslateY = -Vector4::Dot3(Col1, Matrix.mColumns[3]);
        const Real32 TranslateZ = -Vector4::Dot3(Col2, Matrix.mColumns[3]);
        return Matrix4x4(Row0, Row1, Row2, Vector4(TranslateX, TranslateY, TranslateZ, 1.0f));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreatePerspective(Angle FovY, Real32 Aspect, Real32 ZNear, Real32 ZFar)
    {
        LOG_ASSERT(!Base::IsAlmostZero(ZFar - ZNear), "Invalid perspective matrix: near/far planes too close");
        LOG_ASSERT(ZFar > ZNear, "Invalid perspective matrix: ZFar must be greater than ZNear");

        const Real32 ScaleY = 1.0f / Angle::Tangent(FovY * 0.5f);
        const Real32 ScaleX = ScaleY / Aspect;

        return Matrix4x4(
            ScaleX, 0.0f,   0.0f,                              0.0f,
            0.0f,   ScaleY, 0.0f,                              0.0f,
            0.0f,   0.0f,   ZFar / (ZNear - ZFar),            -1.0f,
            0.0f,   0.0f,  (ZFar * ZNear) / (ZNear - ZFar),    0.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreateOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar)
    {
        LOG_ASSERT(!Base::IsAlmostZero(Right - Left), "Invalid orthographic matrix: width is zero");
        LOG_ASSERT(!Base::IsAlmostZero(Top - Bottom), "Invalid orthographic matrix: height is zero");
        LOG_ASSERT(!Base::IsAlmostZero(ZFar - ZNear), "Invalid orthographic matrix: depth is zero");
        LOG_ASSERT(ZFar > ZNear, "Invalid orthographic matrix: ZFar must be greater than ZNear");

        const Real32 InvWidth  = 1.0f / (Right - Left);
        const Real32 InvHeight = 1.0f / (Top - Bottom);
        const Real32 InvDepth  = 1.0f / (ZFar - ZNear);

        return Matrix4x4(
            2.0f * InvWidth,            0.0f,                        0.0f,              0.0f,
            0.0f,                       2.0f * InvHeight,            0.0f,              0.0f,
            0.0f,                       0.0f,                        InvDepth,          0.0f,
            -(Right + Left) * InvWidth, -(Top + Bottom) * InvHeight, -ZNear * InvDepth, 1.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreateLook(Vector3 Eye, Vector3 Focus, Vector3 Up)
    {
        LOG_ASSERT(Up.IsNormalized(), "Up must be normalized");
        LOG_ASSERT(!Vector3::IsParallel(Up, Eye - Focus), "Up vector is parallel to forward direction");

        const Vector3 vForward = Vector3::Normalize(Eye - Focus);
        const Vector3 vRight   = Vector3::Normalize(Vector3::Cross(Up, vForward));
        const Vector3 vUp      = Vector3::Cross(vForward, vRight);

        return Matrix4x4(
            vRight.GetX(), vUp.GetX(), vForward.GetX(), 0.0f,
            vRight.GetY(), vUp.GetY(), vForward.GetY(), 0.0f,
            vRight.GetZ(), vUp.GetZ(), vForward.GetZ(), 0.0f,
            -Vector3::Dot(vRight, Eye), -Vector3::Dot(vUp, Eye), -Vector3::Dot(vForward, Eye), 1.0f);
    }
}