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
    /// \brief Represents an affine 3D transformation matrix, stored as the three non-constant rows of a 3x4.
    class ZY_ALIGN(16) Matrix4x3 final
    {
    public:

        /// \brief A matrix transposed into the basis axes and translation, ready to transform points.
        struct Basis final
        {
            /// \brief Transposes a matrix into its columns.
            ///
            /// \param Matrix The matrix to read.
            ZY_INLINE explicit Basis(ConstRef<Matrix4x3> Matrix)
            {
                const Vector4 Zero  = Vector4::Zero();
                const Vector4 Temp0 = Vector4::ZipLow(Matrix.mColumns[0], Matrix.mColumns[1]);
                const Vector4 Temp1 = Vector4::ZipHigh(Matrix.mColumns[0], Matrix.mColumns[1]);
                const Vector4 Temp2 = Vector4::ZipLow(Matrix.mColumns[2], Zero);
                const Vector4 Temp3 = Vector4::ZipHigh(Matrix.mColumns[2], Zero);

                AxisX  = Vector4::MergeLow(Temp0, Temp2);
                AxisY  = Vector4::MergeHigh(Temp0, Temp2);
                AxisZ  = Vector4::MergeLow(Temp1, Temp3);
                Offset = Vector4::MergeHigh(Temp1, Temp3);
            }

            /// \brief Projects a 2D point, treating it as lying on the Z = 0 plane.
            ///
            /// \param Vector The 2D point to project.
            /// \return The projected 2D point, translation included.
            ZY_INLINE Vector2 Project(Vector2 Vector) const
            {
                return (AxisX * Vector4(Vector.GetX())
                      + AxisY * Vector4(Vector.GetY())
                      + Offset).GetXY();
            }

            /// \brief Projects a 3D point.
            ///
            /// \param Vector The 3D point to project.
            /// \return The projected 3D point, translation included.
            ZY_INLINE Vector3 Project(Vector3 Vector) const
            {
                return (AxisX * Vector4(Vector.GetX())
                      + AxisY * Vector4(Vector.GetY())
                      + AxisZ * Vector4(Vector.GetZ())
                      + Offset).GetXYZ();
            }

            /// The transformed X axis, with the X translation in W.
            Vector4 AxisX;

            /// The transformed Y axis, with the Y translation in W.
            Vector4 AxisY;

            /// The transformed Z axis, with the Z translation in W.
            Vector4 AxisZ;

            /// The translation, with a trailing one.
            Vector4 Offset;
        };

    public:

        /// \brief Constructs an identity matrix.
        ZY_INLINE Matrix4x3()
            : mColumns { Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                         Vector4(0.0f, 1.0f, 0.0f, 0.0f),
                         Vector4(0.0f, 0.0f, 1.0f, 0.0f) }
        {
        }

        /// \brief Constructs a matrix from three column vectors.
        ///
        /// \param Column0 The first column, carrying the X row of the basis and the X translation in W.
        /// \param Column1 The second column, carrying the Y row of the basis and the Y translation in W.
        /// \param Column2 The third column, carrying the Z row of the basis and the Z translation in W.
        ZY_INLINE Matrix4x3(Vector4 Column0, Vector4 Column1, Vector4 Column2)
            : mColumns { Column0, Column1, Column2 }
        {
        }

        /// \brief Constructs a matrix from 12 scalar values in column-major order.
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
        ZY_INLINE Matrix4x3(
            Real32 S00, Real32 S10, Real32 S20, Real32 S30,
            Real32 S01, Real32 S11, Real32 S21, Real32 S31,
            Real32 S02, Real32 S12, Real32 S22, Real32 S32)
            : mColumns { Vector4(S00, S10, S20, S30),
                         Vector4(S01, S11, S21, S31),
                         Vector4(S02, S12, S22, S32) }
        {
        }

        /// \brief Checks if this matrix is the identity matrix.
        ///
        /// \return `true` if the stored columns are approximately the X, Y and Z unit axes, `false` otherwise.
        ZY_INLINE Bool IsIdentity() const
        {
            return mColumns[0].IsAlmostEqual<kTolerance<Real32>>(Vector4::UnitX())
                && mColumns[1].IsAlmostEqual<kTolerance<Real32>>(Vector4::UnitY())
                && mColumns[2].IsAlmostEqual<kTolerance<Real32>>(Vector4::UnitZ());
        }

        /// \brief Checks if this matrix is approximately equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if all corresponding elements are within the default floating-point tolerance, `false` otherwise.
        ZY_INLINE Bool IsAlmostEqual(ConstRef<Matrix4x3> Other) const
        {
            return mColumns[0].IsAlmostEqual(Other.mColumns[0])
                && mColumns[1].IsAlmostEqual(Other.mColumns[1])
                && mColumns[2].IsAlmostEqual(Other.mColumns[2]);
        }

        /// \brief Checks if the basis is orthogonal.
        ///
        /// \return `true` if the basis vectors are mutually orthogonal and normalized, `false` otherwise.
        ZY_INLINE Bool IsOrthogonal() const
        {
            ZY_ASSERT(IsFinite(), "Matrix contains invalid (NaN/Inf) values before orthogonality check");

            const Vector3 X = GetBasisX();
            const Vector3 Y = GetBasisY();
            const Vector3 Z = GetBasisZ();

            return IsAlmostZero(Vector3::Dot(X, Y), kTolerance<Real32>) &&
                   IsAlmostZero(Vector3::Dot(X, Z), kTolerance<Real32>) &&
                   IsAlmostZero(Vector3::Dot(Y, Z), kTolerance<Real32>) &&
                   X.IsNormalized() &&
                   Y.IsNormalized() &&
                   Z.IsNormalized();
        }

        /// \brief Checks if all components of the matrix are finite (not NaN or Inf).
        ///
        /// \return `true` if every element is a finite floating-point value, `false` otherwise.
        ZY_INLINE Bool IsFinite() const
        {
            return mColumns[0].IsFinite() && mColumns[1].IsFinite() && mColumns[2].IsFinite();
        }

        /// \brief Gets a mutable reference to a column vector.
        ///
        /// \param Column The zero-based column index (0–2).
        /// \return A mutable reference to the requested column vector.
        ZY_INLINE Ref<Vector4> GetColumn(UInt32 Column)
        {
            ZY_ASSERT(Column < 3, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Gets a column vector by value.
        ///
        /// \param Column The zero-based column index (0–2).
        /// \return The column vector at the specified index.
        ZY_INLINE Vector4 GetColumn(UInt32 Column) const
        {
            ZY_ASSERT(Column < 3, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Gets the trace of the equivalent 4x4 matrix (sum of diagonal elements).
        ///
        /// \return The sum of the three stored diagonal elements plus the implicit fourth, which is always 1.
        ZY_INLINE Real32 GetTrace() const
        {
            return mColumns[0].GetX() + mColumns[1].GetY() + mColumns[2].GetZ() + 1.0f;
        }

        /// \brief Gets the transformed X axis of the basis.
        ///
        /// \return The X component of every column.
        ZY_INLINE Vector3 GetBasisX() const
        {
            return Vector3(mColumns[0].GetX(), mColumns[1].GetX(), mColumns[2].GetX());
        }

        /// \brief Gets the transformed Y axis of the basis.
        ///
        /// \return The Y component of every column.
        ZY_INLINE Vector3 GetBasisY() const
        {
            return Vector3(mColumns[0].GetY(), mColumns[1].GetY(), mColumns[2].GetY());
        }

        /// \brief Gets the transformed Z axis of the basis.
        ///
        /// \return The Z component of every column.
        ZY_INLINE Vector3 GetBasisZ() const
        {
            return Vector3(mColumns[0].GetZ(), mColumns[1].GetZ(), mColumns[2].GetZ());
        }

        /// \brief Gets the right direction vector from the matrix (positive X axis).
        ///
        /// \return The transformed X axis.
        ZY_INLINE Vector3 GetRight() const
        {
            return GetBasisX();
        }

        /// \brief Gets the left direction vector from the matrix (negative X axis).
        ///
        /// \return The negated transformed X axis.
        ZY_INLINE Vector3 GetLeft() const
        {
            return -GetBasisX();
        }

        /// \brief Gets the up direction vector from the matrix (positive Y axis).
        ///
        /// \return The transformed Y axis.
        ZY_INLINE Vector3 GetUp() const
        {
            return GetBasisY();
        }

        /// \brief Gets the down direction vector from the matrix (negative Y axis).
        ///
        /// \return The negated transformed Y axis.
        ZY_INLINE Vector3 GetDown() const
        {
            return -GetBasisY();
        }

        /// \brief Gets the forward direction vector from the matrix (positive Z axis).
        ///
        /// \return The transformed Z axis.
        ZY_INLINE Vector3 GetForward() const
        {
            return GetBasisZ();
        }

        /// \brief Gets the backward direction vector from the matrix (negative Z axis).
        ///
        /// \return The negated transformed Z axis.
        ZY_INLINE Vector3 GetBackward() const
        {
            return -GetBasisZ();
        }

        /// \brief Gets the per-axis scale factors encoded in the matrix.
        ///
        /// \return A \ref Vector3 containing the scale of each axis (X, Y, Z).
        ZY_INLINE Vector3 GetScale() const
        {
            return Vector3(GetBasisX().GetLength(), GetBasisY().GetLength(), GetBasisZ().GetLength());
        }

        /// \brief Gets the translation component of the transformation matrix.
        ///
        /// \return The W component of every column.
        ZY_INLINE Vector3 GetTranslation() const
        {
            return Vector3(mColumns[0].GetW(), mColumns[1].GetW(), mColumns[2].GetW());
        }

        /// \brief Gets the rotation component of the transformation matrix as a quaternion.
        ///
        /// \return A \ref Quaternion representing the rotation encoded in the basis.
        ZY_INLINE Quaternion GetRotation() const
        {
            return Quaternion::FromDirection(GetBasisZ(), GetBasisY());
        }

        /// \brief Multiplies this matrix by another matrix.
        ///
        /// \param Other The right-hand matrix operand, applied before this one.
        /// \return A new matrix that is the product of the two matrices.
        ZY_INLINE Matrix4x3 operator*(ConstRef<Matrix4x3> Other) const
        {
            return Matrix4x3(
                Vector4::SplatX(mColumns[0]) * Other.mColumns[0]
              + Vector4::SplatY(mColumns[0]) * Other.mColumns[1]
              + Vector4::SplatZ(mColumns[0]) * Other.mColumns[2]
              + Vector4::SplatW(mColumns[0]) * Vector4::UnitW(),

                Vector4::SplatX(mColumns[1]) * Other.mColumns[0]
              + Vector4::SplatY(mColumns[1]) * Other.mColumns[1]
              + Vector4::SplatZ(mColumns[1]) * Other.mColumns[2]
              + Vector4::SplatW(mColumns[1]) * Vector4::UnitW(),

                Vector4::SplatX(mColumns[2]) * Other.mColumns[0]
              + Vector4::SplatY(mColumns[2]) * Other.mColumns[1]
              + Vector4::SplatZ(mColumns[2]) * Other.mColumns[2]
              + Vector4::SplatW(mColumns[2]) * Vector4::UnitW());
        }

        /// \brief Multiplies this matrix by another matrix in-place.
        ///
        /// \param Other The right-hand matrix operand, applied before this one.
        /// \return A reference to the updated matrix.
        ZY_INLINE Ref<Matrix4x3> operator*=(ConstRef<Matrix4x3> Other)
        {
            return (* this) = (* this) * Other;
        }

        /// \brief Checks if this matrix is equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if all corresponding elements are equal, `false` otherwise.
        ZY_INLINE Bool operator==(ConstRef<Matrix4x3> Other) const
        {
            return IsAlmostEqual(Other);
        }

        /// \brief Checks if this matrix is not equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if any corresponding element differs, `false` otherwise.
        ZY_INLINE Bool operator!=(ConstRef<Matrix4x3> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<7> kPattern("({0}, {1}, {2})");
            Format::Processor<Output>::Format(Buffer, kPattern, mColumns[0], mColumns[1], mColumns[2]);
        }

    public:

        /// \brief Gets the identity matrix.
        ///
        /// \return A matrix with 1 on the diagonal and 0 elsewhere.
        ZY_INLINE static Matrix4x3 Identity()
        {
            return Matrix4x3();
        }

        /// \brief Computes the determinant of the matrix's basis.
        ///
        /// \param Matrix The matrix to compute the determinant for.
        /// \return The scalar determinant value. A value of 0 indicates a singular basis.
        ZY_INLINE static Real32 GetDeterminant(ConstRef<Matrix4x3> Matrix)
        {
            return Vector3::Dot(Matrix.GetBasisX(), Vector3::Cross(Matrix.GetBasisY(), Matrix.GetBasisZ()));
        }

        /// \brief Projects a 2D point using the matrix, treating it as lying on the Z = 0 plane.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Vector The 2D point to project.
        /// \return The projected 2D point, translation included, with the Z result discarded.
        ZY_INLINE static Vector2 Project(ConstRef<Matrix4x3> Matrix, Vector2 Vector)
        {
            const Basis Columns(Matrix);

            return (Columns.AxisX * Vector4(Vector.GetX())
                  + Columns.AxisY * Vector4(Vector.GetY())
                  + Columns.Offset).GetXY();
        }

        /// \brief Projects a 3D point using the matrix.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Vector The 3D point to project.
        /// \return The projected 3D point, translation included.
        ZY_INLINE static Vector3 Project(ConstRef<Matrix4x3> Matrix, Vector3 Vector)
        {
            const Basis Columns(Matrix);

            return (Columns.AxisX * Vector4(Vector.GetX())
                  + Columns.AxisY * Vector4(Vector.GetY())
                  + Columns.AxisZ * Vector4(Vector.GetZ())
                  + Columns.Offset).GetXYZ();
        }

        /// \brief Projects many 2D points using the matrix, treating them as lying on the Z = 0 plane.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Source The points to project.
        /// \param Output Receives the projected points; may alias \p Source.
        ZY_INLINE static void Project(ConstRef<Matrix4x3> Matrix, ConstSpan<Vector2> Source, Span<Vector2> Output)
        {
            ZY_ASSERT(Output.GetSize() >= Source.GetSize(), "Projection output must have room for every point");

            const Basis Columns(Matrix);

            for (UInt Index = 0; Index < Source.GetSize(); ++Index)
            {
                Output[Index] = (Columns.AxisX * Vector4(Source[Index].GetX())
                               + Columns.AxisY * Vector4(Source[Index].GetY())
                               + Columns.Offset).GetXY();
            }
        }

        /// \brief Projects many 3D points using the matrix.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Source The points to project.
        /// \param Output Receives the projected points; may alias \p Source.
        ZY_INLINE static void Project(ConstRef<Matrix4x3> Matrix, ConstSpan<Vector3> Source, Span<Vector3> Output)
        {
            ZY_ASSERT(Output.GetSize() >= Source.GetSize(), "Projection output must have room for every point");

            const Basis Columns(Matrix);

            for (UInt Index = 0; Index < Source.GetSize(); ++Index)
            {
                Output[Index] = (Columns.AxisX * Vector4(Source[Index].GetX())
                               + Columns.AxisY * Vector4(Source[Index].GetY())
                               + Columns.AxisZ * Vector4(Source[Index].GetZ())
                               + Columns.Offset).GetXYZ();
            }
        }

        /// \brief Rotates and scales a 3D direction using the matrix, ignoring its translation.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Vector The 3D direction to transform.
        /// \return The transformed direction.
        ZY_INLINE static Vector3 ProjectDirection(ConstRef<Matrix4x3> Matrix, Vector3 Vector)
        {
            const Vector4 Direction(Vector.GetX(), Vector.GetY(), Vector.GetZ(), 0.0f);

            return Vector3(
                Vector4::Dot(Matrix.mColumns[0], Direction),
                Vector4::Dot(Matrix.mColumns[1], Direction),
                Vector4::Dot(Matrix.mColumns[2], Direction));
        }

        /// \brief Creates a right-handed look-at view matrix given a target position.
        ///
        /// \param Eye   The camera position in world space.
        /// \param Focus The target position the camera looks toward.
        /// \param Up    The world-space up direction (must be normalized and non-parallel to the view direction).
        /// \return A look-at view matrix.
        ZY_INLINE static Matrix4x3 CreateLook(Vector3 Eye, Vector3 Focus, Vector3 Up)
        {
            return CreateDirection(Eye, Vector3::Normalize(Eye - Focus), Up);
        }

        /// \brief Creates a right-handed view matrix from an explicit forward direction.
        ///
        /// A view matrix is rigid, so it belongs here rather than on \ref Matrix4x4: it is the basis
        /// transposed with the eye projected onto each axis, which is exactly one row per axis.
        ///
        /// \param Eye       The camera position in world space.
        /// \param Direction The normalized forward direction the camera is facing.
        /// \param Up        The world-space up direction (must be normalized and non-parallel to Direction).
        /// \return A view matrix oriented along the given direction.
        static Matrix4x3 CreateDirection(Vector3 Eye, Vector3 Direction, Vector3 Up);

        /// \brief Computes the inverse of the matrix.
        ///
        /// \param Matrix The matrix to invert. Its basis must be non-singular.
        /// \return The inverse of the given matrix.
        static Matrix4x3 Inverse(ConstRef<Matrix4x3> Matrix);

        /// \brief Performs a component-wise linear interpolation between two matrices.
        ///
        /// \param Start      The starting matrix (returned when \p Alpha is 0).
        /// \param End        The ending matrix (returned when \p Alpha is 1).
        /// \param Percentage The interpolation factor in the range [0, 1].
        /// \return The interpolated matrix.
        static Matrix4x3 Lerp(ConstRef<Matrix4x3> Start, ConstRef<Matrix4x3> End, Real32 Percentage);

        /// \brief Packs a matrix into the twelve loose floats a shader reads a `float4x3` from.
        ///
        /// \param Matrix The matrix to pack, whose stored columns are its rows.
        /// \return Each row of the basis followed by its translation, three times over.
        ZY_INLINE static Array<Real32, 12> Pack(ConstRef<Matrix4x3> Matrix)
        {
            ZY_ALIGN(16) Array<Real32, 12> Result;

            for (UInt32 Element = 0; Element < 3; ++Element)
            {
                Matrix.mColumns[Element].Store(Result.GetData() + Element * 4);
            }
            return Result;
        }

        /// \brief Creates a translation matrix.
        ///
        /// \param Vector The translation to apply along each axis.
        /// \return A matrix that translates by the given amount.
        ZY_INLINE static Matrix4x3 FromTranslation(Vector3 Vector)
        {
            return Matrix4x3(
                Vector4(1.0f, 0.0f, 0.0f, Vector.GetX()),
                Vector4(0.0f, 1.0f, 0.0f, Vector.GetY()),
                Vector4(0.0f, 0.0f, 1.0f, Vector.GetZ()));
        }

        /// \brief Creates a scaling matrix.
        ///
        /// \param Vector The scale factor to apply along each axis.
        /// \return A matrix that scales by the given factors.
        ZY_INLINE static Matrix4x3 FromScale(Vector3 Vector)
        {
            return Matrix4x3(
                Vector4(Vector.GetX(), 0.0f, 0.0f, 0.0f),
                Vector4(0.0f, Vector.GetY(), 0.0f, 0.0f),
                Vector4(0.0f, 0.0f, Vector.GetZ(), 0.0f));
        }

        /// \brief Creates a rotation matrix from a quaternion.
        ///
        /// \param Rotation The unit quaternion representing the desired rotation.
        /// \return A 4×3 rotation matrix equivalent to the quaternion.
        ZY_INLINE static Matrix4x3 FromRotation(Quaternion Rotation)
        {
            return FromTransform(Vector3(0.0f, 0.0f, 0.0f), Vector3(1.0f, 1.0f, 1.0f), Rotation);
        }

        /// \brief Creates a combined TRS (translation × rotation × scale) matrix.
        ///
        /// \param Translation The world-space position.
        /// \param Scale       The per-axis scale factors.
        /// \param Rotation    The rotation quaternion (should be normalized).
        /// \return The combined TRS transformation matrix.
        ZY_INLINE static Matrix4x3 FromTransform(Vector3 Translation, Vector3 Scale, Quaternion Rotation)
        {
            const Vector4 Components = Rotation;
            const Vector4 Doubled    = Components + Components;
            const Vector4 Squared    = Components * Doubled;

            const Vector4 Diagonal = Vector4::One()
                                   - Vector4::Swizzle<1, 0, 0, 3>(Squared)
                                   - Vector4::Swizzle<2, 2, 1, 3>(Squared);

            const Vector4 Symmetric     = Vector4::Swizzle<0, 1, 0, 3>(Components) * Vector4::Swizzle<1, 2, 2, 3>(Doubled);
            const Vector4 Antisymmetric = Vector4::SplatW(Components) * Vector4::Swizzle<2, 0, 1, 3>(Doubled);

            const Vector4 Sum        = Symmetric + Antisymmetric;
            const Vector4 Difference = Symmetric - Antisymmetric;

            const Vector4 RowX = Vector4::Select<0b0100>(
                Vector4::Select<0b0010>(Diagonal, Vector4::SplatX(Difference)), Sum);
            const Vector4 RowY = Vector4::Select<0b0100>(
                Vector4::Select<0b0010>(Sum, Diagonal), Vector4::SplatY(Difference));
            const Vector4 RowZ = Vector4::Select<0b0100>(
                Vector4::Select<0b0001>(Sum, Vector4::SplatZ(Difference)), Diagonal);

            const Vector4 Factor(Scale.GetX(), Scale.GetY(), Scale.GetZ(), 1.0f);
            const Vector4 Offset(Translation.GetX(), Translation.GetY(), Translation.GetZ(), 0.0f);

            return Matrix4x3(
                Vector4::Select<0b1000>(RowX, Vector4::SplatX(Offset)) * Factor,
                Vector4::Select<0b1000>(RowY, Vector4::SplatY(Offset)) * Factor,
                Vector4::Select<0b1000>(RowZ, Vector4::SplatZ(Offset)) * Factor);
        }

        /// \brief Creates a combined TRS matrix whose rotation and scale pivot around a point.
        ///
        /// \param Origin      The point, in local space, that scale and rotation are applied around.
        /// \param Translation The world-space position the origin ends up at.
        /// \param Scale       The per-axis scale factors.
        /// \param Rotation    The rotation quaternion (should be normalized).
        /// \return The combined TRS transformation matrix built around the given origin.
        ZY_INLINE static Matrix4x3 FromTransform(Vector3 Origin, Vector3 Translation, Vector3 Scale, Quaternion Rotation)
        {
            const Matrix4x3 Matrix = FromTransform(Translation, Scale, Rotation);

            return WithTranslation(Matrix, -ProjectDirection(Matrix, Origin));
        }

        /// \brief Applies a translation to an existing matrix, returning a new matrix with the translation applied.
        ///
        /// \param Matrix The original matrix to apply the translation to.
        /// \param Offset The 3D offset to apply as a translation to the matrix.
        /// \return The matrix with the translation applied.
        ZY_INLINE static Matrix4x3 WithTranslation(ConstRef<Matrix4x3> Matrix, Vector3 Offset)
        {
            return Matrix4x3(
                Matrix.mColumns[0] + Vector4(0.0f, 0.0f, 0.0f, Offset.GetX()),
                Matrix.mColumns[1] + Vector4(0.0f, 0.0f, 0.0f, Offset.GetY()),
                Matrix.mColumns[2] + Vector4(0.0f, 0.0f, 0.0f, Offset.GetZ()));
        }

        /// \brief Scales the basis of an existing matrix, leaving the translation where the transform put it.
        ///
        /// \param Matrix The original matrix to scale the basis of.
        /// \param Vector The per-axis factors the basis is scaled by.
        /// \return The matrix with its basis scaled.
        ZY_INLINE static Matrix4x3 WithScale(ConstRef<Matrix4x3> Matrix, Vector3 Vector)
        {
            const Vector4 Factor(Vector.GetX(), Vector.GetY(), Vector.GetZ(), 1.0f);

            return Matrix4x3(Matrix.mColumns[0] * Factor,
                             Matrix.mColumns[1] * Factor,
                             Matrix.mColumns[2] * Factor);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mColumns[3];
    };
}