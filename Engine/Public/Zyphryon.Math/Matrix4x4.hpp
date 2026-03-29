// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

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
        ZYPHRYON_INLINE explicit Matrix4x4(Real32 Diagonal)
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
        /// \param S00 Element at column 0, row 0
        /// \param S10 Element at column 0, row 1
        /// \param S20 Element at column 0, row 2
        /// \param S30 Element at column 0, row 3
        /// \param S01 Element at column 1, row 0
        /// \param S11 Element at column 1, row 1
        /// \param S21 Element at column 1, row 2
        /// \param S31 Element at column 1, row 3
        /// \param S02 Element at column 2, row 0
        /// \param S12 Element at column 2, row 1
        /// \param S22 Element at column 2, row 2
        /// \param S32 Element at column 2, row 3
        /// \param S03 Element at column 3, row 0
        /// \param S13 Element at column 3, row 1
        /// \param S23 Element at column 3, row 2
        /// \param S33 Element at column 3, row 3
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

            return (Transpose(* this) * (* this)).IsIdentity();
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
        ZYPHRYON_INLINE Vector4 GetColumn(UInt32 Column) const
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
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Matrix4x4> Other) const
        {
            return !(* this == Other);
        }

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

        /// \brief Gets the determinant of a 4x4 matrix.
        ///
        /// \param Matrix The matrix to compute the determinant for.
        /// \return The determinant value.
        static Real32 GetDeterminant(ConstRef<Matrix4x4> Matrix);

        /// \brief Projects a 2D vector using a 4x4 transformation matrix.
        ///
        /// \tparam Affine When `true`, uses the affine fast path, otherwise uses the general 4×4 inverse.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 2D vector to be projected.
        /// \return A 2D vector resulting from the projection.
        template<Bool Affine>
        ZYPHRYON_INLINE static Vector2 Project(ConstRef<Matrix4x4> Matrix, Vector2 Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4(Vector.GetX()) +
                Matrix.mColumns[1] * Vector4(Vector.GetY()) +
                Matrix.mColumns[3];

            if constexpr(Affine)
            {
                return Result.GetXY();
            }
            else
            {
                const Real32 W = Result.GetW();
                LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

                const Real32 InvW = 1.0f / W;
                return Vector2(Result.GetX() * InvW, Result.GetY() * InvW);
            }
        }

        /// \brief Projects a 3D vector using a 4x4 transformation matrix.
        ///
        /// \tparam Affine When `true`, uses the affine fast path, otherwise uses the general 4×4 inverse.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 3D vector to be projected.
        /// \return A 3D vector resulting from the projection.
        template<Bool Affine>
        ZYPHRYON_INLINE static Vector3 Project(ConstRef<Matrix4x4> Matrix, Vector3 Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4(Vector.GetX()) +
                Matrix.mColumns[1] * Vector4(Vector.GetY()) +
                Matrix.mColumns[2] * Vector4(Vector.GetZ()) +
                Matrix.mColumns[3];

            if constexpr(Affine)
            {
                return Result.GetXYZ();
            }
            else
            {
                const Real32 W = Result.GetW();
                LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

                const Real32 InvW = 1.0f / W;
                return Vector3(Result.GetX() * InvW, Result.GetY() * InvW, Result.GetZ() * InvW);
            }
        }

        /// \brief Projects a 4D vector using a 4x4 transformation matrix.
        ///
        /// \tparam Affine When `true`, uses the affine fast path, otherwise uses the general 4×4 inverse.
        ///
        /// \param Matrix The transformation matrix to apply to the vector.
        /// \param Vector The 4D vector to be projected.
        /// \return A 4D vector resulting from the projection.
        template<Bool Affine>
        ZYPHRYON_INLINE static Vector4 Project(ConstRef<Matrix4x4> Matrix, Vector4 Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4::SplatX(Vector) +
                Matrix.mColumns[1] * Vector4::SplatY(Vector) +
                Matrix.mColumns[2] * Vector4::SplatZ(Vector) +
                Matrix.mColumns[3] * Vector4::SplatW(Vector);

            if constexpr(Affine)
            {
                return Result;
            }
            else
            {
                const Real32 W = Result.GetW();
                LOG_ASSERT(!Base::IsAlmostZero(W), "Division by zero (W)");

                return (W != 0.0f ? Result * (1.0f / W) : Result);
            }
        }

        /// \brief Computes the transpose of a 4x4 matrix.
        ///
        /// \param Matrix The matrix to transpose.
        /// \return The transposed 4x4 matrix.
        ZYPHRYON_INLINE static Matrix4x4 Transpose(ConstRef<Matrix4x4> Matrix)
        {
            const Vector4 Temp0 = Vector4::InterleaveLow(Matrix.mColumns[0], Matrix.mColumns[1]);
            const Vector4 Temp1 = Vector4::InterleaveHigh(Matrix.mColumns[0], Matrix.mColumns[1]);
            const Vector4 Temp2 = Vector4::InterleaveLow(Matrix.mColumns[2], Matrix.mColumns[3]);
            const Vector4 Temp3 = Vector4::InterleaveHigh(Matrix.mColumns[2], Matrix.mColumns[3]);

            return Matrix4x4(
                Vector4::Shuffle0101(Temp0, Temp2),
                Vector4::Shuffle2323(Temp2, Temp0),
                Vector4::Shuffle0101(Temp1, Temp3),
                Vector4::Shuffle2323(Temp3, Temp1));
        }

        /// \brief Computes the inverse of a general 4x4 matrix.
        ///
        /// \param Matrix The general matrix to invert.
        /// \return The inverted general 4x4 matrix.
        static Matrix4x4 Inverse(ConstRef<Matrix4x4> Matrix);

        /// \brief Computes the inverse of an affine 4x4 matrix.
        ///
        /// \param Matrix The affine matrix to invert.
        /// \return The inverted affine 4x4 matrix.
        static Matrix4x4 InverseAffine(ConstRef<Matrix4x4> Matrix);

        /// \brief Creates a perspective projection matrix.
        ///
        /// \param FovY   The field of view angle in radians.
        /// \param Aspect The aspect ratio (width/height).
        /// \param ZNear  The near plane distance.
        /// \param ZFar   The far plane distance.
        /// \return A perspective projection matrix.
        static Matrix4x4 CreatePerspective(Angle FovY, Real32 Aspect, Real32 ZNear, Real32 ZFar);

        /// \brief Creates an orthographic projection matrix.
        ///
        /// \param Left   The left boundary of the view volume.
        /// \param Right  The right boundary of the view volume.
        /// \param Bottom The bottom boundary of the view volume.
        /// \param Top    The top boundary of the view volume.
        /// \param ZNear  The near plane distance.
        /// \param ZFar   The far plane distance.
        /// \return An orthographic projection matrix.
        static Matrix4x4 CreateOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar);

        /// \brief Creates a view matrix for camera positioning.
        ///
        /// \param Eye   The camera's position in world space.
        /// \param Focus The target position the camera is looking at.
        /// \param Up    The up direction for the camera.
        /// \return A look-at transformation matrix.
        static Matrix4x4 CreateLook(Vector3 Eye, Vector3 Focus, Vector3 Up);

        /// \brief Creates a translation matrix from a 2D vector.
        ///
        /// \param Vector The 2D translation vector.
        /// \return A translation matrix based on the input vector.
        ZYPHRYON_INLINE static Matrix4x4 FromTranslation(Vector2 Vector)
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
        ZYPHRYON_INLINE static Matrix4x4 FromTranslation(Vector3 Vector)
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
        ZYPHRYON_INLINE static Matrix4x4 FromScale(Vector2 Vector)
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
        ZYPHRYON_INLINE static Matrix4x4 FromScale(Vector3 Vector)
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
        ZYPHRYON_INLINE static Matrix4x4 FromRotation(Quaternion Rotation)
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
        ZYPHRYON_INLINE static Matrix4x4 FromTransform(Vector3 Translation, Vector3 Scale, Quaternion Rotation)
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