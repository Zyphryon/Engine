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

#include "Matrix3x2.hpp"
#include "Quaternion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a column-major 4x4 floating-point transformation matrix.
    class ZY_ALIGN(16) Matrix4x4 final
    {
    public:

        /// \brief Constructs an identity matrix.
        ZY_INLINE Matrix4x4()
            : Matrix4x4 { 1.0f }
        {
        }

        /// \brief Constructs a diagonal matrix with specified value.
        ///
        /// \param Diagonal The value to set for all diagonal elements.
        ZY_INLINE explicit Matrix4x4(Real32 Diagonal)
            : Matrix4x4 { Diagonal, 0.0f, 0.0f, 0.0f,
                          0.0f, Diagonal, 0.0f, 0.0f,
                          0.0f, 0.0f, Diagonal, 0.0f,
                          0.0f, 0.0f, 0.0f, Diagonal }
        {
        }

        /// \brief Constructs a matrix from four column vectors.
        ///
        /// \param Column0 The first column vector of the matrix.
        /// \param Column1 The second column vector of the matrix.
        /// \param Column2 The third column vector of the matrix.
        /// \param Column3 The fourth column vector of the matrix.
        ZY_INLINE Matrix4x4(Vector4 Column0, Vector4 Column1, Vector4 Column2, Vector4 Column3)
            : mColumns { Column0, Column1, Column2, Column3 }
        {
        }

        /// \brief Constructs a matrix from 16 scalar values in column-major order.
        ///
        /// \param S00 Element at column 0, row 0.
        /// \param S10 Element at column 0, row 1.
        /// \param S20 Element at column 0, row 2.
        /// \param S30 Element at column 0, row 3.
        /// \param S01 Element at column 1, row 0.
        /// \param S11 Element at column 1, row 1.
        /// \param S21 Element at column 1, row 2.
        /// \param S31 Element at column 1, row 3.
        /// \param S02 Element at column 2, row 0.
        /// \param S12 Element at column 2, row 1.
        /// \param S22 Element at column 2, row 2.
        /// \param S32 Element at column 2, row 3.
        /// \param S03 Element at column 3, row 0.
        /// \param S13 Element at column 3, row 1.
        /// \param S23 Element at column 3, row 2.
        /// \param S33 Element at column 3, row 3.
        ZY_INLINE Matrix4x4(
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

        /// \brief Checks if this matrix is the identity matrix.
        ///
        /// \return `true` if all diagonal elements are approximately 1 and all off-diagonal elements are approximately 0, `false` otherwise.
        ZY_INLINE Bool IsIdentity() const
        {
            return mColumns[0].IsAlmostEqual(Vector4::UnitX())
                && mColumns[1].IsAlmostEqual(Vector4::UnitY())
                && mColumns[2].IsAlmostEqual(Vector4::UnitZ())
                && mColumns[3].IsAlmostEqual(Vector4::UnitW());
        }

        /// \brief Checks if this matrix is approximately equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if all corresponding elements are within the default floating-point tolerance, `false` otherwise.
        ZY_INLINE Bool IsAlmostEqual(ConstRef<Matrix4x4> Other) const
        {
            return mColumns[0].IsAlmostEqual(Other.mColumns[0])
                && mColumns[1].IsAlmostEqual(Other.mColumns[1])
                && mColumns[2].IsAlmostEqual(Other.mColumns[2])
                && mColumns[3].IsAlmostEqual(Other.mColumns[3]);
        }

        /// \brief Checks if the upper-left 3×3 sub-matrix is orthogonal.
        ///
        /// \return `true` if the basis vectors are mutually orthogonal and normalized, `false` otherwise.
        ZY_INLINE Bool IsOrthogonal() const
        {
            ZY_ASSERT(IsFinite(), "Matrix contains invalid (NaN/Inf) values before orthogonality check");

            const Vector3 X = mColumns[0].GetXYZ();
            const Vector3 Y = mColumns[1].GetXYZ();
            const Vector3 Z = mColumns[2].GetXYZ();

            return IsAlmostZero(Vector3::Dot(X, Y)) &&
                   IsAlmostZero(Vector3::Dot(X, Z)) &&
                   IsAlmostZero(Vector3::Dot(Y, Z)) &&
                   X.IsNormalized() &&
                   Y.IsNormalized() &&
                   Z.IsNormalized();
        }

        /// \brief Checks if all components of the matrix are finite (not NaN or Inf).
        ///
        /// \return `true` if every element is a finite floating-point value, `false` otherwise.
        ZY_INLINE Bool IsFinite() const
        {
            return mColumns[0].IsFinite() && mColumns[1].IsFinite() && mColumns[2].IsFinite() && mColumns[3].IsFinite();
        }

        /// \brief Gets a mutable reference to a column vector.
        ///
        /// \param Column The zero-based column index (0–3).
        /// \return A mutable reference to the requested column vector.
        ZY_INLINE Ref<Vector4> GetColumn(UInt32 Column)
        {
            ZY_ASSERT(Column < 4, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Gets a column vector by value.
        ///
        /// \param Column The zero-based column index (0–3).
        /// \return The column vector at the specified index.
        ZY_INLINE Vector4 GetColumn(UInt32 Column) const
        {
            ZY_ASSERT(Column < 4, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Gets the trace of the matrix (sum of diagonal elements).
        ///
        /// \return The sum of the four diagonal elements.
        ZY_INLINE Real32 GetTrace() const
        {
            return mColumns[0].GetX() + mColumns[1].GetY() + mColumns[2].GetZ() + mColumns[3].GetW();
        }

        /// \brief Gets the right direction vector from the matrix (positive X axis).
        ///
        /// \return The XYZ components of the first column.
        ZY_INLINE Vector3 GetRight() const
        {
            return mColumns[0].GetXYZ();
        }

        /// \brief Gets the left direction vector from the matrix (negative X axis).
        ///
        /// \return The negated XYZ components of the first column.
        ZY_INLINE Vector3 GetLeft() const
        {
            return -GetRight();
        }

        /// \brief Gets the up direction vector from the matrix (positive Y axis).
        ///
        /// \return The XYZ components of the second column.
        ZY_INLINE Vector3 GetUp() const
        {
            return mColumns[1].GetXYZ();
        }

        /// \brief Gets the down direction vector from the matrix (negative Y axis).
        ///
        /// \return The negated XYZ components of the second column.
        ZY_INLINE Vector3 GetDown() const
        {
            return -GetUp();
        }

        /// \brief Gets the forward direction vector from the matrix (positive Z axis).
        ///
        /// \return The XYZ components of the third column.
        ZY_INLINE Vector3 GetForward() const
        {
            return mColumns[2].GetXYZ();
        }

        /// \brief Gets the backward direction vector from the matrix (negative Z axis).
        ///
        /// \return The negated XYZ components of the third column.
        ZY_INLINE Vector3 GetBackward() const
        {
            return -GetForward();
        }

        /// \brief Gets the per-axis scale factors encoded in the matrix.
        ///
        /// \return A \ref Vector3 containing the scale of each axis (X, Y, Z).
        ZY_INLINE Vector3 GetScale() const
        {
            const Real32 ScaleX = mColumns[0].GetLength();
            const Real32 ScaleY = mColumns[1].GetLength();
            const Real32 ScaleZ = mColumns[2].GetLength();

            return Vector3(ScaleX, ScaleY, ScaleZ);
        }

        /// \brief Gets the translation component of the transformation matrix.
        ///
        /// \return The XYZ components of the fourth column.
        ZY_INLINE Vector3 GetTranslation() const
        {
            return mColumns[3].GetXYZ();
        }

        /// \brief Gets the rotation component of the transformation matrix as a quaternion.
        ///
        /// Use \ref Decompose when the matrix may carry a non-uniform scale.
        ///
        /// \return A \ref Quaternion representing the rotation encoded in the upper-left 3×3 sub-matrix.
        ZY_INLINE Quaternion GetRotation() const
        {
            return Quaternion::FromDirection(mColumns[2].GetXYZ(), mColumns[1].GetXYZ());
        }

        /// \brief Adds another matrix to this matrix element-wise.
        ///
        /// \param Other The matrix to add.
        /// \return A new matrix that is the element-wise sum.
        ZY_INLINE Matrix4x4 operator+(ConstRef<Matrix4x4> Other) const
        {
            return Matrix4x4(
                mColumns[0] + Other.mColumns[0],
                mColumns[1] + Other.mColumns[1],
                mColumns[2] + Other.mColumns[2],
                mColumns[3] + Other.mColumns[3]);
        }

        /// \brief Subtracts another matrix from this matrix element-wise.
        ///
        /// \param Other The matrix to subtract.
        /// \return A new matrix that is the element-wise difference.
        ZY_INLINE Matrix4x4 operator-(ConstRef<Matrix4x4> Other) const
        {
            return Matrix4x4(
                mColumns[0] - Other.mColumns[0],
                mColumns[1] - Other.mColumns[1],
                mColumns[2] - Other.mColumns[2],
                mColumns[3] - Other.mColumns[3]);
        }

        /// \brief Multiplies this matrix by another matrix.
        ///
        /// \param Other The right-hand matrix operand.
        /// \return A new matrix that is the product of the two matrices.
        ZY_INLINE Matrix4x4 operator*(ConstRef<Matrix4x4> Other) const
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

        /// \brief Scales all elements of this matrix by a scalar value.
        ///
        /// \param Scalar The scalar to multiply every element by.
        /// \return A new matrix with all elements scaled by the given value.
        ZY_INLINE Matrix4x4 operator*(Real32 Scalar) const
        {
            return Matrix4x4(
                mColumns[0] * Scalar,
                mColumns[1] * Scalar,
                mColumns[2] * Scalar,
                mColumns[3] * Scalar);
        }

        /// \brief Divides all elements of this matrix by a scalar value.
        ///
        /// \param Scalar The scalar to divide every element by.
        /// \return A new matrix with all elements divided by the given value.
        ZY_INLINE Matrix4x4 operator/(Real32 Scalar) const
        {
            ZY_ASSERT(!::IsAlmostZero(Scalar), "Division by zero");

            const Real32 InvScalar = 1.0f / Scalar;
            return (* this) * InvScalar;
        }

        /// \brief Adds another matrix to this matrix in-place.
        ///
        /// \param Other The matrix to add.
        /// \return A reference to the updated matrix.
        ZY_INLINE Ref<Matrix4x4> operator+=(ConstRef<Matrix4x4> Other)
        {
            mColumns[0] += Other.mColumns[0];
            mColumns[1] += Other.mColumns[1];
            mColumns[2] += Other.mColumns[2];
            mColumns[3] += Other.mColumns[3];
            return (* this);
        }

        /// \brief Subtracts another matrix from this matrix in-place.
        ///
        /// \param Other The matrix to subtract.
        /// \return A reference to the updated matrix.
        ZY_INLINE Ref<Matrix4x4> operator-=(ConstRef<Matrix4x4> Other)
        {
            mColumns[0] -= Other.mColumns[0];
            mColumns[1] -= Other.mColumns[1];
            mColumns[2] -= Other.mColumns[2];
            mColumns[3] -= Other.mColumns[3];
            return (* this);
        }

        /// \brief Multiplies this matrix by another matrix in-place.
        ///
        /// \param Other The matrix to multiply by.
        /// \return A reference to the updated matrix.
        ZY_INLINE Ref<Matrix4x4> operator*=(ConstRef<Matrix4x4> Other)
        {
            const Vector4 C0 = mColumns[0];
            const Vector4 C1 = mColumns[1];
            const Vector4 C2 = mColumns[2];
            const Vector4 C3 = mColumns[3];

            mColumns[0] = C0 * Vector4::SplatX(Other.mColumns[0]) +
                          C1 * Vector4::SplatY(Other.mColumns[0]) +
                          C2 * Vector4::SplatZ(Other.mColumns[0]) +
                          C3 * Vector4::SplatW(Other.mColumns[0]);

            mColumns[1] = C0 * Vector4::SplatX(Other.mColumns[1]) +
                          C1 * Vector4::SplatY(Other.mColumns[1]) +
                          C2 * Vector4::SplatZ(Other.mColumns[1]) +
                          C3 * Vector4::SplatW(Other.mColumns[1]);

            mColumns[2] = C0 * Vector4::SplatX(Other.mColumns[2]) +
                          C1 * Vector4::SplatY(Other.mColumns[2]) +
                          C2 * Vector4::SplatZ(Other.mColumns[2]) +
                          C3 * Vector4::SplatW(Other.mColumns[2]);

            mColumns[3] = C0 * Vector4::SplatX(Other.mColumns[3]) +
                          C1 * Vector4::SplatY(Other.mColumns[3]) +
                          C2 * Vector4::SplatZ(Other.mColumns[3]) +
                          C3 * Vector4::SplatW(Other.mColumns[3]);

            return (* this);
        }

        /// \brief Checks if this matrix is equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if all corresponding elements are exactly equal, `false` otherwise.
        ZY_INLINE Bool operator==(ConstRef<Matrix4x4> Other) const
        {
            return mColumns[0].IsAlmostEqual(Other.mColumns[0])
                && mColumns[1].IsAlmostEqual(Other.mColumns[1])
                && mColumns[2].IsAlmostEqual(Other.mColumns[2])
                && mColumns[3].IsAlmostEqual(Other.mColumns[3]);
        }

        /// \brief Checks if this matrix is not equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if any corresponding element differs, `false` otherwise.
        ZY_INLINE Bool operator!=(ConstRef<Matrix4x4> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<9> kPattern("({0}, {1}, {2}, {3})");
            Format::Processor<Output>::Format(Buffer, kPattern, mColumns[0], mColumns[1], mColumns[2], mColumns[3]);
        }

    public:

        /// \brief Gets the 4x4 identity matrix.
        ///
        /// \return A matrix with 1 on the diagonal and 0 elsewhere.
        ZY_INLINE static Matrix4x4 Identity()
        {
            return Matrix4x4();
        }

        /// \brief Computes the determinant of a 4×4 matrix.
        ///
        /// \param Matrix The matrix to compute the determinant for.
        /// \return The scalar determinant value. A value of 0 indicates a singular matrix.
        static Real32 GetDeterminant(ConstRef<Matrix4x4> Matrix);

        /// \brief Projects a 2D point using a 4×4 transformation matrix.
        ///
        /// \tparam Affine When `true`, skips the perspective divide and returns the raw XY result.
        /// \param Matrix  The transformation matrix to apply.
        /// \param Vector  The 2D point to project.
        /// \return The projected 2D point.
        template<Bool Affine>
        ZY_INLINE static Vector2 Project(ConstRef<Matrix4x4> Matrix, Vector2 Vector)
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
                ZY_ASSERT(!::IsAlmostZero(W), "Division by zero (W)");

                const Real32 InvW = 1.0f / W;
                return Vector2(Result.GetX() * InvW, Result.GetY() * InvW);
            }
        }

        /// \brief Projects a 3D point using a 4×4 transformation matrix.
        ///
        /// \tparam Affine When `true`, skips the perspective divide and returns the raw XYZ result.
        /// \param Matrix  The transformation matrix to apply.
        /// \param Vector  The 3D point to project.
        /// \return The projected 3D point.
        template<Bool Affine>
        ZY_INLINE static Vector3 Project(ConstRef<Matrix4x4> Matrix, Vector3 Vector)
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
                ZY_ASSERT(!::IsAlmostZero(W), "Division by zero (W)");

                const Real32 InvW = 1.0f / W;
                return Vector3(Result.GetX() * InvW, Result.GetY() * InvW, Result.GetZ() * InvW);
            }
        }

        /// \brief Projects a 4D vector using a 4×4 transformation matrix.
        ///
        /// \tparam Affine When `true`, returns the result without perspective division.
        /// \param Matrix  The transformation matrix to apply.
        /// \param Vector  The 4D vector to project.
        /// \return The projected 4D vector.
        template<Bool Affine>
        ZY_INLINE static Vector4 Project(ConstRef<Matrix4x4> Matrix, Vector4 Vector)
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
                ZY_ASSERT(!::IsAlmostZero(W), "Division by zero (W)");

                return (W != 0.0f ? Result * (1.0f / W) : Result);
            }
        }

        /// \brief Computes the transpose of a 4×4 matrix.
        ///
        /// \param Matrix The matrix to transpose.
        /// \return The transposed matrix.
        ZY_INLINE static Matrix4x4 Transpose(ConstRef<Matrix4x4> Matrix)
        {
            const Vector4 Temp0 = Vector4::ZipLow(Matrix.mColumns[0], Matrix.mColumns[1]);
            const Vector4 Temp1 = Vector4::ZipHigh(Matrix.mColumns[0], Matrix.mColumns[1]);
            const Vector4 Temp2 = Vector4::ZipLow(Matrix.mColumns[2], Matrix.mColumns[3]);
            const Vector4 Temp3 = Vector4::ZipHigh(Matrix.mColumns[2], Matrix.mColumns[3]);

            return Matrix4x4(
                Vector4::MergeLow(Temp0, Temp2),
                Vector4::MergeHigh(Temp2, Temp0),
                Vector4::MergeLow(Temp1, Temp3),
                Vector4::MergeHigh(Temp3, Temp1));
        }

        /// \brief Computes the inverse of a general 4×4 matrix using cofactor expansion.
        ///
        /// For matrices that are known to be affine (no perspective), prefer \ref InverseAffine.
        ///
        /// \param Matrix The matrix to invert.
        /// \return The inverse of the given matrix.
        static Matrix4x4 Inverse(ConstRef<Matrix4x4> Matrix);

        /// \brief Computes the inverse of an affine 4×4 matrix.
        ///
        /// \param Matrix The affine matrix to invert.
        /// \return The inverse of the given affine matrix.
        static Matrix4x4 InverseAffine(ConstRef<Matrix4x4> Matrix);

        /// \brief Performs a component-wise linear interpolation between two matrices.
        ///
        /// \param From   The starting matrix (returned when \p Alpha is 0).
        /// \param To     The ending matrix (returned when \p Alpha is 1).
        /// \param Alpha  The interpolation factor in the range [0, 1].
        /// \return The interpolated matrix.
        static Matrix4x4 Lerp(ConstRef<Matrix4x4> From, ConstRef<Matrix4x4> To, Real32 Alpha);

        /// \brief Decomposes a TRS matrix into its translation, scale, and rotation components.
        ///
        /// Assumes the matrix was built as T * R * S (or an equivalent affine combination).
        /// The result is undefined for matrices that contain a perspective projection or
        /// non-uniform shear.
        ///
        /// \param Matrix      The TRS matrix to decompose.
        /// \param Translation The extracted translation vector.
        /// \param Scale       The extracted scale vector (always positive).
        /// \param Rotation    The extracted rotation quaternion (unit quaternion).
        static void Decompose(ConstRef<Matrix4x4> Matrix, Ref<Vector3> Translation, Ref<Vector3> Scale, Ref<Quaternion> Rotation);

        /// \brief Creates a right-handed perspective projection matrix with depth in [0, 1].
        ///
        /// \param FovY   The vertical field-of-view angle.
        /// \param Aspect The viewport aspect ratio (width / height).
        /// \param ZNear  The distance to the near clip plane (must be greater than 0).
        /// \param ZFar   The distance to the far clip plane (must be greater than ZNear).
        /// \return A perspective projection matrix.
        static Matrix4x4 CreatePerspective(Angle FovY, Real32 Aspect, Real32 ZNear, Real32 ZFar);

        /// \brief Creates a right-handed orthographic projection matrix with depth in [0, 1].
        ///
        /// \param Left   The left boundary of the view volume.
        /// \param Right  The right boundary of the view volume.
        /// \param Bottom The bottom boundary of the view volume.
        /// \param Top    The top boundary of the view volume.
        /// \param ZNear  The near plane distance.
        /// \param ZFar   The far plane distance.
        /// \return An orthographic projection matrix.
        static Matrix4x4 CreateOrthographic(Real32 Left, Real32 Right, Real32 Bottom, Real32 Top, Real32 ZNear, Real32 ZFar);

        /// \brief Creates a right-handed look-at view matrix given a target position.
        ///
        /// \param Eye   The camera position in world space.
        /// \param Focus The target position the camera looks toward.
        /// \param Up    The world-space up direction (must be normalized and non-parallel to the view direction).
        /// \return A look-at view matrix.
        ZY_INLINE static Matrix4x4 CreateLook(Vector3 Eye, Vector3 Focus, Vector3 Up)
        {
            return CreateDirection(Eye, Vector3::Normalize(Eye - Focus), Up);
        }

        /// \brief Creates a right-handed view matrix from an explicit forward direction.
        ///
        /// \param Eye       The camera position in world space.
        /// \param Direction The normalized forward direction the camera is facing.
        /// \param Up        The world-space up direction (must be normalized and non-parallel to Direction).
        /// \return A view matrix oriented along the given direction.
        static Matrix4x4 CreateDirection(Vector3 Eye, Vector3 Direction, Vector3 Up);

        /// \brief Creates a translation matrix from a 2D vector.
        ///
        /// \param Vector The 2D translation offset (Z is set to 0).
        /// \return A column-major translation matrix.
        ZY_INLINE static Matrix4x4 FromTranslation(Vector2 Vector)
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
        /// \param Vector The 3D translation offset.
        /// \return A column-major translation matrix.
        ZY_INLINE static Matrix4x4 FromTranslation(Vector3 Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();
            const Real32 Z = Vector.GetZ();

            return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             X,    Y,     Z,   1.0f);
        }

        /// \brief Creates a non-uniform scaling matrix from a 2D vector.
        ///
        /// \param Vector The scale factors along X and Y.
        /// \return A scaling matrix.
        ZY_INLINE static Matrix4x4 FromScale(Vector2 Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();

            return Matrix4x4(X,    0.0f, 0.0f, 0.0f,
                             0.0f, Y,    0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a non-uniform scaling matrix from a 3D vector.
        ///
        /// \param Vector The scale factors along X, Y, and Z.
        /// \return A scaling matrix.
        ZY_INLINE static Matrix4x4 FromScale(Vector3 Vector)
        {
            const Real32 X = Vector.GetX();
            const Real32 Y = Vector.GetY();
            const Real32 Z = Vector.GetZ();

            return Matrix4x4(X,    0.0f, 0.0f, 0.0f,
                             0.0f, Y,    0.0f, 0.0f,
                             0.0f, 0.0f, Z,    0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a rotation matrix around the world X axis (pitch).
        ///
        /// \param Rotation The rotation angle.
        /// \return A rotation matrix.
        ZY_INLINE static Matrix4x4 FromRotationX(Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            return Matrix4x4(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, C,    S,    0.0f,
                             0.0f, -S,   C,    0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a rotation matrix around the world Y axis (yaw).
        ///
        /// \param Rotation The rotation angle.
        /// \return A rotation matrix.
        ZY_INLINE static Matrix4x4 FromRotationY(Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            return Matrix4x4(C,    0.0f, -S,   0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             S,    0.0f, C,    0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a rotation matrix around the world Z axis (roll).
        ///
        /// \param Rotation The rotation angle.
        /// \return A rotation matrix.
        ZY_INLINE static Matrix4x4 FromRotationZ(Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            return Matrix4x4(C,    S,    0.0f, 0.0f,
                             -S,   C,    0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f);
        }

        /// \brief Creates a rotation matrix from a quaternion.
        ///
        /// \param Rotation The unit quaternion representing the desired rotation.
        /// \return A 4×4 rotation matrix equivalent to the quaternion.
        ZY_INLINE static Matrix4x4 FromRotation(Quaternion Rotation)
        {
            ZY_ALIGN(16) Real32 XYZW[4];
            Rotation.Store(XYZW);

            const Real32 XX = 2.0f * XYZW[0] * XYZW[0];
            const Real32 YY = 2.0f * XYZW[1] * XYZW[1];
            const Real32 ZZ = 2.0f * XYZW[2] * XYZW[2];
            const Real32 XY = 2.0f * XYZW[0] * XYZW[1];
            const Real32 XZ = 2.0f * XYZW[0] * XYZW[2];
            const Real32 YZ = 2.0f * XYZW[1] * XYZW[2];
            const Real32 WX = 2.0f * XYZW[3] * XYZW[0];
            const Real32 WY = 2.0f * XYZW[3] * XYZW[1];
            const Real32 WZ = 2.0f * XYZW[3] * XYZW[2];

            const Vector4 C0(1.0f - (YY + ZZ), XY + WZ, XZ - WY, 0.0f);
            const Vector4 C1(XY - WZ, 1.0f - (XX + ZZ), YZ + WX, 0.0f);
            const Vector4 C2(XZ + WY, YZ - WX, 1.0f - (XX + YY), 0.0f);
            const Vector4 C3(0.0f, 0.0f, 0.0f, 1.0f);
            return Matrix4x4(C0, C1, C2, C3);
        }

        /// \brief Promotes a 2D affine Matrix3x2 to a column-major 4×4 matrix.
        ///
        /// \param Matrix The 2D affine matrix to promote.
        /// \return A 4×4 matrix equivalent to the 2D affine transform.
        ZY_INLINE static Matrix4x4 FromMatrix3x2(ConstRef<Matrix3x2> Matrix)
        {
            const Vector3 C0 = Matrix.GetColumn(0);
            const Vector3 C1 = Matrix.GetColumn(1);

            return Matrix4x4(C0.GetX(),  C1.GetX(),  0.0f, 0.0f,
                             C0.GetY(),  C1.GetY(),  0.0f, 0.0f,
                             0.0f,            0.0f,  1.0f, 0.0f,
                             C0.GetZ(),  C1.GetZ(),  0.0f, 1.0f);
        }

        /// \brief Creates a combined TRS (translation × rotation × scale) matrix.
        ///
        /// \param Translation The world-space position.
        /// \param Scale       The per-axis scale factors.
        /// \param Rotation    The rotation quaternion (should be normalized).
        /// \return The combined TRS transformation matrix.
        ZY_INLINE static Matrix4x4 FromTransform(Vector3 Translation, Vector3 Scale, Quaternion Rotation)
        {
            Matrix4x4 Result = FromRotation(Rotation);

            Result.mColumns[0] *= Scale.GetX();
            Result.mColumns[1] *= Scale.GetY();
            Result.mColumns[2] *= Scale.GetZ();
            Result.mColumns[3]  = Vector4(Translation.GetX(), Translation.GetY(), Translation.GetZ(), 1.0f);
            return Result;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mColumns[4];
    };
}