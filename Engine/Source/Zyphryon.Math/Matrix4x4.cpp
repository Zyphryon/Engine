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
        ZY_ASSERT(false, "Not implemented");
        return 0.0f;
    }

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

        ZY_ASSERT(Abs(Determinant) >= 1e-8f, "Cannot invert a singular matrix (determinant is zero)");
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
        ZY_ASSERT(!::IsAlmostZero(Len0) && !::IsAlmostZero(Len1) && !::IsAlmostZero(Len2), "Invalid affine matrix: zero-length basis vector");

        // Divide each basis column by its squared length to obtain the transposed inverse rotation/scale block.
        const Vector4 Col0 = Matrix.mColumns[0] * (1.0f / Len0);
        const Vector4 Col1 = Matrix.mColumns[1] * (1.0f / Len1);
        const Vector4 Col2 = Matrix.mColumns[2] * (1.0f / Len2);

        Vector4 Row0 = Vector4::SplatX(Col0);
        Row0         = Vector4::Select<0b0010>(Row0, Vector4::SplatX(Col1));
        Row0         = Vector4::Select<0b0100>(Row0, Vector4::SplatX(Col2));
        Row0         = Vector4::Select<0b1000>(Row0, Vector4(0.0f));

        Vector4 Row1 = Vector4::SplatY(Col0);
        Row1         = Vector4::Select<0b0010>(Row1, Vector4::SplatY(Col1));
        Row1         = Vector4::Select<0b0100>(Row1, Vector4::SplatY(Col2));
        Row1         = Vector4::Select<0b1000>(Row1, Vector4(0.0f));

        Vector4 Row2 = Vector4::SplatZ(Col0);
        Row2         = Vector4::Select<0b0010>(Row2, Vector4::SplatZ(Col1));
        Row2         = Vector4::Select<0b0100>(Row2, Vector4::SplatZ(Col2));
        Row2         = Vector4::Select<0b1000>(Row2, Vector4(0.0f));

        const Real32 TranslateX = -Vector4::Dot3(Col0, Matrix.mColumns[3]);
        const Real32 TranslateY = -Vector4::Dot3(Col1, Matrix.mColumns[3]);
        const Real32 TranslateZ = -Vector4::Dot3(Col2, Matrix.mColumns[3]);
        return Matrix4x4(Row0, Row1, Row2, Vector4(TranslateX, TranslateY, TranslateZ, 1.0f));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::Lerp(ConstRef<Matrix4x4> From, ConstRef<Matrix4x4> To, Real32 Alpha)
    {
        return Matrix4x4(
            From.mColumns[0] + (To.mColumns[0] - From.mColumns[0]) * Alpha,
            From.mColumns[1] + (To.mColumns[1] - From.mColumns[1]) * Alpha,
            From.mColumns[2] + (To.mColumns[2] - From.mColumns[2]) * Alpha,
            From.mColumns[3] + (To.mColumns[3] - From.mColumns[3]) * Alpha);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Matrix4x4::Decompose(ConstRef<Matrix4x4> Matrix, Ref<Vector3> Translation, Ref<Vector3> Scale, Ref<Quaternion> Rotation)
    {
        Translation = Matrix.GetTranslation();
        Scale       = Matrix.GetScale();

        // Normalize the basis vectors to isolate the rotation component.
        const Real32 InvSX = !::IsAlmostZero(Scale.GetX()) ? 1.0f / Scale.GetX() : 0.0f;
        const Real32 InvSY = !::IsAlmostZero(Scale.GetY()) ? 1.0f / Scale.GetY() : 0.0f;
        const Real32 InvSZ = !::IsAlmostZero(Scale.GetZ()) ? 1.0f / Scale.GetZ() : 0.0f;

        Matrix4x4 RotMatrix = Matrix;
        RotMatrix.mColumns[0] = RotMatrix.mColumns[0] * InvSX;
        RotMatrix.mColumns[1] = RotMatrix.mColumns[1] * InvSY;
        RotMatrix.mColumns[2] = RotMatrix.mColumns[2] * InvSZ;
        RotMatrix.mColumns[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

        Rotation = RotMatrix.GetRotation();
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
            0.0f,   0.0f,   ZFar / (ZNear - ZFar),            -1.0f,
            0.0f,   0.0f,  (ZFar * ZNear) / (ZNear - ZFar),    0.0f);
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

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Matrix4x4 Matrix4x4::CreateDirection(Vector3 Eye, Vector3 Direction, Vector3 Up)
    {
        ZY_ASSERT(Direction.IsNormalized(), "Direction must be normalized");
        ZY_ASSERT(Up.IsNormalized(), "Up must be normalized");
        ZY_ASSERT(!Up.IsParallel(Direction), "Up vector is parallel to Direction");

        const Vector3 vForward = -Direction;
        const Vector3 vRight   = Vector3::Normalize(Vector3::Cross(Up, vForward));
        const Vector3 vUp      = Vector3::Cross(vForward, vRight);

        return Matrix4x4(
            vRight.GetX(), vUp.GetX(), vForward.GetX(), 0.0f,
            vRight.GetY(), vUp.GetY(), vForward.GetY(), 0.0f,
            vRight.GetZ(), vUp.GetZ(), vForward.GetZ(), 0.0f,
            -Vector3::Dot(vRight, Eye), -Vector3::Dot(vUp, Eye), -Vector3::Dot(vForward, Eye), 1.0f);
    }
}