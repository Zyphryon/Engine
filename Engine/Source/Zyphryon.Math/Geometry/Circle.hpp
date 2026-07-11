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

#include "Zyphryon.Math/Matrix3x2.hpp"
#include "Zyphryon.Math/Matrix4x4.hpp"
#include "Zyphryon.Math/Pivot2D.hpp"

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
        ZY_INLINE constexpr Circle()
            : mCenter { 0.0f, 0.0f },
              mRadius { 0.0f }
        {
        }

        /// \brief Constructor initializing the circle with specified center and radius.
        ///
        /// \param Center The center point of the circle.
        /// \param Radius The radius of the circle.
        ZY_INLINE constexpr Circle(Vector2 Center, Real32 Radius)
            : mCenter { Center },
              mRadius { Radius }
        {
            ZY_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
        }

        /// \brief Constructor initializing the circle with specified center coordinates and radius.
        ///
        /// \param X      The x-coordinate of the center.
        /// \param Y      The y-coordinate of the center.
        /// \param Radius The radius of the circle.
        ZY_INLINE constexpr Circle(Real32 X, Real32 Y, Real32 Radius)
            : mCenter { X, Y },
              mRadius { Radius }
        {
            ZY_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
        }

        /// \brief Checks if the circle is the zero circle.
        ///
        /// \return `true` if center is approximately zero and radius is approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsAlmostZero() const
        {
            return mCenter.IsAlmostZero() && ::IsAlmostZero(mRadius);
        }

        /// \brief Checks if the circle is valid (radius >= 0).
        ///
        /// \return `true` if the circle is valid, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mRadius >= 0.0f;
        }

        /// \brief Checks if the circle has zero area.
        ///
        /// \return `true` if the radius is approximately zero, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return ::IsAlmostZero(mRadius);
        }

        /// \brief Sets the center and radius of the circle.
        ///
        /// \param Center The new center point.
        /// \param Radius The new radius.
        ZY_INLINE constexpr void Set(Vector2 Center, Real32 Radius)
        {
            mCenter = Center;
            mRadius = Radius;
        }

        /// \brief Sets the center of the circle.
        ///
        /// \param Center The new center point.
        ZY_INLINE constexpr void SetCenter(Vector2 Center)
        {
            mCenter = Center;
        }

        /// \brief Gets the center of the circle.
        ///
        /// \return The center point of the circle.
        ZY_INLINE constexpr Vector2 GetCenter() const
        {
            return mCenter;
        }

        /// \brief Sets the radius of the circle.
        ///
        /// \param Radius The new radius.
        ZY_INLINE constexpr void SetRadius(Real32 Radius)
        {
            ZY_ASSERT(Radius >= 0.0f, "Circle radius must be non-negative");
            mRadius = Radius;
        }

        /// \brief Gets the radius of the circle.
        ///
        /// \return The radius of the circle.
        ZY_INLINE constexpr Real32 GetRadius() const
        {
            return mRadius;
        }

        /// \brief Calculates the diameter of the circle.
        ///
        /// \return The diameter of the circle.
        ZY_INLINE constexpr Real32 GetDiameter() const
        {
            return mRadius * 2.0f;
        }

        /// \brief Calculates the circumference of the circle.
        ///
        /// \return The circumference of the circle.
        ZY_INLINE constexpr Real32 GetCircumference() const
        {
            return 2.0f * kPI<Real32> * mRadius;
        }

        /// \brief Calculates the area of the circle.
        ///
        /// \return The area of the circle.
        ZY_INLINE constexpr Real32 GetArea() const
        {
            return kPI<Real32> * mRadius * mRadius;
        }

        /// \brief Calculates the distance from the circle's boundary to a point.
        ///
        /// \param Point The point to calculate distance to.
        /// \return The distance from the circle's boundary to the point (negative if inside the circle).
        ZY_INLINE Real32 GetDistance(Vector2 Point) const
        {
            return mCenter.GetDistance(Point) - mRadius;
        }

        /// \brief Calculates the signed distance between this circle's boundary and another circle's boundary.
        ///
        /// \param Other The other circle to calculate distance to.
        /// \return The signed distance between boundaries (negative if circles overlap).
        ZY_INLINE Real32 GetDistance(Circle Other) const
        {
            return mCenter.GetDistance(Other.mCenter) - (mRadius + Other.mRadius);
        }

        /// \brief Expands the circle by the given amount.
        ///
        /// \param Amount The amount to expand by.
        /// \return The expanded circle.
        ZY_INLINE constexpr Circle Expand(Real32 Amount) const
        {
            return Circle(mCenter, mRadius + Amount);
        }

        /// \brief Contracts the circle by the given amount.
        ///
        /// \param Amount The amount to contract by.
        /// \return The contracted circle.
        ZY_INLINE constexpr Circle Contract(Real32 Amount) const
        {
            return Circle(mCenter, Max(0.0f, mRadius - Amount));
        }

        /// \brief Checks if this circle completely contains another circle.
        ///
        /// \param Other The other circle to check.
        /// \return `true` if this circle contains the other, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Circle Other) const
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
        ZY_INLINE constexpr Bool Contains(Vector2 Point) const
        {
            return mCenter.GetDistanceSquared(Point) <= (mRadius * mRadius);
        }

        /// \brief Checks if this circle intersects with another circle.
        ///
        /// \param Other The other circle to check.
        /// \return `true` if the circles intersect, `false` otherwise.
        ZY_INLINE constexpr Bool Test(Circle Other) const
        {
            const Real32 DistanceSquared = mCenter.GetDistanceSquared(Other.mCenter);
            const Real32 RadiusSum       = mRadius + Other.mRadius;
            return DistanceSquared <= (RadiusSum * RadiusSum);
        }

        /// \brief Checks if this circle is equal to another circle.
        ///
        /// \param Other The circle to compare to.
        /// \return `true` if center and radius are approximately equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Circle Other) const
        {
            return mCenter == Other.mCenter && IsAlmostEqual(mRadius, Other.mRadius);
        }

        /// \brief Checks if this circle is not equal to another circle.
        ///
        /// \param Other The circle to compare to.
        /// \return `true` if the circles are not equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(Circle Other) const
        {
            return !(* this == Other);
        }

    public:

        /// \brief Gets the unit circle centered at origin with radius 1.0.
        ///
        /// \return A unit circle.
        ZY_INLINE static constexpr Circle Unit()
        {
            return Circle(Vector2::Zero(), 1.0f);
        }

        /// \brief Canonicalizes a circle by ensuring its radius is non-negative.
        ///
        /// \param Source The circle to canonicalize.
        /// \return A circle with non-negative radius.
        ZY_INLINE static constexpr Circle Canonicalize(Circle Source)
        {
            return ::Circle(Source.mCenter, Abs(Source.mRadius));
        }

        /// \brief Gets the bounding circle of two circles.
        ///
        /// \param First  The first circle.
        /// \param Second The second circle.
        /// \return The bounding circle that contains both circles.
        ZY_INLINE static Circle Merge(Circle First, Circle Second)
        {
            const Vector2 Direction = Second.mCenter - First.mCenter;
            const Real32 Distance   = Direction.GetLength();

            if (Distance + Second.mRadius <= First.mRadius)
            {
                return First;
            }

            if (Distance + First.mRadius <= Second.mRadius)
            {
                return Second;
            }

            ZY_ASSERT(!::IsAlmostZero(Distance), "Centers are identical and neither circle contains the other");

            const Real32 Radius = (Distance + First.mRadius + Second.mRadius) * 0.5f;
            return Circle(First.mCenter + Direction * ((Radius - First.mRadius) / Distance), Radius);
        }

        /// \brief Anchors a circle relative to a pivot point.
        ///
        /// \param Source The source circle.
        /// \param Origin The pivot point for anchoring.
        /// \return A circle anchored according to the pivot.
        ZY_INLINE static constexpr Circle Anchor(Circle Source, Pivot2D Origin)
        {
            const Real32 Radius   = Source.GetRadius();
            const Real32 Diameter = Radius * 2;

            const Vector2 Translation = -Source.GetCenter() - Vector2(
                (Origin.GetX() * Diameter) - Radius,
                (Origin.GetY() * Diameter) - Radius
            );
            return ::Circle(Source.GetCenter() + Translation, Radius);
        }

        /// \brief Linearly interpolates between two circles.
        ///
        /// \param Start      The starting circle.
        /// \param End        The ending circle.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A circle interpolated between the start and end circle.
        ZY_INLINE static constexpr Circle Lerp(Circle Start, Circle End, Real32 Percentage)
        {
            ZY_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            const Real32 Radius = ::Lerp(Start.mRadius, End.mRadius, Percentage);
            return Circle(::Lerp(Start.mCenter, End.mCenter, Percentage), Radius);
        }

        /// \brief Transform a circle using a 4x4 transformation matrix.
        ///
        /// \param Source The circle to transform.
        /// \param Matrix The 4x4 transformation matrix to apply.
        /// \return A circle resulting from transforming the circle with the matrix.
        ZY_INLINE static Circle Transform(Circle Source, ConstRef<Matrix4x4> Matrix)
        {
            const Vector2 Center = Matrix4x4::Project<true>(Matrix, Source.GetCenter());

            const Real32 ScaleX = Matrix.GetColumn(0).GetLength();
            const Real32 ScaleY = Matrix.GetColumn(1).GetLength();

            if (IsAlmostEqual(ScaleX, ScaleY))
            {
                return ::Circle(Center, Source.GetRadius() * ScaleX);
            }

            const Vector2 Edge = Matrix4x4::Project<true>(Matrix, Source.GetCenter() + Vector2(Source.mRadius, 0));
            return Circle(Center, (Edge - Center).GetLength());
        }

        /// \brief Transform a circle using a 3x2 transformation matrix.
        ///
        /// \param Source The circle to transform.
        /// \param Matrix The 3x2 transformation matrix to apply.
        /// \return A circle resulting from transforming the circle with the matrix.
        ZY_INLINE static Circle Transform(Circle Source, ConstRef<Matrix3x2> Matrix)
        {
            const Vector2 Center = Matrix3x2::Project(Matrix, Source.GetCenter());

            const Real32 ScaleX = Matrix.GetBasisX().GetLength();
            const Real32 ScaleY = Matrix.GetBasisY().GetLength();

            if (IsAlmostEqual(ScaleX, ScaleY))
            {
                return ::Circle(Center, Source.GetRadius() * ScaleX);
            }

            const Vector2 Edge = Matrix3x2::Project(Matrix, Source.GetCenter() + Vector2(Source.GetRadius(), 0.0f));
            return Circle(Center, (Edge - Center).GetLength());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mCenter;
        Real32  mRadius;
    };
}