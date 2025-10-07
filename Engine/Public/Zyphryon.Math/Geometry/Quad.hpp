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

#include "Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a generic 2D quadrilateral defined by four corner points.
    class Quad final
    {
    public:

        /// \brief Constructor initializing all corners to zero.
        ZYPHRYON_INLINE constexpr Quad() = default;

        /// \brief Constructor initializing the quadrilateral with specified values.
        ///
        /// \param C0 The first corner point.
        /// \param C1 The second corner point.
        /// \param C2 The third corner point.
        /// \param C3 The fourth corner point.
        ZYPHRYON_INLINE constexpr Quad(ConstRef<Vector2> C0, ConstRef<Vector2> C1,
                                       ConstRef<Vector2> C2, ConstRef<Vector2> C3)
            : mCorners { C0, C1, C2, C3 }
        {
        }

        /// \brief Checks if the quadrilateral is zero.
        ///
        /// \return `true` if all coordinates are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return mCorners[0].IsAlmostZero() &&
                   mCorners[1].IsAlmostZero() &&
                   mCorners[2].IsAlmostZero() &&
                   mCorners[3].IsAlmostZero();
        }

        /// \brief Sets all four corner coordinates of the quadrilateral.
        ///
        /// \param C0 The first corner point.
        /// \param C1 The second corner point.
        /// \param C2 The third corner point.
        /// \param C3 The fourth corner point.
        ZYPHRYON_INLINE constexpr void Set(ConstRef<Vector2> C0, ConstRef<Vector2> C1, ConstRef<Vector2> C2, ConstRef<Vector2> C3)
        {
            mCorners[0] = C0;
            mCorners[1] = C1;
            mCorners[2] = C2;
            mCorners[3] = C3;
        }

        /// \brief Sets a specific corner of the quadrilateral.
        ///
        /// \param Corner The index of the corner to set (0-3).
        /// \param Point  The new coordinate value for the specified corner.
        ZYPHRYON_INLINE constexpr void SetCorner(UInt32 Corner, ConstRef<Vector2> Point)
        {
            LOG_ASSERT(Corner < 4, "Corner index must be in range [0, 3]");
            mCorners[Corner] = Point;
        }

        /// \brief Gets a specific corner of the quadrilateral.
        ///
        /// \param Corner The index of the corner to retrieve (0-3).
        /// \return A constant reference to the Vector2 coordinate of the specified corner.
        ZYPHRYON_INLINE constexpr ConstRef<Vector2> GetCorner(UInt32 Corner) const
        {
            LOG_ASSERT(Corner < 4, "Corner index must be in range [0, 3]");
            return mCorners[Corner];
        }

        /// \brief Calculates the axis-aligned bounding rectangle that contains the quadrilateral.
        ///
        /// \return A Rect representing the bounding box that contains all corners of the quadrilateral.
        ZYPHRYON_INLINE Rect GetBoundaries() const
        {
            const Real32 MinX = Base::Min(mCorners[0].GetX(), mCorners[1].GetX(), mCorners[2].GetX(), mCorners[3].GetX());
            const Real32 MaxX = Base::Max(mCorners[0].GetX(), mCorners[1].GetX(), mCorners[2].GetX(), mCorners[3].GetX());
            const Real32 MinY = Base::Min(mCorners[0].GetY(), mCorners[1].GetY(), mCorners[2].GetY(), mCorners[3].GetY());
            const Real32 MaxY = Base::Max(mCorners[0].GetY(), mCorners[1].GetY(), mCorners[2].GetY(), mCorners[3].GetY());
            return Rect(MinX, MinY, MaxX, MaxY);
        }

        /// \brief Calculates the center point of the quadrilateral.
        ///
        /// \return The center point of the quadrilateral.
        ZYPHRYON_INLINE constexpr Vector2 GetCenter() const
        {
            return (mCorners[0] + mCorners[1] + mCorners[2] + mCorners[3]) * 0.25f;
        }

        /// \brief Calculates the area of the quadrilateral.
        ///
        /// \return The area of the quadrilateral.
        ZYPHRYON_INLINE constexpr Real32 GetArea() const
        {
            const Real32 Sum1 = mCorners[0].GetX() * mCorners[1].GetY() +
                                mCorners[1].GetX() * mCorners[2].GetY() +
                                mCorners[2].GetX() * mCorners[3].GetY() +
                                mCorners[3].GetX() * mCorners[0].GetY();

            const Real32 Sum2 = mCorners[0].GetY() * mCorners[1].GetX() +
                                mCorners[1].GetY() * mCorners[2].GetX() +
                                mCorners[2].GetY() * mCorners[3].GetX() +
                                mCorners[3].GetY() * mCorners[0].GetX();

            return Abs((Sum1 - Sum2) * 0.5f);
        }

        /// \brief Gets the perimeter of the quadrilateral.
        ///
        /// \return The perimeter of the quadrilateral.
        ZYPHRYON_INLINE constexpr Real32 GetPerimeter() const
        {
            return (mCorners[1] - mCorners[0]).GetLength() +
                   (mCorners[2] - mCorners[1]).GetLength() +
                   (mCorners[3] - mCorners[2]).GetLength() +
                   (mCorners[0] - mCorners[3]).GetLength();
        }

        /// \brief Checks if this quadrilateral contains a point.
        ///
        /// \param X The X coordinate of the point.
        /// \param Y The Y coordinate of the point.
        /// \return `true` if the point is inside the quadrilateral, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(Real32 X, Real32 Y) const
        {
            return Contains(Vector2(X, Y));
        }

        /// \brief Checks if this quadrilateral contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point is inside the quadrilateral, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(ConstRef<Vector2> Point) const
        {
            const Real32 C0 = Vector2::Cross(mCorners[1] - mCorners[0], Point - mCorners[0]);
            const Real32 C1 = Vector2::Cross(mCorners[2] - mCorners[1], Point - mCorners[1]);
            const Real32 C2 = Vector2::Cross(mCorners[3] - mCorners[2], Point - mCorners[2]);
            const Real32 C3 = Vector2::Cross(mCorners[0] - mCorners[3], Point - mCorners[3]);
            
            return (C0 >= 0 && C1 >= 0 && C2 >= 0 && C3 >= 0) || 
                   (C0 <= 0 && C1 <= 0 && C2 <= 0 && C3 <= 0);
        }

        /// \brief Checks if this quadrilateral intersects with another quadrilateral.
        ///
        /// \param Other The other quadrilateral to check.
        /// \return `true` if the quads intersect, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Intersects(ConstRef<Quad> Other) const
        {
            return GetBoundaries().Intersects(Other.GetBoundaries());   // TODO: More precise intersection test
        }

        /// \brief Checks if this quadrilateral is equal to another quadrilateral.
        ///
        /// \param Other The quadrilateral to compare to.
        /// \return `true` if all coordinates are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<Quad> Other) const
        {
            return mCorners[0] == Other.mCorners[0] &&
                   mCorners[1] == Other.mCorners[1] &&
                   mCorners[2] == Other.mCorners[2] &&
                   mCorners[3] == Other.mCorners[3];
        }

        /// \brief Checks if this quadrilateral is not equal to another quadrilateral.
        ///
        /// \param Other The quadrilateral to compare to.
        /// \return `true` if the quads are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<Quad> Other) const
        {
            return !(*this == Other);
        }

        /// \brief Adds another quadrilateral to this quadrilateral.
        ///
        /// \param Other The quadrilateral to add.
        /// \return A new quadrilateral that is the sum of the two quads.
        ZYPHRYON_INLINE constexpr Quad operator+(ConstRef<Quad> Other) const
        {
            return Quad(mCorners[0] + Other.mCorners[0],
                        mCorners[1] + Other.mCorners[1],
                        mCorners[2] + Other.mCorners[2],
                        mCorners[3] + Other.mCorners[3]);
        }

        /// \brief Adds a scalar to all coordinates of this quadrilateral.
        ///
        /// \param Scalar The scalar to add.
        /// \return A new quadrilateral with the scalar added to all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator+(Real32 Scalar) const
        {
            return Quad(mCorners[0] + Scalar,
                        mCorners[1] + Scalar, 
                        mCorners[2] + Scalar, 
                        mCorners[3] + Scalar);
        }

        /// \brief Adds a vector to all coordinates of this quadrilateral.
        ///
        /// \param Vector The vector to add.
        /// \return A new quadrilateral with the vector added to all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator+(ConstRef<Vector2> Vector) const
        {
            return Quad(mCorners[0] + Vector,
                        mCorners[1] + Vector, 
                        mCorners[2] + Vector, 
                        mCorners[3] + Vector);
        }

        /// \brief Subtracts another quadrilateral from this quadrilateral.
        ///
        /// \param Other The quadrilateral to subtract.
        /// \return A new quadrilateral that is the difference of the two quads.
        ZYPHRYON_INLINE constexpr Quad operator-(ConstRef<Quad> Other) const
        {
            return Quad(mCorners[0] - Other.mCorners[0],
                        mCorners[1] - Other.mCorners[1],
                        mCorners[2] - Other.mCorners[2],
                        mCorners[3] - Other.mCorners[3]);
        }

        /// \brief Subtracts a scalar from all coordinates of this quadrilateral.
        ///
        /// \param Scalar The scalar to subtract.
        /// \return A new quadrilateral with the scalar subtracted from all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator-(Real32 Scalar) const
        {
            return Quad(mCorners[0] - Scalar,
                        mCorners[1] - Scalar, 
                        mCorners[2] - Scalar, 
                        mCorners[3] - Scalar);
        }

        /// \brief Subtracts a vector from all coordinates of this quadrilateral.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new quadrilateral with the vector subtracted from all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator-(ConstRef<Vector2> Vector) const
        {
            return Quad(mCorners[0] - Vector,
                        mCorners[1] - Vector, 
                        mCorners[2] - Vector, 
                        mCorners[3] - Vector);
        }

        /// \brief Multiplies all coordinates of this quadrilateral by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new quadrilateral with the scalar multiplied by all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator*(Real32 Scalar) const
        {
            return Quad(mCorners[0] * Scalar,
                        mCorners[1] * Scalar, 
                        mCorners[2] * Scalar, 
                        mCorners[3] * Scalar);
        }

        /// \brief Multiplies all coordinates of this quadrilateral by a vector.
        ///
        /// \param Vector The vector to multiply by.
        /// \return A new quadrilateral with the vector multiplied by all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator*(ConstRef<Vector2> Vector) const
        {
            return Quad(mCorners[0] * Vector,
                        mCorners[1] * Vector, 
                        mCorners[2] * Vector, 
                        mCorners[3] * Vector);
        }

        /// \brief Divides all coordinates of this quadrilateral by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new quadrilateral with the scalar divided by all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator/(Real32 Scalar) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");
            return Quad(mCorners[0] / Scalar,
                        mCorners[1] / Scalar, 
                        mCorners[2] / Scalar, 
                        mCorners[3] / Scalar);
        }

        /// \brief Divides all coordinates of this quadrilateral by a vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A new quadrilateral with the vector divided by all coordinates.
        ZYPHRYON_INLINE constexpr Quad operator/(ConstRef<Vector2> Vector) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");
            return Quad(mCorners[0] / Vector,
                        mCorners[1] / Vector, 
                        mCorners[2] / Vector, 
                        mCorners[3] / Vector);
        }

        /// \brief Adds another quadrilateral to the current quadrilateral.
        ///
        /// \param Other The quadrilateral to add.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator+=(ConstRef<Quad> Other)
        {
            mCorners[0] += Other.mCorners[0];
            mCorners[1] += Other.mCorners[1];
            mCorners[2] += Other.mCorners[2];
            mCorners[3] += Other.mCorners[3];
            return (* this);
        }

        /// \brief Adds a scalar value to the quadrilateral.
        ///
        /// \param Scalar The scalar value to add.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator+=(Real32 Scalar)
        {
            mCorners[0] += Scalar;
            mCorners[1] += Scalar;
            mCorners[2] += Scalar;
            mCorners[3] += Scalar;
            return (* this);
        }

        /// \brief Adds a vector to the quadrilateral.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator+=(ConstRef<Vector2> Vector)
        {
            mCorners[0] += Vector;
            mCorners[1] += Vector;
            mCorners[2] += Vector;
            mCorners[3] += Vector;
            return (* this);
        }

        /// \brief Subtracts another quadrilateral from the current quadrilateral.
        ///
        /// \param Other The quadrilateral to subtract.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator-=(ConstRef<Quad> Other)
        {
            mCorners[0] -= Other.mCorners[0];
            mCorners[1] -= Other.mCorners[1];
            mCorners[2] -= Other.mCorners[2];
            mCorners[3] -= Other.mCorners[3];
            return (* this);
        }

        /// \brief Subtracts a scalar value from the quadrilateral.
        ///
        /// \param Scalar The scalar value to subtract.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator-=(Real32 Scalar)
        {
            mCorners[0] -= Scalar;
            mCorners[1] -= Scalar;
            mCorners[2] -= Scalar;
            mCorners[3] -= Scalar;
            return (* this);
        }

        /// \brief Subtracts a vector from the quadrilateral.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator-=(ConstRef<Vector2> Vector)
        {
            mCorners[0] -= Vector;
            mCorners[1] -= Vector;
            mCorners[2] -= Vector;
            mCorners[3] -= Vector;
            return (* this);
        }

        /// \brief Multiplies the quadrilateral coordinates by a scalar value.
        ///
        /// \param Scalar The scalar value to multiply.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator*=(Real32 Scalar)
        {
            mCorners[0] *= Scalar;
            mCorners[1] *= Scalar;
            mCorners[2] *= Scalar;
            mCorners[3] *= Scalar;
            return (* this);
        }

        /// \brief Multiplies the quadrilateral coordinates by a vector.
        ///
        /// \param Vector The vector to multiply.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator*=(ConstRef<Vector2> Vector)
        {
            mCorners[0] *= Vector;
            mCorners[1] *= Vector;
            mCorners[2] *= Vector;
            mCorners[3] *= Vector;
            return (* this);
        }

        /// \brief Divides the quadrilateral coordinates by a scalar value.
        ///
        /// \param Scalar The scalar value to divide by.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator/=(Real32 Scalar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");
            mCorners[0] /= Scalar;
            mCorners[1] /= Scalar;
            mCorners[2] /= Scalar;
            mCorners[3] /= Scalar;
            return (* this);
        }

        /// \brief Divides the quadrilateral coordinates by a vector.
        ///
        /// \param Vector The vector to divide by.
        /// \return A reference to the updated quadrilateral.
        ZYPHRYON_INLINE constexpr Ref<Quad> operator/=(ConstRef<Vector2> Vector)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetX()), "Division by zero (X)");
            LOG_ASSERT(!Base::IsAlmostZero(Vector.GetY()), "Division by zero (Y)");
            mCorners[0] /= Vector;
            mCorners[1] /= Vector;
            mCorners[2] /= Vector;
            mCorners[3] /= Vector;
            return (* this);
        }

        /// \brief Computes a hash value for the object.
        ///
        /// \return A hash value uniquely representing the current state of the object.
        ZYPHRYON_INLINE constexpr UInt64 Hash() const
        {
            return HashCombine(this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mCorners[0]);
            Archive.SerializeObject(mCorners[1]);
            Archive.SerializeObject(mCorners[2]);
            Archive.SerializeObject(mCorners[3]);
        }

    public:

        /// \brief Returns an empty quadrilateral.
        ///
        /// \return An empty quadrilateral.
        ZYPHRYON_INLINE constexpr static Quad Zero()
        {
            return Quad(Vector2(0, 0), Vector2(0, 0), Vector2(0, 0), Vector2(0, 0));
        }

        /// \brief Returns the unit quadrilateral with size 1.
        ///
        /// \return A unit quadrilateral.
        ZYPHRYON_INLINE constexpr static Quad One()
        {
            return Quad(Vector2(0, 0), Vector2(1, 0), Vector2(1, 1), Vector2(0, 1));
        }

        /// \brief Returns the component-wise minimum of two quads.
        ///
        /// \param First  The first quadrilateral.
        /// \param Second The second quadrilateral.
        /// \return A quadrilateral with the component-wise minimum values.
        ZYPHRYON_INLINE constexpr static Quad Min(ConstRef<Quad> First, ConstRef<Quad> Second)
        {
            return Quad(Vector2::Min(First.mCorners[0], Second.mCorners[0]),
                        Vector2::Min(First.mCorners[1], Second.mCorners[1]),
                        Vector2::Min(First.mCorners[2], Second.mCorners[2]),
                        Vector2::Min(First.mCorners[3], Second.mCorners[3]));
        }

        /// \brief Returns the component-wise maximum of two quads.
        ///
        /// \param First  The first quadrilateral.
        /// \param Second The second quadrilateral.
        /// \return A quadrilateral with the component-wise maximum values.
        ZYPHRYON_INLINE constexpr static Quad Max(ConstRef<Quad> First, ConstRef<Quad> Second)
        {
            return Quad(Vector2::Max(First.mCorners[0], Second.mCorners[0]),
                        Vector2::Max(First.mCorners[1], Second.mCorners[1]),
                        Vector2::Max(First.mCorners[2], Second.mCorners[2]),
                        Vector2::Max(First.mCorners[3], Second.mCorners[3]));
        }

        /// \brief Linearly interpolates between two quads.
        ///
        /// \param Start      The starting quadrilateral.
        /// \param End        The ending quadrilateral.
        /// \param Percentage The interpolation percentage.
        /// \return A quadrilateral interpolated between the start and end quads.
        ZYPHRYON_INLINE constexpr static Quad Lerp(ConstRef<Quad> Start, ConstRef<Quad> End, Real32 Percentage)
        {
            return Quad(Vector2::Lerp(Start.mCorners[0], End.mCorners[0], Percentage),
                        Vector2::Lerp(Start.mCorners[1], End.mCorners[1], Percentage),
                        Vector2::Lerp(Start.mCorners[2], End.mCorners[2], Percentage),
                        Vector2::Lerp(Start.mCorners[3], End.mCorners[3], Percentage));
        }

        /// \brief Transform an axis-aligned rectangle by a 4x4 matrix.
        ///
        /// \param Rectangle The input rectangle.
        /// \param Matrix    The transformation matrix.
        /// \return The projected quadrilateral.
        ZYPHRYON_INLINE static Quad Transform(ConstRef<AnyRect<Real32>> Rectangle, ConstRef<Matrix4x4> Matrix)
        {
            const Vector4 CornerX(Rectangle.GetMinimumX(), Rectangle.GetMaximumX(),
                                  Rectangle.GetMaximumX(), Rectangle.GetMinimumX());
            const Vector4 CornerY(Rectangle.GetMinimumY(), Rectangle.GetMinimumY(),
                                  Rectangle.GetMaximumY(), Rectangle.GetMaximumY());

            const Vector4 ProjectionX = (CornerX * Vector4::SplatX(Matrix.GetColumn(0)) +
                                         CornerY * Vector4::SplatX(Matrix.GetColumn(1)) +
                                                   Vector4::SplatX(Matrix.GetColumn(3)));

            const Vector4 ProjectionY = (CornerX * Vector4::SplatY(Matrix.GetColumn(0)) +
                                         CornerY * Vector4::SplatY(Matrix.GetColumn(1)) +
                                                   Vector4::SplatY(Matrix.GetColumn(3)));

            ZYPHRYON_ALIGN(16) Real32 VectorX[4];
            ZYPHRYON_ALIGN(16) Real32 VectorY[4];
            ProjectionX.Store(VectorX);
            ProjectionY.Store(VectorY);

            return Quad(Vector2(VectorX[0], VectorY[0]),
                        Vector2(VectorX[1], VectorY[1]),
                        Vector2(VectorX[2], VectorY[2]),
                        Vector2(VectorX[3], VectorY[3]));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mCorners[4];
    };
}