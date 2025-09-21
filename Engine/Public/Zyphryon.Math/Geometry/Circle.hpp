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

#include "Zyphryon.Base/Enum.hpp"
#include "Zyphryon.Math/Matrix4x4.hpp"
#include "Zyphryon.Math/Pivot.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a 2D circle defined by a center point and radius.
    class Circle final
    {
    public:

        /// \brief Initializes the circle to center (0.0f, 0.0f) with radius 0.0f.
        ZYPHRYON_INLINE constexpr Circle()
            : mCenter { 0.0f, 0.0f },
              mRadius { 0.0f }
        {
        }

        /// \brief Constructor initializing the circle with specified center and radius.
        ///
        /// \param Center The center point of the circle.
        /// \param Radius The radius of the circle.
        ZYPHRYON_INLINE constexpr Circle(ConstRef<Vector2> Center, Real32 Radius)
            : mCenter { Center },
              mRadius { Radius }
        {
            LOG_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
        }

        /// \brief Constructor initializing the circle with specified center coordinates and radius.
        ///
        /// \param X      The x-coordinate of the center.
        /// \param Y      The y-coordinate of the center.
        /// \param Radius The radius of the circle.
        ZYPHRYON_INLINE constexpr Circle(Real32 X, Real32 Y, Real32 Radius)
            : mCenter { X, Y },
              mRadius { Radius }
        {
            LOG_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
        }

        /// \brief Checks if the circle is the zero circle.
        ///
        /// \return `true` if center is approximately zero and radius is approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return mCenter.IsAlmostZero() && Base::IsAlmostZero(mRadius);
        }

        /// \brief Checks if the circle is valid (radius >= 0).
        ///
        /// \return `true` if the circle is valid, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsValid() const
        {
            return mRadius >= 0.0f;
        }

        /// \brief Checks if the circle has zero area.
        ///
        /// \return `true` if the radius is approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsEmpty() const
        {
            return Base::IsAlmostZero(mRadius);
        }

        /// \brief Sets the center and radius of the circle.
        ///
        /// \param Center The new center point.
        /// \param Radius The new radius.
        ZYPHRYON_INLINE constexpr void Set(ConstRef<Vector2> Center, Real32 Radius)
        {
            mCenter = Center;
            mRadius = Radius;
        }

        /// \brief Sets the center of the circle.
        ///
        /// \param Center The new center point.
        ZYPHRYON_INLINE constexpr void SetCenter(ConstRef<Vector2> Center)
        {
            mCenter = Center;
        }

        /// \brief Gets the center of the circle.
        ///
        /// \return The center point of the circle.
        ZYPHRYON_INLINE constexpr ConstRef<Vector2> GetCenter() const
        {
            return mCenter;
        }

        /// \brief Sets the radius of the circle.
        ///
        /// \param Radius The new radius.
        ZYPHRYON_INLINE constexpr void SetRadius(Real32 Radius)
        {
            LOG_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
            mRadius = Radius;
        }

        /// \brief Gets the radius of the circle.
        ///
        /// \return The radius of the circle.
        ZYPHRYON_INLINE constexpr Real32 GetRadius() const
        {
            return mRadius;
        }

        /// \brief Calculates the diameter of the circle.
        ///
        /// \return The diameter of the circle.
        ZYPHRYON_INLINE constexpr Real32 GetDiameter() const
        {
            return mRadius * 2.0f;
        }

        /// \brief Calculates the circumference of the circle.
        ///
        /// \return The circumference of the circle.
        ZYPHRYON_INLINE constexpr Real32 GetCircumference() const
        {
            return 2.0f * kPI<Real32> * mRadius;
        }

        /// \brief Calculates the area of the circle.
        ///
        /// \return The area of the circle.
        ZYPHRYON_INLINE constexpr Real32 GetArea() const
        {
            return kPI<Real32> * mRadius * mRadius;
        }

        /// \brief Calculates the distance from the circle's boundary to a point.
        ///
        /// \param Point The point to calculate distance to.
        /// \return The distance from the circle's boundary to the point (negative if inside the circle).
        ZYPHRYON_INLINE constexpr Real32 GetDistance(ConstRef<Vector2> Point) const
        {
            return mCenter.GetDistance(Point) - mRadius;
        }

        /// \brief Calculates the signed distance between this circle's boundary and another circle's boundary.
        ///
        /// \param Other The other circle to calculate distance to.
        /// \return The signed distance between boundaries (negative if circles overlap).
        ZYPHRYON_INLINE constexpr Real32 GetDistance(ConstRef<Circle> Other) const
        {
            return mCenter.GetDistance(Other.mCenter) - (mRadius + Other.mRadius);
        }

        /// \brief Expands the circle by the given amount.
        ///
        /// \param Amount The amount to expand by.
        /// \return The expanded circle.
        ZYPHRYON_INLINE constexpr Circle Expand(Real32 Amount) const
        {
            return Circle(mCenter, mRadius + Amount);
        }

        /// \brief Contracts the circle by the given amount.
        ///
        /// \param Amount The amount to contract by.
        /// \return The contracted circle.
        ZYPHRYON_INLINE constexpr Circle Contract(Real32 Amount) const
        {
            return Circle(mCenter, Max(0.0f, mRadius - Amount));
        }

        /// \brief Checks if this circle completely contains another circle.
        ///
        /// \param Other The other circle to check.
        /// \return `true` if this circle contains the other, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(ConstRef<Circle> Other) const
        {
            if (mRadius >= Other.mRadius)
            {
                const Real32 DistanceSquared = mCenter.GetDistanceSquared(Other.mCenter);
                const Real32 DistanceRadius  = mRadius - Other.mRadius;
                return DistanceSquared <= (DistanceRadius * DistanceRadius);
            }
            return false;
        }

        /// \brief Checks if this circle contains a point.
        ///
        /// \param Point The point to check.
        /// \return `true` if the point is inside the circle, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(ConstRef<Vector2> Point) const
        {
            return mCenter.GetDistanceSquared(Point) <= (mRadius * mRadius);
        }

        /// \brief Checks if this circle intersects with another circle.
        ///
        /// \param Other The other circle to check.
        /// \return `true` if the circles intersect, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Intersects(ConstRef<Circle> Other) const
        {
            const Real32 DistanceSquared = mCenter.GetDistanceSquared(Other.mCenter);
            return DistanceSquared <= (mRadius + Other.mRadius);
        }

        /// \brief Checks if this circle is equal to another circle.
        ///
        /// \param Other The circle to compare to.
        /// \return `true` if center and radius are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<Circle> Other) const
        {
            return mCenter == Other.mCenter && IsAlmostEqual(mRadius, Other.mRadius);
        }

        /// \brief Checks if this circle is not equal to another circle.
        ///
        /// \param Other The circle to compare to.
        /// \return `true` if the circles are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(ConstRef<Circle> Other) const
        {
            return !(* this == Other);
        }

        /// \brief Adds a vector to the circle's center.
        ///
        /// \param Vector The vector to add.
        /// \return A new circle with translated center.
        ZYPHRYON_INLINE constexpr Circle operator+(ConstRef<Vector2> Vector) const
        {
            return Circle(mCenter + Vector, mRadius);
        }

        /// \brief Subtracts a vector from the circle's center.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new circle with translated center.
        ZYPHRYON_INLINE constexpr Circle operator-(ConstRef<Vector2> Vector) const
        {
            return Circle(mCenter - Vector, mRadius);
        }

        /// \brief Multiplies the circle's radius by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A new circle with scaled radius.
        ZYPHRYON_INLINE constexpr Circle operator*(Real32 Scalar) const
        {
            return Circle(mCenter, mRadius * Scalar);
        }

        /// \brief Divides the circle's radius by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A new circle with scaled radius.
        ZYPHRYON_INLINE constexpr Circle operator/(Real32 Scalar) const
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            return Circle(mCenter, mRadius / Scalar);
        }

        /// \brief Adds a vector to the circle's center.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated circle.
        ZYPHRYON_INLINE constexpr Ref<Circle> operator+=(ConstRef<Vector2> Vector)
        {
            mCenter += Vector;
            return (* this);
        }

        /// \brief Subtracts a vector from the circle's center.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated circle.
        ZYPHRYON_INLINE constexpr Ref<Circle> operator-=(ConstRef<Vector2> Vector)
        {
            mCenter -= Vector;
            return (* this);
        }

        /// \brief Multiplies the circle's radius by a scalar.
        ///
        /// \param Scalar The scalar to multiply by.
        /// \return A reference to the updated circle.
        ZYPHRYON_INLINE constexpr Ref<Circle> operator*=(Real32 Scalar)
        {
            mRadius *= Scalar;
            return (* this);
        }

        /// \brief Divides the circle's radius by a scalar.
        ///
        /// \param Scalar The scalar to divide by.
        /// \return A reference to the updated circle.
        ZYPHRYON_INLINE constexpr Ref<Circle> operator/=(Real32 Scalar)
        {
            LOG_ASSERT(!Base::IsAlmostZero(Scalar), "Division by zero");

            mRadius /= Scalar;
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
            Archive.SerializeObject(mCenter);
            Archive.SerializeReal32(mRadius);
        }

    public:

        /// \brief Returns the unit circle centered at origin with radius 1.0.
        ///
        /// \return A unit circle.
        ZYPHRYON_INLINE constexpr static Circle Unit()
        {
            return Circle(Vector2::Zero(), 1.0f);
        }

        /// \brief Canonicalizes a circle by ensuring its radius is non-negative.
        ///
        /// \param Circle The circle to canonicalize.
        /// \return A circle with non-negative radius.
        ZYPHRYON_INLINE constexpr static Circle Canonicalize(ConstRef<Circle> Circle)
        {
            return Math::Circle(Circle.mCenter, Abs(Circle.mRadius));
        }

        /// \brief Returns the bounding circle of two circles.
        ///
        /// \param First  The first circle.
        /// \param Second The second circle.
        /// \return The bounding circle that contains both circles.
        ZYPHRYON_INLINE constexpr static Circle Merge(ConstRef<Circle> First, ConstRef<Circle> Second)
        {
            const Vector2 Direction = Second.mCenter - First.mCenter;
            const Real32 Distance   = Direction.GetLength();
            LOG_ASSERT(!Base::IsAlmostZero(Distance), "Centers are identical and neither circle contains the other");

            if (Distance + Second.mRadius <= First.mRadius)
            {
                return First;
            }

            if (Distance + First.mRadius <= Second.mRadius)
            {
                return Second;
            }

            const Real32 Radius = (Distance + First.mRadius + Second.mRadius) * 0.5f;
            return Circle(First.mCenter + Direction * ((Radius - First.mRadius) / Distance), Radius);
        }

        /// \brief Anchors a circle relative to a pivot point.
        ///
        /// \param Circle The source circle.
        /// \param Pivot  The pivot alignment mode.
        /// \return A circle anchored according to the pivot.
        ZYPHRYON_INLINE constexpr static Circle Anchor(ConstRef<Circle> Circle, Pivot Pivot)
        {
            constexpr Vector2 kMultiplier[] = {
                Vector2( 0.0f, -1.0f),  // LeftTop
                Vector2( 0.0f, -0.5f),  // LeftMiddle
                Vector2( 0.0f,  0.0f),  // LeftBottom
                Vector2(-0.5f, -1.0f),  // CenterTop
                Vector2(-0.5f, -0.5f),  // CenterMiddle
                Vector2(-0.5f,  0.0f),  // CenterBottom
                Vector2(-1.0f, -1.0f),  // RightTop
                Vector2(-1.0f, -0.5f),  // RightMiddle
                Vector2(-1.0f,  0.0f),  // RightBottom
            };
            return Math::Circle(Circle.GetCenter() + kMultiplier[Enum::Cast(Pivot)] * (Circle.mRadius * 2), Circle.mRadius);
        }

        /// \brief Linearly interpolates between two circles.
        ///
        /// \param Start      The starting circle.
        /// \param End        The ending circle.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A rectangle interpolated between the start and end circle.
        ZYPHRYON_INLINE constexpr static Circle Lerp(ConstRef<Circle> Start, ConstRef<Circle> End, Real32 Percentage)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            const Real32 Radius = Base::Lerp(Start.mRadius, End.mRadius, Percentage);
            return Circle(Vector2::Lerp(Start.mCenter, End.mCenter, Percentage), Radius);
        }

        /// \brief Transform a 2D circle by a 4x4 transformation matrix.
        ///
        /// \param Circle The input circle in local space.
        /// \param Matrix The transformation matrix to apply.
        /// \return A transformed circle in world space.
        ZYPHRYON_INLINE static Circle Transform(ConstRef<Circle> Circle, ConstRef<Matrix4x4> Matrix)
        {
            const Vector2 Center = Matrix4x4::Project<true>(Matrix, Circle.GetCenter());

            const Real32 ScaleX = Matrix.GetColumn(0).GetLength();
            const Real32 ScaleY = Matrix.GetColumn(1).GetLength();
            if (IsAlmostEqual(ScaleX, ScaleY))
            {
                return Math::Circle(Center, Circle.GetRadius() * ScaleX);
            }

            const Vector2 Edge = Matrix4x4::Project<true>(Matrix, Vector2(Circle.GetCenter() + Vector2(Circle.mRadius, 0)));
            return Math::Circle(Center, (Edge - Center).GetLength());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mCenter;
        Real32  mRadius;
    };
}