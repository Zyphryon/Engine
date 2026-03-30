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
#include "Zyphryon.Math/Pivot.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a 2D line segment defined by two points.
    class Line final
    {
    public:

        /// \brief Initializes the line to points (0.0f, 0.0f) and (0.0f, 0.0f).
        ZYPHRYON_INLINE constexpr Line()
            : mStart { Vector2::Zero() },
              mEnd   { Vector2::Zero() }
        {
        }

        /// \brief Constructor initializing the line with specified start and end points.
        ///
        /// \param Start The start point of the line.
        /// \param End   The end point of the line.
        ZYPHRYON_INLINE constexpr Line(Vector2 Start, Vector2 End)
            : mStart { Start },
              mEnd   { End }
        {
        }

        /// \brief Checks if the line is the zero line.
        ///
        /// \return `true` if both points are approximately zero, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsAlmostZero() const
        {
            return mStart.IsAlmostZero() && mEnd.IsAlmostZero();
        }

        /// \brief Checks if the line is valid.
        ///
        /// \return `true` if the line is valid, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsValid() const
        {
            return mStart != mEnd;
        }

        /// \brief Checks if the line has zero length.
        ///
        /// \return `true` if the line has zero length, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool IsEmpty() const
        {
            return mStart == mEnd;
        }

        /// \brief Sets the start and end points of the line.
        ///
        /// \param Start The new start point.
        /// \param End   The new end point.
        ZYPHRYON_INLINE constexpr void Set(Vector2 Start, Vector2 End)
        {
            mStart = Start;
            mEnd   = End;
        }

        /// \brief Sets the start point of the line.
        ///
        /// \param Start The new start point.
        ZYPHRYON_INLINE constexpr void SetStart(Vector2 Start)
        {
            mStart = Start;
        }

        /// \brief Gets the start point of the line.
        ///
        /// \return The start point of the line.
        ZYPHRYON_INLINE constexpr Vector2 GetStart() const
        {
            return mStart;
        }

        /// \brief Sets the end point of the line.
        ///
        /// \param End The new end point.
        ZYPHRYON_INLINE constexpr void SetEnd(Vector2 End)
        {
            mEnd = End;
        }

        /// \brief Gets the end point of the line.
        ///
        /// \return The end point of the line.
        ZYPHRYON_INLINE constexpr Vector2 GetEnd() const
        {
            return mEnd;
        }

        /// \brief Calculates the length of the line.
        ///
        /// \return The length of the line.
        ZYPHRYON_INLINE constexpr Real32 GetLength() const
        {
            return mStart.GetDistance(mEnd);
        }

        /// \brief Calculates the squared length of the line.
        ///
        /// \return The squared length of the line.
        ZYPHRYON_INLINE constexpr Real32 GetLengthSquared() const
        {
            return mStart.GetDistanceSquared(mEnd);
        }

        /// \brief Calculates the direction vector of the line.
        ///
        /// \return The normalized direction vector of the line.
        ZYPHRYON_INLINE constexpr Vector2 GetDirection() const
        {
            LOG_ASSERT(mStart != mEnd, "Cannot compute direction of a zero-length line");

            return Vector2::Normalize(mEnd - mStart);
        }

        /// \brief Calculates the midpoint of the line.
        ///
        /// \return The midpoint of the line.
        ZYPHRYON_INLINE constexpr Vector2 GetMidpoint() const
        {
            return (mStart + mEnd) * 0.5f;
        }

        /// \brief Calculates the normal vector of the line.
        ///
        /// \return The normalized normal vector of the line.
        template<Bool Clockwise = false>
        ZYPHRYON_INLINE constexpr Vector2 GetNormal() const
        {
            LOG_ASSERT(mStart != mEnd, "Cannot compute normal of a zero-length line");

            const Vector2 Direction = GetDirection();

            if constexpr (Clockwise)
            {
                return Vector2(-Direction.GetY(), Direction.GetX());
            }
            else
            {
                return Vector2(Direction.GetY(), -Direction.GetX());
            }
        }

        /// \brief Returns a point on the line segment by linear interpolation.
        ///
        /// \param Factor Interpolation parameter (0 = start, 1 = end).
        /// \return The interpolated point.
        ZYPHRYON_INLINE constexpr Vector2 GetPoint(Real32 Factor) const
        {
            return Vector2::Lerp(mStart, mEnd, Factor);
        }

        /// \brief Checks if the line segment contains a point (point lies on the segment).
        ///
        /// \param Point The point to check for containment.
        /// \return `true` if the point lies on the line segment, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Contains(Vector2 Point) const
        {
            const Vector2 Direction = mEnd - mStart;
            const Vector2 Offset    = Point - mStart;

            if (const Real32 Cross = Vector2::Cross(Direction, Offset); Abs(Cross) > kEpsilon<Real32>)
            {
                return false;
            }

            const Real32 Projection = Vector2::Dot(Direction, Offset);
            return !(Projection < 0.0f || Projection > Vector2::Dot(Direction, Direction));
        }

        /// \brief Checks if this line segment intersects with another line segment.
        ///
        /// \param Other The other line segment to check for intersection.
        /// \return `true` if the line segments intersect, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool Test(Line Other) const
        {
            const Real32 O1 = Vector2::Cross(mStart, mEnd, Other.mStart);
            const Real32 O2 = Vector2::Cross(mStart, mEnd, Other.mEnd);
            const Real32 O3 = Vector2::Cross(Other.mStart, Other.mEnd, mStart);
            const Real32 O4 = Vector2::Cross(Other.mStart, Other.mEnd, mEnd);

            if ((O1 * O2 < 0) && (O3 * O4 < 0))
            {
                return true;
            }

            return (Base::IsAlmostZero(O1) && Contains(Other.mStart)
                ||  Base::IsAlmostZero(O2) && Contains(Other.mEnd)
                ||  Base::IsAlmostZero(O3) && Other.Contains(mStart)
                ||  Base::IsAlmostZero(O4) && Other.Contains(mEnd));
        }

        /// \brief Checks if this line is equal to another line.
        ///
        /// \param Other The line to compare to.
        /// \return `true` if both points are approximately equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator==(Line Other) const
        {
            return (mStart == Other.mStart && mEnd == Other.mEnd) ||
                   (mStart == Other.mEnd   && mEnd == Other.mStart);
        }

        /// \brief Checks if this line is not equal to another line.
        ///
        /// \param Other The line to compare to.
        /// \return `true` if the lines are not equal, `false` otherwise.
        ZYPHRYON_INLINE constexpr Bool operator!=(Line Other) const
        {
            return !(* this == Other);
        }

        /// \brief Adds a vector to both points of the line.
        ///
        /// \param Vector The vector to add.
        /// \return A new line with translated points.
        ZYPHRYON_INLINE constexpr Line operator+(Vector2 Vector) const
        {
            return Line(mStart + Vector, mEnd + Vector);
        }

        /// \brief Subtracts a vector from both points of the line.
        ///
        /// \param Vector The vector to subtract.
        /// \return A new line with translated points.
        ZYPHRYON_INLINE constexpr Line operator-(Vector2 Vector) const
        {
            return Line(mStart - Vector, mEnd - Vector);
        }

        /// \brief Adds a vector to both points of the line.
        ///
        /// \param Vector The vector to add.
        /// \return A reference to the updated line.
        ZYPHRYON_INLINE constexpr Ref<Line> operator+=(Vector2 Vector)
        {
            mStart += Vector;
            mEnd   += Vector;
            return (* this);
        }

        /// \brief Subtracts a vector from both points of the line.
        ///
        /// \param Vector The vector to subtract.
        /// \return A reference to the updated line.
        ZYPHRYON_INLINE constexpr Ref<Line> operator-=(Vector2 Vector)
        {
            mStart -= Vector;
            mEnd   -= Vector;
            return (* this);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mStart);
            Archive.SerializeObject(mEnd);
        }

    public:

        /// \brief Returns a unit line segment in a specified direction from the origin.
        ///
        /// \param Direction The direction vector for the unit line (will be normalized).
        /// \return A unit line segment in the specified direction starting from origin.
        ZYPHRYON_INLINE static constexpr Line Unit(Vector2 Direction)
        {
            LOG_ASSERT(!Direction.IsAlmostZero(), "Direction must be non-zero");

            return Line(Vector2::Zero(), Vector2::Normalize(Direction));
        }

        /// \brief Returns a unit line segment along the X-axis from (0, 0) to (1, 0).
        ///
        /// \return A unit line segment along the X-axis.
        ZYPHRYON_INLINE static constexpr Line UnitX()
        {
            return Line(Vector2::Zero(), Vector2::UnitX());
        }

        /// \brief Returns a unit line segment along the Y-axis from (0, 0) to (0, 1).
        ///
        /// \return A unit line segment along the Y-axis.
        ZYPHRYON_INLINE static constexpr Line UnitY()
        {
            return Line(Vector2::Zero(), Vector2::UnitY());
        }

        /// \brief Anchors a line relative to a pivot point.
        ///
        /// \param Line   The line circle.
        /// \param Origin The pivot point for anchoring.
        /// \return A line anchored according to the pivot.
        ZYPHRYON_INLINE static constexpr Line Anchor(Line Line, Pivot Origin)
        {
            const Vector2 MinPoint    = Vector2::Min(Line.GetStart(), Line.GetEnd());
            const Vector2 MaxPoint    = Vector2::Max(Line.GetStart(), Line.GetEnd());
            const Vector2 Size        = MaxPoint - MinPoint;
            const Vector2 Translation = MinPoint + Vector2(Origin.GetX() * Size.GetX(), Origin.GetY() * Size.GetY());

            return Math::Line(Line.GetStart() - Translation, Line.GetEnd() - Translation);
        }

        /// \brief Linearly interpolates between two lines.
        ///
        /// \param Start      The starting line.
        /// \param End        The ending line.
        /// \param Percentage The interpolation percentage (range between 0 and 1).
        /// \return A line interpolated between the start and end lines.
        ZYPHRYON_INLINE static constexpr Line Lerp(Line Start, Line End, Real32 Percentage)
        {
            LOG_ASSERT(Percentage >= 0.0f && Percentage <= 1.0f, "Percentage must be in [0, 1]");

            return Line(Vector2::Lerp(Start.mStart, End.mStart, Percentage),
                        Vector2::Lerp(Start.mEnd,   End.mEnd,   Percentage));
        }

        /// \brief Transform a line using a 4x4 transformation matrix.
        ///
        /// \param Line   The line to transform.
        /// \param Matrix The 4x4 transformation matrix to apply.
        /// \return A line resulting from transforming the line with the matrix.
        ZYPHRYON_INLINE static Line Transform(Line Line, ConstRef<Matrix4x4> Matrix)
        {
            return Math::Line(Matrix4x4::Project<true>(Matrix, Line.GetStart()),
                              Matrix4x4::Project<true>(Matrix, Line.GetEnd()));
        }

        /// \brief Transform a line using a 3x2 transformation matrix.
        ///
        /// \param Line   The line to transform.
        /// \param Matrix The 3x2 transformation matrix to apply.
        /// \return A line resulting from transforming the line with the matrix.
        ZYPHRYON_INLINE static Line Transform(Line Line, ConstRef<Matrix3x2> Matrix)
        {
            return Math::Line(Matrix3x2::Project(Matrix, Line.GetStart()),
                              Matrix3x2::Project(Matrix, Line.GetEnd()));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2 mStart;
        Vector2 mEnd;
    };
}