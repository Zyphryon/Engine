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

#include "Pivot.hpp"
#include "Vector2.hpp"
#include "Vector4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a 3x2 affine transformation matrix for 2D operations.
    class ZYPHRYON_ALIGN(16) Matrix3x2 final
    {
    public:

        /// \brief Constructs an identity matrix.
        ZYPHRYON_INLINE Matrix3x2()
            : mColumns { Vector4(1.0f, 0.0f, 0.0f, 0.0f),
                         Vector4(0.0f, 1.0f, 0.0f, 0.0f) }
        {
        }

        /// \brief Constructs a matrix from six scalar values.
        ///
        /// \param M00 Element at column 0, row 0 (X scale).
        /// \param M10 Element at column 1, row 0 (X shear).
        /// \param M20 Element at column 2, row 0 (X translation).
        /// \param M01 Element at column 0, row 1 (Y shear).
        /// \param M11 Element at column 1, row 1 (Y scale).
        /// \param M21 Element at column 2, row 1 (Y translation).
        ZYPHRYON_INLINE Matrix3x2(Real32 M00, Real32 M10, Real32 M20, Real32 M01, Real32 M11, Real32 M21)
            : mColumns { Vector4(M00, M10, M20, 0.0f),
                         Vector4(M01, M11, M21, 0.0f) }
        {
        }

        /// \brief Checks if the matrix is an identity matrix.
        ///
        /// \return `true` if the matrix is identity, `false` otherwise.
        ZYPHRYON_INLINE Bool IsIdentity() const
        {
            return mColumns[0].IsAlmostEqual(1.0f, 0.0f, 0.0f, 0.0f) && mColumns[1].IsAlmostEqual(0.0f, 1.0f, 0.0f, 0.0f);
        }
    
        /// \brief Retrieves a column vector from the matrix.
        ///
        /// \param Column Zero-based index of the column to access.
        /// \return Reference to the column vector at the specified index.
        ZYPHRYON_INLINE Ref<Vector4> GetColumn(UInt32 Column)
        {
            LOG_ASSERT(Column < 2, "GetColumn index out of range");

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

        /// \brief Gets the X basis vector (first column).
        ///
        /// \return The X basis vector.
        ZYPHRYON_INLINE Vector2 GetBasisX() const
        {
            return Vector2(mColumns[0].GetX(), mColumns[1].GetX());
        }

        /// \brief Gets the Y basis vector (second column).
        ///
        /// \return The Y basis vector.
        ZYPHRYON_INLINE Vector2 GetBasisY() const
        {
            return Vector2(mColumns[0].GetY(), mColumns[1].GetY());
        }

        /// \brief Gets the translation vector (third column).
        ///
        /// \return The translation vector.
        ZYPHRYON_INLINE Vector2 GetTranslation() const
        {
            return Vector2(mColumns[0].GetZ(), mColumns[1].GetZ());
        }

        /// \brief Gets the scale vector derived from the basis vectors lengths.
        ///
        /// \return The scale vector.
        ZYPHRYON_INLINE Vector2 GetScale() const
        {
            const Real32 ScaleX = GetBasisX().GetLength();
            const Real32 ScaleY = GetBasisY().GetLength();
            return Vector2(ScaleX, ScaleY);
        }

        /// \brief Multiplies this matrix by another matrix.
        ///
        /// \param Other The matrix to multiply by.
        /// \return A new matrix that is the product of the two matrices.
        ZYPHRYON_INLINE Matrix3x2 operator*(ConstRef<Matrix3x2> Other) const
        {
            const Vector4 R0 = Vector4::SplatX(mColumns[0]) * Other.mColumns[0] + Vector4::SplatY(mColumns[0]) * Other.mColumns[1];
            const Vector4 R1 = Vector4::SplatX(mColumns[1]) * Other.mColumns[0] + Vector4::SplatY(mColumns[1]) * Other.mColumns[1];

            return Matrix3x2(
                R0 + Vector4::Blend<0b0100>(Vector4(), mColumns[0]),
                R1 + Vector4::Blend<0b0100>(Vector4(), mColumns[1]));
        }

        /// \brief Multiplies this matrix by another matrix (in-place).
        ///
        /// \param Other The matrix to multiply by.
        /// \return A reference to the updated matrix.
        ZYPHRYON_INLINE Ref<Matrix3x2> operator*=(ConstRef<Matrix3x2> Other)
        {
            const Vector4 R0 = Vector4::SplatX(mColumns[0]) * Other.mColumns[0] + Vector4::SplatY(mColumns[0]) * Other.mColumns[1];
            const Vector4 R1 = Vector4::SplatX(mColumns[1]) * Other.mColumns[0] + Vector4::SplatY(mColumns[1]) * Other.mColumns[1];

            mColumns[0] = R0 + Vector4::Blend<0b0100>(Vector4(), mColumns[0]);
            mColumns[1] = R1 + Vector4::Blend<0b0100>(Vector4(), mColumns[1]);

            return (*this);
        }

        /// \brief Checks if this matrix is equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator==(ConstRef<Matrix3x2> Other) const
        {
            return mColumns[0] == Other.mColumns[0] && mColumns[1] == Other.mColumns[1];
        }

        /// \brief Checks if this matrix is not equal to another matrix.
        ///
        /// \param Other The matrix to compare to.
        /// \return `true` if the matrices are not equal, `false` otherwise.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Matrix3x2> Other) const
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
        }

    public:

        /// \brief Returns the identity matrix.
        ///
        /// \return The identity matrix.
        ZYPHRYON_INLINE static Matrix3x2 Identity()
        {
            return Matrix3x2();
        }

        /// \brief Projects a 2D vector using an affine matrix.
        ///
        /// \param Matrix The transformation matrix to apply.
        /// \param Vector The 2D vector to project.
        /// \return The transformed 2D vector.
        ZYPHRYON_INLINE static Vector2 Project(ConstRef<Matrix3x2> Matrix, Vector2 Vector)
        {
            const Vector4 Result =
                Matrix.mColumns[0] * Vector4(Vector.GetX()) +
                Matrix.mColumns[1] * Vector4(Vector.GetY()) +
                Vector4(Matrix.mColumns[0].GetZ(), Matrix.mColumns[1].GetZ(), 0.0f, 0.0f);

            return Result.GetXY();
        }

        /// \brief Creates a translation matrix.
        ///
        /// \param Vector The translation vector.
        /// \return A translation matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromTranslation(Vector2 Vector)
        {
            return Matrix3x2(1.0f, 0.0f, Vector.GetX(), 0.0f, 1.0f, Vector.GetY());
        }

        /// \brief Creates a uniform scaling matrix.
        ///
        /// \param Vector The scale vector.
        /// \return A scaling matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromScale(Vector2 Vector)
        {
            return Matrix3x2(Vector.GetX(), 0.0f, 0.0f, 0.0f, Vector.GetY(), 0.0f);
        }

        /// \brief Creates a rotation matrix from an angle.
        ///
        /// \param Rotation The rotation angle.
        /// \return A rotation matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromRotation(Angle Rotation)
        {
            const Real32 C = Angle::Cosine(Rotation);
            const Real32 S = Angle::Sine(Rotation);

            return Matrix3x2(C, -S, 0.0f, S,  C, 0.0f);
        }

        /// \brief Creates a horizontal flip matrix (mirror along Y axis).
        ///
        /// \return A horizontal flip matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromFlipX()
        {
            return Matrix3x2(-1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        }

        /// \brief Creates a vertical flip matrix (mirror along X axis).
        ///
        /// \return A vertical flip matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromFlipY()
        {
            return Matrix3x2(1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        }

        /// \brief Creates a skew matrix from angles.
        ///
        /// \param X The skew angle along the X axis.
        /// \param Y The skew angle along the Y axis.
        /// \return A skew matrix.
        ZYPHRYON_INLINE static Matrix3x2 FromSkew(Angle X, Angle Y)
        {
            return Matrix3x2(1.0f, Angle::Tangent(X), 0.0f, Angle::Tangent(Y), 1.0f, 0.0f);
        }

        /// \brief Creates a combined TRS matrix.
        ///
        /// \param Translation The translation vector.
        /// \param Scale       The scale vector.
        /// \param Rotation    The rotation angle.
        /// \return Combined TRS transformation matrix with pivot.
        ZYPHRYON_INLINE static Matrix3x2 FromTransform(Vector2 Translation, Vector2 Scale, Angle Rotation)
        {
            const Real32 C  = Angle::Cosine(Rotation);
            const Real32 S  = Angle::Sine(Rotation);
            const Real32 SX = Scale.GetX();
            const Real32 SY = Scale.GetY();

            return Matrix3x2(C * SX,  S * SX, Translation.GetX(), -S * SY,  C * SY, Translation.GetY());
        }

        /// \brief Creates a combined TRS matrix with an origin point.
        ///
        /// \param Translation The translation vector.
        /// \param Scale       The scale vector.
        /// \param Rotation    The rotation angle.
        /// \param Origin      The origin point for rotation and scale.
        /// \return Combined TRS transformation matrix with pivot.
        ZYPHRYON_INLINE static Matrix3x2 FromTransform(Vector2 Translation, Vector2 Scale, Angle Rotation, Pivot Origin)
        {
            const Real32 C  = Angle::Cosine(Rotation);
            const Real32 S  = Angle::Sine(Rotation);
            const Real32 SX = Scale.GetX();
            const Real32 SY = Scale.GetY();
            const Real32 PX = Origin.GetX();
            const Real32 PY = Origin.GetY();

            return Matrix3x2(
                C * SX, -S * SY, Translation.GetX() + PX - C * SX * PX + S * SY * PY,
                S * SX,  C * SY, Translation.GetY() + PY - S * SX * PX - C * SY * PY);
        }

    private:

        /// \brief Constructs a matrix directly from two Vector4 rows (internal use).
        ///
        /// \param Row0 The first row vector.
        /// \param Row1 The second row vector.
        ZYPHRYON_INLINE Matrix3x2(Vector4 Row0, Vector4 Row1)
            : mColumns { Row0, Row1 }
        {
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector4 mColumns[2];
    };
}
