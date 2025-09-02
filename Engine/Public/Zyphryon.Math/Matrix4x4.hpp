// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Vector2.hpp"
#include "Vector3.hpp"
#include "Vector4.hpp"
#include "Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a 4x4 Matrix with column-major storage.
    class ZYPHRYON_ALIGN(16) Matrix4x4 final
    {
    public:

        /// \brief Constructs an identity matrix.
        ZYPHRYON_INLINE Matrix4x4()
            : Matrix4x4 { 1.0f }
        {
        }

        /// \brief Constructs a diagonal matrix with specified value.
        ///
        /// \param Diagonal Value to set for all diagonal elements.
        ZYPHRYON_INLINE Matrix4x4(Real32 Diagonal)
            : Matrix4x4 { Diagonal, 0.0f, 0.0f, 0.0f,
                          0.0f, Diagonal, 0.0f, 0.0f,
                          0.0f, 0.0f, Diagonal, 0.0f,
                          0.0f, 0.0f, 0.0f, Diagonal }
        {
        }

        /// \brief Constructs a matrix from vectors.
        ///
        /// \param Column0 The first column vector of the matrix.
        /// \param Column1 The second column vector of the matrix.
        /// \param Column2 The third column vector of the matrix.
        /// \param Column3 The fourth column vector of the matrix.
        ZYPHRYON_INLINE Matrix4x4(Vector4 Column0, Vector4 Column1, Vector4 Column2, Vector4 Column3)
            : mColumns { Column0, Column1, Column2, Column3 }
        {
        }

        /// \brief Constructs matrix from 16 scalar values (column-major order).
        ///
        /// \param S00 ,S10,S20,S30 First column elements
        /// \param S01 ,S11,S21,S31 Second column elements
        /// \param S02 ,S12,S22,S32 Third column elements
        /// \param S03 ,S13,S23,S33 Fourth column elements
        ZYPHRYON_INLINE Matrix4x4(
            Real32 S00, Real32 S10, Real32 S20, Real32 S30,
            Real32 S01, Real32 S11, Real32 S21, Real32 S31,
            Real32 S02, Real32 S12, Real32 S22, Real32 S32,
            Real32 S03, Real32 S13, Real32 S23, Real32 S33)
            : mColumns { Vector4(S00, S10, S20, S30),
                         Vector4(S01, S11, S21, S31),
                         Vector4(S02, S12, S22, S32),
                         Vector4(S03, S13, S23, S33) }
        {
        }

        /// \brief Copy constructor.
        ///
        /// \param Other The matrix to copy.
        ZYPHRYON_INLINE Matrix4x4(ConstRef<Matrix4x4> Other) = default;

        /// \brief Checks if matrix is identity.
        ///
        /// \return `true` if matrix is identity, `false` otherwise.
        ZYPHRYON_INLINE Bool IsIdentity() const
        {
            return mColumns[0].IsAlmostEqual(1.0f, 0.0f, 0.0f, 0.0f)
                && mColumns[1].IsAlmostEqual(0.0f, 1.0f, 0.0f, 0.0f)
                && mColumns[2].IsAlmostEqual(0.0f, 0.0f, 1.0f, 0.0f)
                && mColumns[3].IsAlmostEqual(0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Checks if matrix is orthogonal.
        ///
        /// \return `true` if matrix is orthogonal, `false` otherwise.
        ZYPHRYON_INLINE Bool IsOrthogonal() const
        {
            LOG_ASSERT(IsFinite(), "Matrix contains invalid (NaN/Inf) values before orthogonality check");

            return (Transpose(*this) * (* this)).IsIdentity();
        }

        /// \brief Checks if all components of the matrix are finite (not NaN or Inf).
        ///
        /// \return `true` if all values are finite, `false` otherwise.
        ZYPHRYON_INLINE Bool IsFinite() const
        {
            return mColumns[0].IsFinite() && mColumns[1].IsFinite() && mColumns[2].IsFinite() && mColumns[3].IsFinite();
        }

        /// \brief Retrieves a column vector from the matrix.
        ///
        /// \param Column Zero-based index of the column to access.
        /// \return Reference to the column vector at the specified index.
        ZYPHRYON_INLINE Ref<Vector4> GetColumn(UInt32 Column)
        {
            LOG_ASSERT(Column < 4, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Retrieves a column vector from the matrix.
        ///
        /// \param Column Zero-based index of the column to access.
        /// \return Reference to the column vector at the specified index.
        ZYPHRYON_INLINE ConstRef<Vector4> GetColumn(UInt32 Column) const
        {
            return mColumns[Column];
        }

        /// \brief Gets the trace of the matrix (sum of diagonal elements).
        ///
        /// \return The trace value.
        ZYPHRYON_INLINE Real32 GetTrace() const
        {
            return mColumns[0].GetX() + mColumns[1].GetY() + mColumns[2].GetZ() + mColumns[3].GetW();
        }

        /// \brief Gets the determinant of the matrix.
        ///
        /// \return The determinant value.
        ZYPHRYON_INLINE Real32 GetDeterminant() const
        {
            ZYPHRYON_ALIGN(16) Real32 A[4];
            ZYPHRYON_ALIGN(16) Real32 B[4];
            ZYPHRYON_ALIGN(16) Real32 C[4];
            ZYPHRYON_ALIGN(16) Real32 D[4];

            mColumns[0].Store(A);
            mColumns[1].Store(B);
            mColumns[2].Store(C);
            mColumns[3].Store(D);

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

        /// \brief Gets the right direction vector from the matrix.
        ///
        /// \return The right vector (first column).
        ZYPHRYON_INLINE Vector3 GetRight() const
        {
            return mColumns[0].GetXYZ();
        }

        /// \brief Gets the left direction vector from the matrix.
        ///
        /// \return The left vector (negative right).
        ZYPHRYON_INLINE Vector3 GetLeft() const
        {
            return -GetRight();
        }

        /// \brief Gets the up direction vector from the matrix.
        ///
        /// \return The up vector (second column).
        ZYPHRYON_INLINE Vector3 GetUp() const
        {
            return mColumns[1].GetXYZ();
        }

        /// \brief Gets the down direction vector from the matrix.
        ///
        /// \return The down vector (negative up).
        ZYPHRYON_INLINE Vector3 GetDown() const
        {
            return -GetUp();
        }

        /// \brief Gets the forward direction vector from the matrix.
        ///
        /// \return The forward vector (third column).
        ZYPHRYON_INLINE Vector3 GetForward() const
        {
            return mColumns[2].GetXYZ();
        }

        /// \brief Gets the backward direction vector from the matrix.
        ///
        /// \return The backward vector (negative forward).
        ZYPHRYON_INLINE Vector3 GetBackward() const
        {
            return -GetForward();
        }

        /// \brief Gets the scale vector from the matrix.
        ///
        /// \return Vector containing the scale of each axis.
        ZYPHRYON_INLINE Vector3 GetScale() const
        {
            return Vector3(mColumns[0].GetLength(), mColumns[1].GetLength(), mColumns[2].GetLength());
        }

        /// \brief Gets the translation vector from the matrix.
        ///
        /// \return The translation components (fourth column).
        ZYPHRYON_INLINE Vector3 GetTranslation() const
        {
            return mColumns[3].GetXYZ();
        }

        /// \brief Gets the rotation component of the transformation matrix as a quaternion.
        ///
        /// \return The quaternion representing the rotation.
        ZYPHRYON_INLINE Quaternion GetRotation() const
        {
            return Quaternion::FromDirection(mColumns[2].GetXYZ(), mColumns[1].GetXYZ());
        }

        /// \brief Adds another matrix to this matrix.
        ///
        /// \param Other The matrix to add.
        /// \return A new matrix that is the sum of this matrix and the input matrix.
        ZYPHRYON_INLINE Matrix4x4 operator+(ConstRef<Matrix4x4> Other) const
        {
            return Matrix4x4(
                mColumns[0] + Other.mColumns[0],
                mColumns[1] + Other.mColumns[1],
                mColumns[2] + Other.mColumns[2],
                mColumns[3] + Other.mColumns[3]);
        }

        /// \brief Subtracts another matrix from this matrix.
        ///
        /// \param Other The matrix to subtract.
        /// \return A new matrix that is the difference of the two matrices.
        ZYPHRYON_INLINE Matrix4x4 operator-(ConstRef<Matrix4x4> Other) const
        {
            return Matrix4x4(
                mColumns[0] - Other.mColumns[0],
                mColumns[1] - Other.mColumns[1],
                mColumns[2] - Other.mColumns[2],
                mColumns[3] - Other.mColumns[3]);
        }

        /// \brief Multiplies this matrix by another matrix.
        ///
        /// \param Other The matrix to multiply by.
        /// \return A new matrix that is the product of the two matrices.
        ZYPHRYON_INLINE Matrix4x4 operator*(ConstRef<Matrix4x4> Other) const
        {
            const Vector4 Col0 = mColumns[0] * Vector4::SplatX(Other.mColumns[0]) +
                                 mColumns[1] * Vector4::SplatY(Other.mColumns[0]) +
                                 mColumns[2] * Vector4::SplatZ(Other.mColumns[0]) +
                                 mColumns[3] * Vector4::SplatW(Other.mColumns[0]);

            const Vector4 Col1 = mColumns[0] * Vector4::SplatX(Other.mColumns[1]) +
                                 mColumns[1] * Vector4::SplatY(Other.mColumns[1]) +
                                 mColumns[2] * Vector4::SplatZ(Other.mColumns[1]) +
                                 mColumns[3] * Vector4::SplatW(Other.mColumns[1]);

            const Vector4 Col2 = mColumns[0] * Vector4::SplatX(Other.mColumns[2]) +
                                 mColumns[1] * Vector4::SplatY(Other.mColumns[2]) +
                                 mColumns[2] * Vector4::SplatZ(Other.mColumns[2]) +
                                 mColumns[3] * Vector4::SplatW(Other.mColumns[2]);

            const Vector4 Col3 = mColumns[0] * Vector4::SplatX(Other.mColumns[3]) +
                                 mColumns[1] * Vector4::SplatY(Other.mColumns[3]) +
                                 mColumns[2] * Vector4::SplatZ(Other.mColumns[3]) +
                                 mColumns[3] * Vector4::SplatW(Other.mColumns[3]);

            return Matrix4x4(Col0, Col1, Col2, Col3);
        }

        /// \brief Adds another matrix to the current matrix.
        ///
        /// \param Other The matrix to add.
        /// \return A reference to the updated matrix.
        ZYPHRYON_INLINE Ref<Matrix4x4> operator+=(ConstRef<Matrix4x4> Other)
        {
            mColumns[0] += Other.mColumns[0];
            mColumns[1] += Other.mColumns[1];
            mColumns[2] += Other.mColumns[2];
            mColumns[3] += Other.mColumns[3];
            return (* this);
        }

        /// \brief Subtracts another matrix to the current matrix.
        ///
        /// \param Other The matrix to subtract.
        /// \return A reference to the updated matrix.
        ZYPHRYON_INLINE Ref<Matrix4x4> operator-=(ConstRef<Matrix4x4> Other)
        {
            mColumns[0] -= Other.mColumns[0];
            mColumns[1] -= Other.mColumns[1];
            mColumns[2] -= Other.mColumns[2];
            mColumns[3] -= Other.mColumns[3];
            return (* this);
        }

        /// \brief Multiplies another matrix to the current matrix.
        ///
        /// \param Other The matrix to multiply by.
        /// \return A reference to the updated matrix.
        ZYPHRYON_INLINE Ref<Matrix4x4> operator*=(ConstRef<Matrix4x4> Other)
        {
            mColumns[0] = Other.mColumns[0] * Vector4::SplatX(mColumns[0]) +
                          Other.mColumns[1] * Vector4::SplatY(mColumns[0]) +
                          Other.mColumns[2] * Vector4::SplatZ(mColumns[0]) +
                          Other.mColumns[3] * Vector4::SplatW(mColumns[0]);

            mColumns[1] = Other.mColumns[0] * Vector4::SplatX(mColumns[1]) +
                          Other.mColumns[1] * Vector4::SplatY(mColumns[1]) +
                          Other.mColumns[2] * Vector4::SplatZ(mColumns[1]) +
                          Other.mColumns[3] * Vector4::SplatW(mColumns[1]);

            mColumns[2] = Other.mColumns[0] * Vector4::SplatX(mColumns[2]) +
                          Other.mColumns[1] * Vector4::SplatY(mColumns[2]) +
                          Other.mColumns[2] * Vector4::SplatZ(mColumns[2]) +
                          Other.mColumns[3] * Vector4::SplatW(mColumns[2]);

            mColumns[3] = Other.mColumns[0] * Vector4::SplatX(mColumns[3]) +
                          Other.mColumns[1] * Vector4::SplatY(mColumns[3]) +
                          Other.mColumns[2] * Vector4::SplatZ(mColumns[3]) +
                          Other.mColumns[3] * Vector4::SplatW(mColumns[3]);

            return (* this);
        }

        /// \brief Checks if this matrix is equal to another matrix.
        ///
        /// \param Matrix The matrix to compare to.
        /// \return `true` if the matrices are equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator==(ConstRef<Matrix4x4> Matrix) const
        {
            return mColumns[0] == Matrix.mColumns[0]
                && mColumns[1] == Matrix.mColumns[1]
                && mColumns[2] == Matrix.mColumns[2]
                && mColumns[3] == Matrix.mColumns[3];
        }

        /// \brief Checks if this matrix is not equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are not equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Matrix4x4> Matrix) const = default;

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mColumns[0]);
            Archive.SerializeObject(mColumns[1]);
            Archive.SerializeObject(mColumns[2]);
            Archive.SerializeObject(mColumns[3]);
        }

    public:

        /// \brief Projects a 2D vector using a 4x4 transformation matrix.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 2D vector to be projected.
        /// \return A 2D vector resulting from the projection.
        ZYPHRYON_INLINE static Vector2 Project(ConstRef<Matrix4x4> Matrix, ConstRef<Vector2> Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4(Vector.GetX()) +
                Matrix.mColumns[1] * Vector4(Vector.GetY()) +
                Matrix.mColumns[3];

            const Real32 W = Result.GetW();
            LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

            const Real32 InvW = 1.0f / W;
            return Vector2(Result.GetX() * InvW, Result.GetY() * InvW);
        }

        /// \brief Projects a 3D vector using a 4x4 transformation matrix.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 3D vector to be projected.
        /// \return A 3D vector resulting from the projection.
        ZYPHRYON_INLINE static Vector3 Project(ConstRef<Matrix4x4> Matrix, ConstRef<Vector3> Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4(Vector.GetX()) +
                Matrix.mColumns[1] * Vector4(Vector.GetY()) +
                Matrix.mColumns[2] * Vector4(Vector.GetZ()) +
                Matrix.mColumns[3];

            const Real32 W = Result.GetW();
            LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

            const Real32 InvW = 1.0f / W;
            return Vector3(Result.GetX() * InvW, Result.GetY() * InvW, Result.GetZ() * InvW);
        }

        /// \brief Projects a 4D vector using a 4x4 transformation matrix.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 4D vector to be projected.
        /// \return A 4D vector resulting from the projection.
        ZYPHRYON_INLINE static Vector4 Project(ConstRef<Matrix4x4> Matrix, ConstRef<Vector4> Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4::SplatX(Vector) +
                Matrix.mColumns[1] * Vector4::SplatY(Vector) +
                Matrix.mColumns[2] * Vector4::SplatZ(Vector) +
                Matrix.mColumns[3] * Vector4::SplatW(Vector);

            const Real32 W = Result.GetW();
            LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

            return (W != 0.0f ? Result * (1.0f / W) : Result);
        }

        /// \brief Computes the transpose of a 4x4 matrix.
        ///
        /// \param Matrix The matrix to transpose.
        /// \return The transposed 4x4 matrix.
        ZYPHRYON_INLINE static Matrix4x4 Transpose(ConstRef<Matrix4x4> Matrix)
        {
            const __m128 Temp0 = _mm_unpacklo_ps(Matrix.mColumns[0].mRegister, Matrix.mColumns[1].mRegister);
            const __m128 Temp1 = _mm_unpackhi_ps(Matrix.mColumns[0].mRegister, Matrix.mColumns[1].mRegister);
            const __m128 Temp2 = _mm_unpacklo_ps(Matrix.mColumns[2].mRegister, Matrix.mColumns[3].mRegister);
            const __m128 Temp3 = _mm_unpackhi_ps(Matrix.mColumns[2].mRegister, Matrix.mColumns[3].mRegister);

            return Matrix4x4(
                Vector4(_mm_movelh_ps(Temp0, Temp2)),
                Vector4(_mm_movehl_ps(Temp2, Temp0)),
                Vector4(_mm_movelh_ps(Temp1, Temp3)),
                Vector4(_mm_movehl_ps(Temp3, Temp1)));
        }

        /// \brief Computes the inverse of a 4×4 matrix.
        ///
        /// \tparam Affine When `true`, uses the affine fast path, otherwise uses the general 4×4 inverse.
        ///
        /// \param Matrix The matrix to invert.
        template<Bool Affine = false>
        ZYPHRYON_INLINE static Matrix4x4 Inverse(ConstRef<Matrix4x4> Matrix)
        {
            if constexpr (Affine)
            {
                const Real32 Len0 = Vector4::Dot3(Matrix.mColumns[0], Matrix.mColumns[0]);
                const Real32 Len1 = Vector4::Dot3(Matrix.mColumns[1], Matrix.mColumns[1]);
                const Real32 Len2 = Vector4::Dot3(Matrix.mColumns[2], Matrix.mColumns[2]);
                LOG_ASSERT(!Base::IsAlmostZero(Len0)
                        && !Base::IsAlmostZero(Len1)
                        && !Base::IsAlmostZero(Len2), "Invalid affine matrix: zero-length basis vector");

                const Vector4 Col0 = Matrix.mColumns[0] * (Len0 < kEpsilon<Real32> ? 1.0f : 1.0f / Len0);
                const Vector4 Col1 = Matrix.mColumns[1] * (Len1 < kEpsilon<Real32> ? 1.0f : 1.0f / Len1);
                const Vector4 Col2 = Matrix.mColumns[2] * (Len2 < kEpsilon<Real32> ? 1.0f : 1.0f / Len2);

                Vector4 Row0 = Vector4::SplatX(Col0);
                Row0 = Vector4::Blend<0b0010>(Row0, Vector4::SplatX(Col1));
                Row0 = Vector4::Blend<0b0100>(Row0, Vector4::SplatX(Col2));
                Row0 = Vector4::Blend<0b1000>(Row0, Vector4(0.0f));

                Vector4 Row1 = Vector4::SplatY(Col0);
                Row1 = Vector4::Blend<0b0010>(Row1, Vector4::SplatY(Col1));
                Row1 = Vector4::Blend<0b0100>(Row1, Vector4::SplatY(Col2));
                Row1 = Vector4::Blend<0b1000>(Row1, Vector4(0.0f));

                Vector4 Row2 = Vector4::SplatZ(Col0);
                Row2 = Vector4::Blend<0b0010>(Row2, Vector4::SplatZ(Col1));
                Row2 = Vector4::Blend<0b0100>(Row2, Vector4::SplatZ(Col2));
                Row2 = Vector4::Blend<0b1000>(Row2, Vector4(0.0f));

                const Real32 TranslateX = -Vector4::Dot3(Col0, Matrix.mColumns[3]);
                const Real32 TranslateY = -Vector4::Dot3(Col1, Matrix.mColumns[3]);
                const Real32 TranslateZ = -Vector4::Dot3(Col2, Matrix.mColumns[3]);
                return Matrix4x4(Row0, Row1, Row2, Vector4(TranslateX, TranslateY, TranslateZ, 1.0f));
            }
            else
            {
                ConstRef<Vector4> Col0 = Matrix.mColumns[0];
                ConstRef<Vector4> Col1 = Matrix.mColumns[1];
                ConstRef<Vector4> Col2 = Matrix.mColumns[2];
                ConstRef<Vector4> Col3 = Matrix.mColumns[3];

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
                LOG_ASSERT(!Base::IsAlmostZero(Determinant), "Matrix is singular, cannot invert");

                const Real32 InvDet = 1.0f / Determinant;
                return Matrix4x4(Inv0 * InvDet, Inv1 * InvDet, Inv2 * InvDet, Inv3 * InvDet);
            }
        }

        /// \brief Creates a perspective projection matrix.
        ///
        /// \param FovY   The field of view angle in radians.
        /// \param Aspect The aspect ratio (width/height).
        /// \param ZNear  The near plane distance.
        /// \param ZFar   The far plane distance.
        /// \return A perspective projection matrix.
        static Matrix4x4 CreatePerspective(Real32 FovY, Real32 Aspect, Real32 ZNear, Real32 ZFar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(ZFar - ZNear), "Invalid perspective matrix: near/far planes too close");
            LOG_ASSERT(ZFar > ZNear, "Invalid perspective matrix: ZFar must be greater than ZNear");

            const Real32 ScaleY = 1.0f / Tan(0.5f * FovY);
            const Real32 ScaleX = ScaleY / Aspect;

            return Matrix4x4(
                ScaleX, 0.0f,   0.0f,                              0.0f,
                0.0f,   ScaleY, 0.0f,                              0.0f,
                0.0f,   0.0f,   ZFar / (ZNear - ZFar),            -1.0f,
                0.0f,   0.0f,  (ZFar * ZNear) / (ZNear - ZFar),    0.0f);
        }

        /// \brief Creates an orthographic projection matrix.
        ///
        /// \param Left   The left boundary of the view volume.
        /// \param Right  The right boundary of the view volume.
        /// \param Bottom The bottom boundary of the view volume.
        /// \param Top    The top boundary of the view volume.
        /// \param ZNear  The near plane distance.
        /// \param ZFar   The far plane distance.
        /// \return An orthographic projection matrix.
        static Matrix4x4 CreateOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar)
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

        /// \brief Creates a view matrix for camera positioning.
        ///
        /// \param Eye   The camera's position in world space.
        /// \param Focus The target position the camera is looking at.
        /// \param Up    The up direction for the camera.
        /// \return A look-at transformation matrix.
        static Matrix4x4 CreateLook(ConstRef<Vector3> Eye, ConstRef<Vector3> Focus, ConstRef<Vector3> Up)
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

        /// \brief Creates a translation matrix from a 2D vector.
        ///
        /// \param Vector The 2D translation vector.
        /// \return A translation matrix based on the input vector.
        ZYPHRYON_INLINE static Matrix4x4 FromTranslation(ConstRef<Vector2> Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();

            return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             X,    Y,    0.0f, 1.0f);
        }

        /// \brief Creates a translation matrix from a 3D vector.
        ///
        /// \param Vector The 3D translation vector.
        /// \return A translation matrix based on the input vector.
        ZYPHRYON_INLINE static Matrix4x4 FromTranslation(ConstRef<Vector3> Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();
            const Real32 Z = Vector.GetZ();

            return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             X,    Y,     Z,   1.0f);
        }

        /// \brief Creates a scaling matrix from a 2D vector.
        ///
        /// \param Vector The 2D scaling vector.
        /// \return A scaling matrix based on the input vector.
        ZYPHRYON_INLINE static Matrix4x4 FromScale(ConstRef<Vector2> Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();

            return Matrix4x4(X,    0.0f, 0.0f, 0.0f,
                             0.0f, Y,    0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a scaling matrix from a 3D vector.
        ///
        /// \param Vector The 3D scaling vector.
        /// \return A scaling matrix based on the input vector.
        ZYPHRYON_INLINE static Matrix4x4 FromScale(ConstRef<Vector3> Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();
            const Real32 Z = Vector.GetZ();

            return Matrix4x4(X,    0.0f, 0.0f, 0.0f,
                             0.0f, Y,    0.0f, 0.0f,
                             0.0f, 0.0f, Z,    0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a rotation matrix from a quaternion.
        ///
        /// \param Rotation The rotation quaternion.
        /// \return A rotation matrix based on the input quaternion.
        static Matrix4x4 FromRotation(ConstRef<Quaternion> Rotation)
        {
            ZYPHRYON_ALIGN(16) Real32 Vector[4];
            Rotation.Store(Vector);

            const Real32 XX = 2.0f * Vector[0] * Vector[0];
            const Real32 YY = 2.0f * Vector[1] * Vector[1];
            const Real32 ZZ = 2.0f * Vector[2] * Vector[2];
            const Real32 XY = 2.0f * Vector[0] * Vector[1];
            const Real32 XZ = 2.0f * Vector[0] * Vector[2];
            const Real32 YZ = 2.0f * Vector[1] * Vector[2];
            const Real32 WX = 2.0f * Vector[3] * Vector[0];
            const Real32 WY = 2.0f * Vector[3] * Vector[1];
            const Real32 WZ = 2.0f * Vector[3] * Vector[2];

            const Vector4 C0(1.0f - (YY + ZZ), XY + WZ, XZ - WY, 0.0f);
            const Vector4 C1(XY - WZ, 1.0f - (XX + ZZ), YZ + WX, 0.0f);
            const Vector4 C2(XZ + WY, YZ - WX, 1.0f - (XX + YY), 0.0f);
            const Vector4 C3(0.0f, 0.0f, 0.0f, 1.0f);
            return Matrix4x4(C0, C1, C2, C3);
        }

        /// \brief Creates a transformation matrix from components.
        ///
        /// \param Translation The translation vector (x, y, z).
        /// \param Scale       The scale factors (x, y, z).
        /// \param Rotation    The rotation quaternion.
        /// \return Combined transformation matrix.
        ZYPHRYON_INLINE static Matrix4x4 FromTransform(ConstRef<Vector3> Translation, ConstRef<Vector3> Scale, ConstRef<Quaternion> Rotation)
        {
            Matrix4x4 Result = FromRotation(Rotation);

            Result.mColumns[0] *= Scale.GetX();
            Result.mColumns[1] *= Scale.GetY();
            Result.mColumns[2] *= Scale.GetZ();
            Result.mColumns[3].Set(Translation.GetX(), Translation.GetY(), Translation.GetZ(), 1.0f);
            return Result;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mColumns[4];
    };
}