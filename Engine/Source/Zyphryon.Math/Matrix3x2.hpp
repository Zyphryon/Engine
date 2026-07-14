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

#include "Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents an affine 2D transformation matrix (3 rows x 2 columns, column-major).
    class Matrix3x2 final
    {
    public:

        /// \brief Constructs an identity matrix.
        ZY_INLINE constexpr Matrix3x2()
            : mColumns { Vector3(1.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f) }
        {
        }

        /// \brief Constructs a matrix from six scalar values.
        ///
        /// \param M00 The X scale component of the first basis vector ([0,0]).
        /// \param M10 The Y scale component of the first basis vector ([1,0]).
        /// \param M20 The X translation component ([2,0]).
        /// \param M01 The X shear component of the second basis vector ([0,1]).
        /// \param M11 The Y scale component of the second basis vector ([1,1]).
        /// \param M21 The Y translation component ([2,1]).
        ZY_INLINE constexpr Matrix3x2(Real32 M00, Real32 M10, Real32 M20, Real32 M01, Real32 M11, Real32 M21)
            : mColumns { Vector3(M00, M10, M20), Vector3(M01, M11, M21) }
        {
        }

        /// \brief Checks if this matrix is approximately equal to another matrix within a tolerance.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostEqual(ConstRef<Matrix3x2> Other) const
        {
            return mColumns[0] == Other.mColumns[0] && mColumns[1] == Other.mColumns[1];
        }

        /// \brief Checks if the matrix is an identity matrix.
        ///
        /// \return `true` if the matrix is identity, `false` otherwise.
        ZY_INLINE constexpr Bool IsIdentity() const
        {
            return mColumns[0].IsAlmostEqual(1.0f, 0.0f, 0.0f) && mColumns[1].IsAlmostEqual(0.0f, 1.0f, 0.0f);
        }
    
        /// \brief Gets a column vector from the matrix.
        ///
        /// \param Column The zero-based index of the column to access.
        /// \return The column vector at the specified index.
        ZY_INLINE constexpr Ref<Vector3> GetColumn(UInt32 Column)
        {
            ZY_ASSERT(Column < 2, "GetColumn index out of range");

            return mColumns[Column];
        }

        /// \brief Gets a column vector from the matrix.
        ///
        /// \param Column The zero-based index of the column to access.
        /// \return The column vector at the specified index.
        ZY_INLINE constexpr Vector3 GetColumn(UInt32 Column) const
        {
            return mColumns[Column];
        }

        /// \brief Gets the X basis vector (first column).
        ///
        /// \return The X basis vector.
        ZY_INLINE constexpr Vector2 GetBasisX() const
        {
            return Vector2(mColumns[0].GetX(), mColumns[1].GetX());
        }

        /// \brief Gets the Y basis vector (second column).
        ///
        /// \return The Y basis vector.
        ZY_INLINE constexpr Vector2 GetBasisY() const
        {
            return Vector2(mColumns[0].GetY(), mColumns[1].GetY());
        }

        /// \brief Gets the translation vector (third column).
        ///
        /// \return The translation vector.
        ZY_INLINE constexpr Vector2 GetTranslation() const
        {
            return Vector2(mColumns[0].GetZ(), mColumns[1].GetZ());
        }

        /// \brief Gets the scale vector derived from the basis vectors lengths.
        ///
        /// \return The scale vector.
        ZY_INLINE Vector2 GetScale() const
        {
            const Real32 ScaleX = GetBasisX().GetLength();
            const Real32 ScaleY = GetBasisY().GetLength();
            return Vector2(ScaleX, ScaleY);
        }

        /// \brief Gets the rotation angle derived from the first basis vector.
        ///
        /// \return The rotation angle of the transformation.
        ZY_INLINE Angle GetRotation() const
        {
            return Angle::FromCartesian(mColumns[0].GetX(), mColumns[1].GetX());
        }

        /// \brief Multiplies this matrix by another matrix.
        ///
        /// \param Other The matrix to multiply by.
        /// \return A new matrix that is the product of the two matrices.
        ZY_INLINE constexpr Matrix3x2 operator*(ConstRef<Matrix3x2> Other) const
        {
            const Vector3 A0 = mColumns[0];
            const Vector3 A1 = mColumns[1];
            const Vector3 B0 = Other.mColumns[0];
            const Vector3 B1 = Other.mColumns[1];

            const Real32 M00 = A0.GetX() * B0.GetX() + A1.GetX() * B0.GetY();
            const Real32 M10 = A0.GetY() * B0.GetX() + A1.GetY() * B0.GetY();
            const Real32 M20 = A0.GetZ() + A0.GetX() * B0.GetZ() + A0.GetY() * B1.GetZ();
            const Real32 M01 = A0.GetX() * B1.GetX() + A1.GetX() * B1.GetY();
            const Real32 M11 = A0.GetY() * B1.GetX() + A1.GetY() * B1.GetY();
            const Real32 M21 = A1.GetZ() + A1.GetX() * B0.GetZ() + A1.GetY() * B1.GetZ();

            return Matrix3x2(M00, M10, M20, M01, M11, M21);
        }

        /// \brief Multiplies this matrix by another matrix (in-place).
        ///
        /// \param Other The matrix to multiply by.
        /// \return A reference to the updated matrix.
        ZY_INLINE constexpr Ref<Matrix3x2> operator*=(ConstRef<Matrix3x2> Other)
        {
            const Vector3 A0 = mColumns[0];
            const Vector3 A1 = mColumns[1];
            const Vector3 B0 = Other.mColumns[0];
            const Vector3 B1 = Other.mColumns[1];

            const Real32 M00 = A0.GetX() * B0.GetX() + A1.GetX() * B0.GetY();
            const Real32 M10 = A0.GetY() * B0.GetX() + A1.GetY() * B0.GetY();
            const Real32 M20 = A0.GetZ() + A0.GetX() * B0.GetZ() + A0.GetY() * B1.GetZ();
            const Real32 M01 = A0.GetX() * B1.GetX() + A1.GetX() * B1.GetY();
            const Real32 M11 = A0.GetY() * B1.GetX() + A1.GetY() * B1.GetY();
            const Real32 M21 = A1.GetZ() + A1.GetX() * B0.GetZ() + A1.GetY() * B1.GetZ();

            mColumns[0].Set(M00, M10, M20);
            mColumns[1].Set(M01, M11, M21);

            return (* this);
        }

        /// \brief Checks if this matrix is equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Matrix3x2> Other) const
        {
            return IsAlmostEqual(Other);
        }

        /// \brief Checks if this matrix is not equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(ConstRef<Matrix3x2> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Gets a string representation of this object.
        ///
        /// \param Buffer The string buffer to write the output to.
        template<typename Output>
        ZY_INLINE constexpr void OnFormat(Ref<Output> Buffer) const
        {
            static constexpr Format::Pattern<5> kPattern("({0}, {1})");
            Format::Processor<Output>::Format(Buffer, kPattern, mColumns[0], mColumns[1]);
        }

    public:

        /// \brief Gets the identity matrix.
        ///
        /// \return The identity matrix.
        ZY_INLINE static constexpr Matrix3x2 Identity()
        {
            return Matrix3x2();
        }

        /// \brief Computes the determinant of the linear transformation part of the matrix.
        ///
        /// \param Matrix The matrix to compute the determinant of.
        /// \return The determinant of the 2x2 linear transformation part (basis vectors).
        ZY_INLINE static constexpr Real32 GetDeterminant(ConstRef<Matrix3x2> Matrix)
        {
            const Real32 M00 = Matrix.mColumns[0].GetX();
            const Real32 M10 = Matrix.mColumns[0].GetY();
            const Real32 M01 = Matrix.mColumns[1].GetX();
            const Real32 M11 = Matrix.mColumns[1].GetY();

            return (M00 * M11) - (M10 * M01);
        }

        /// \brief Computes the inverse of an affine transformation matrix.
        ///
        /// \param Matrix The matrix to invert.
        /// \return The inverse of the given matrix.
        ZY_INLINE static constexpr Matrix3x2 Inverse(ConstRef<Matrix3x2> Matrix)
        {
            const Real32 Det = GetDeterminant(Matrix);
            ZY_ASSERT(!::IsAlmostZero(Det), "Cannot invert a singular matrix");

            const Real32 InvDet = 1.0f / Det;
            const Real32 M00    = Matrix.mColumns[0].GetX();
            const Real32 M10    = Matrix.mColumns[0].GetY();
            const Real32 M20    = Matrix.mColumns[0].GetZ();
            const Real32 M01    = Matrix.mColumns[1].GetX();
            const Real32 M11    = Matrix.mColumns[1].GetY();
            const Real32 M21    = Matrix.mColumns[1].GetZ();

            const Real32 IM00 =  M11 * InvDet;
            const Real32 IM10 = -M10 * InvDet;
            const Real32 IM01 = -M01 * InvDet;
            const Real32 IM11 =  M00 * InvDet;
            const Real32 IM20 = (M10 * M21 - M11 * M20) * InvDet;
            const Real32 IM21 = (M01 * M20 - M00 * M21) * InvDet;

            return Matrix3x2(IM00, IM10, IM20, IM01, IM11, IM21);
        }

        /// \brief Projects a 2D vector using an affine matrix.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Vector The 2D vector to project.
        /// \return The transformed 2D vector.
        ZY_INLINE static constexpr Vector2 Project(ConstRef<Matrix3x2> Matrix, Vector2 Vector)
        {
            const Vector3 C0 = Matrix.mColumns[0];
            const Vector3 C1 = Matrix.mColumns[1];

            return Vector2(C0.GetX() * Vector.GetX() + C0.GetY() * Vector.GetY() + C0.GetZ(),
                           C1.GetX() * Vector.GetX() + C1.GetY() * Vector.GetY() + C1.GetZ());
        }

        /// \brief Creates a translation matrix.
        ///
        /// \param Vector The translation vector.
        /// \return The translation matrix.
        ZY_INLINE static constexpr Matrix3x2 FromTranslation(Vector2 Vector)
        {
            return Matrix3x2(1.0f, 0.0f, Vector.GetX(), 0.0f, 1.0f, Vector.GetY());
        }

        /// \brief Creates a uniform scaling matrix.
        ///
        /// \param Vector The scale vector.
        /// \return The scaling matrix.
        ZY_INLINE static constexpr Matrix3x2 FromScale(Vector2 Vector)
        {
            return Matrix3x2(Vector.GetX(), 0.0f, 0.0f, 0.0f, Vector.GetY(), 0.0f);
        }

        /// \brief Creates a rotation matrix from an angle.
        ///
        /// \param Rotation The rotation angle.
        /// \return The rotation transformation matrix.
        ZY_INLINE static Matrix3x2 FromRotation(Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            return Matrix3x2(C, -S, 0.0f, S,  C, 0.0f);
        }

        /// \brief Creates a horizontal flip matrix (mirror along Y axis).
        ///
        /// \return The horizontal flip matrix.
        ZY_INLINE static constexpr Matrix3x2 FromFlipX()
        {
            return Matrix3x2(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        }

        /// \brief Creates a vertical flip matrix (mirror along X axis).
        ///
        /// \return The vertical flip matrix.
        ZY_INLINE static constexpr Matrix3x2 FromFlipY()
        {
            return Matrix3x2(1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        }

        /// \brief Creates a skew matrix from angles.
        ///
        /// \param X The skew angle along the X axis.
        /// \param Y The skew angle along the Y axis.
        /// \return The skew matrix.
        ZY_INLINE static constexpr Matrix3x2 FromSkew(Angle X, Angle Y)
        {
            return Matrix3x2(1.0f, Angle::Tangent(X), 0.0f, Angle::Tangent(Y), 1.0f, 0.0f);
        }

        /// \brief Creates a skew matrix along the X axis.
        ///
        /// \param X The skew angle along the X axis.
        /// \return The skew matrix.
        ZY_INLINE static constexpr Matrix3x2 FromSkewX(Angle X)
        {
            return FromSkew(X, Angle());
        }

        /// \brief Creates a skew matrix along the Y axis.
        ///
        /// \param Y The skew angle along the Y axis.
        /// \return The skew matrix.
        ZY_INLINE static constexpr Matrix3x2 FromSkewY(Angle Y)
        {
            return FromSkew(Angle(), Y);
        }

        /// \brief Creates a transformation matrix from translation, scale, rotation, and origin components.
        ///
        /// \param Origin      The center point around which scale, rotation, and skew are applied.
        /// \param Translation The final position offset applied after all transformations.
        /// \param Scale       The scaling factors along the X and Y axes.
        /// \param Rotation    The rotation angle (in radians or degrees based on Angle type).
        ///
        /// \return The transformation matrix combining scale, rotation, and translation around the given origin.
        ZY_INLINE static Matrix3x2 FromTransform(Vector2 Origin, Vector2 Translation, Vector2 Scale, Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            const Real32 M00 =  C * Scale.GetX();
            const Real32 M10 = -S * Scale.GetY();
            const Real32 M20 = Translation.GetX() - (Origin.GetX() * M00 + Origin.GetY() * M10);
            const Real32 M01 =  S * Scale.GetX();
            const Real32 M11 =  C * Scale.GetY();
            const Real32 M21 = Translation.GetY() - (Origin.GetX() * M01 + Origin.GetY() * M11);

            return Matrix3x2(M00, M10, M20, M01, M11, M21);
        }

        /// \brief Creates a transformation matrix from translation, scale, rotation, skew, and origin components.
        ///
        /// \param Origin      The center point around which scale, rotation, and skew are applied.
        /// \param Translation The final position offset applied after all transformations.
        /// \param Scale       The scaling factors along the X and Y axes.
        /// \param Rotation    The rotation angle (in radians or degrees based on Angle type).
        /// \param Skew        The shear angles: X skew shears horizontally, Y skew shears vertically.
        ///
        /// \return The transformation matrix combining scale, rotation, skew, and translation around the given origin.
        ZY_INLINE static Matrix3x2 FromTransform(Vector2 Origin, Vector2 Translation, Vector2 Scale, Angle Rotation, Vector2 Skew)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);
            const Real32 U = Angle::Tangent(Skew.GetX());
            const Real32 T = Angle::Tangent(Skew.GetY());

            const Real32 M00 =  C * Scale.GetX() + U * S * Scale.GetY();
            const Real32 M10 = -S * Scale.GetY() + U * C * Scale.GetY();
            const Real32 M20 = Translation.GetX() - (Origin.GetX() * M00 + Origin.GetY() * M10);
            const Real32 M01 =  S * Scale.GetX() + T * C * Scale.GetY();
            const Real32 M11 =  C * Scale.GetY() + T * S * Scale.GetY();
            const Real32 M21 = Translation.GetY() - (Origin.GetX() * M01 + Origin.GetY() * M11);

            return Matrix3x2(M00, M10, M20, M01, M11, M21);
        }

        /// \brief Applies a translation to an existing matrix, returning a new matrix with the translation applied.
        ///
        /// \param Matrix The original matrix to apply the translation to.
        /// \param Offset The 2D offset to apply as a translation to the matrix.
        /// \return The matrix with the translation applied.
        ZY_INLINE static constexpr Matrix3x2 WithTranslation(ConstRef<Matrix3x2> Matrix, Vector2 Offset)
        {
            const Vector3 Column0 = Matrix.GetColumn(0);
            const Vector3 Column1 = Matrix.GetColumn(1);

            return Matrix3x2(Column0.GetX(), Column0.GetY(), Column0.GetZ() + Offset.GetX(),
                             Column1.GetX(), Column1.GetY(), Column1.GetZ() + Offset.GetY());
        }

        /// \brief Applies a scaling transformation to an existing matrix, returning a new matrix with the scale applied.
        ///
        /// \param Matrix The original matrix to apply the scaling to.
        /// \param Factor The scaling factors along the X and Y axes to apply to the matrix.
        /// \return The matrix with the scaling applied.
        ZY_INLINE static constexpr Matrix3x2 WithScale(ConstRef<Matrix3x2> Matrix, Vector2 Factor)
        {
            const Vector3 Column0 = Matrix.GetColumn(0);
            const Vector3 Column1 = Matrix.GetColumn(1);

            return Matrix3x2(Column0.GetX() * Factor.GetX(), Column0.GetY() * Factor.GetX(), Column0.GetZ(),
                             Column1.GetX() * Factor.GetY(), Column1.GetY() * Factor.GetY(), Column1.GetZ());
        }

        /// \brief Applies an additional rotation to an existing matrix.
        ///
        /// \param Matrix   The original matrix to apply the rotation to.
        /// \param Rotation The rotation angle to apply.
        /// \return The matrix with the rotation applied.
        ZY_INLINE static constexpr Matrix3x2 WithRotation(ConstRef<Matrix3x2> Matrix, Angle Rotation)
        {
            return FromRotation(Rotation) * Matrix;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3 mColumns[2];
    };
}
