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

#include "Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Represents a 2D polygon defined by an ordered ring of vertices.
    ///
    /// \note The ring is implicitly closed, so the last vertex joins back to the first without being repeated.
    ///
    /// \tparam Count The vertex capacity, or zero to grow on the heap as vertices are added.
    template<UInt Count = 0>
    class AnyPolygon final
    {
    public:

        /// \brief Initializes the polygon to an empty ring.
        ZY_INLINE constexpr AnyPolygon() = default;

        /// \brief Initializes the polygon by copying a ring of vertices.
        ///
        /// \param Vertices The ordered vertices to copy.
        ZY_INLINE constexpr explicit AnyPolygon(ConstSpan<Vector2> Vertices)
        {
            mVertices.Append(Vertices);
        }

        /// \brief Appends a vertex to the end of the ring.
        ///
        /// \param Vertex The vertex to append.
        ZY_INLINE constexpr void Append(Vector2 Vertex)
        {
            mVertices.Append(Vertex);
        }

        /// \brief Removes every vertex without releasing any storage the ring has taken.
        ZY_INLINE constexpr void Clear()
        {
            mVertices.Clear();
        }

        /// \brief Reserves storage for at least the given number of vertices.
        ///
        /// \param Capacity The number of vertices to make room for.
        ZY_INLINE constexpr void Reserve(UInt Capacity)
        {
            if constexpr (Count == 0)
            {
                mVertices.Reserve(Capacity);
            }
        }

        /// \brief Gets the vertices the polygon is defined by.
        ///
        /// \return The ordered vertices.
        ZY_INLINE constexpr ConstSpan<Vector2> GetVertices() const
        {
            return ConstSpan<Vector2>(mVertices.GetData(), mVertices.GetSize());
        }

        /// \brief Gets the number of vertices in the ring.
        ///
        /// \return The vertex count.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mVertices.GetSize();
        }

        /// \brief Checks whether the polygon has any vertex at all.
        ///
        /// \return `true` if the ring is empty, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mVertices.IsEmpty();
        }

        /// \brief Checks whether the polygon has enough vertices to enclose an area.
        ///
        /// \return `true` if the ring has at least three vertices, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mVertices.GetSize() >= 3;
        }

        /// \brief Counts how many times the ring wraps around the given point.
        ///
        /// \param Point The point to wind around.
        /// \return The winding number, which is zero when the point lies outside the ring.
        ZY_INLINE constexpr SInt32 GetWinding(Vector2 Point) const
        {
            SInt32 Winding = 0;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                ConstRef<Vector2> Head = mVertices[Index];
                ConstRef<Vector2> Tail = mVertices[(Index + 1) % mVertices.GetSize()];

                if ((Head.GetY() <= Point.GetY()) != (Tail.GetY() <= Point.GetY()))
                {
                    const Real32 Slope    = (Tail.GetX() - Head.GetX()) / (Tail.GetY() - Head.GetY());
                    const Real32 Crossing = Head.GetX() + (Point.GetY() - Head.GetY()) * Slope;

                    if (Crossing > Point.GetX())
                    {
                        Winding += (Tail.GetY() > Head.GetY()) ? 1 : -1;
                    }
                }
            }
            return Winding;
        }

        /// \brief Checks whether the given point falls inside the ring under the non-zero winding rule.
        ///
        /// \param Point The point to test.
        /// \return `true` if the point is enclosed, `false` otherwise.
        ZY_INLINE constexpr Bool Contains(Vector2 Point) const
        {
            return GetWinding(Point) != 0;
        }

        /// \brief Computes the area the ring encloses, keeping the sign of its winding.
        ///
        /// \return The signed area, which is positive when the ring winds counter-clockwise.
        ZY_INLINE constexpr Real32 GetArea() const
        {
            Real32 Total = 0.0f;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                Total += Vector2::Cross(mVertices[Index], mVertices[(Index + 1) % mVertices.GetSize()]);
            }
            return Total * 0.5f;
        }

        /// \brief Checks which way the ring is wound.
        ///
        /// \return `true` if the ring winds clockwise, `false` otherwise.
        ZY_INLINE constexpr Bool IsClockwise() const
        {
            return GetArea() < 0.0f;
        }

        /// \brief Computes the total length of the ring's edges.
        ///
        /// \return The perimeter.
        ZY_INLINE constexpr Real32 GetPerimeter() const
        {
            Real32 Total = 0.0f;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                Total += mVertices[Index].GetDistance(mVertices[(Index + 1) % mVertices.GetSize()]);
            }
            return Total;
        }

        /// \brief Computes the centre of mass of the area the ring encloses.
        /// \return The centroid.
        ZY_INLINE constexpr Vector2 GetCentroid() const
        {
            Real32  Total = 0.0f;
            Vector2 Center;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                Vector2       Head = mVertices[Index];
                const Vector2 Tail = mVertices[(Index + 1) % mVertices.GetSize()];

                const Real32 Weight = Vector2::Cross(Head, Tail);

                Total  += Weight;
                Center += (Head + Tail) * Weight;
            }

            if (IsAlmostZero(Total))
            {
                return GetAverage();
            }
            return Center / (Total * 3.0f);
        }

        /// \brief Computes the average of every vertex.
        ///
        /// \return The average position, or the origin when the ring is empty.
        ZY_INLINE constexpr Vector2 GetAverage() const
        {
            if (mVertices.IsEmpty())
            {
                return Vector2();
            }

            Vector2 Total;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                Total += mVertices[Index];
            }
            return Total / static_cast<Real32>(mVertices.GetSize());
        }

        /// \brief Computes the box that tightly encloses every vertex.
        ///
        /// \return The bounding rectangle, or an empty rectangle when the ring is empty.
        ZY_INLINE constexpr Rect GetBounds() const
        {
            if (mVertices.IsEmpty())
            {
                return Rect();
            }

            Real32 MinimumX = mVertices[0].GetX();
            Real32 MinimumY = mVertices[0].GetY();
            Real32 MaximumX = MinimumX;
            Real32 MaximumY = MinimumY;

            for (UInt Index = 1; Index < mVertices.GetSize(); ++Index)
            {
                MinimumX = Min(MinimumX, mVertices[Index].GetX());
                MinimumY = Min(MinimumY, mVertices[Index].GetY());
                MaximumX = Max(MaximumX, mVertices[Index].GetX());
                MaximumY = Max(MaximumY, mVertices[Index].GetY());
            }
            return Rect(MinimumX, MinimumY, MaximumX, MaximumY);
        }

        /// \brief Checks whether the ring turns the same way at every vertex.
        ///
        /// \return `true` if the polygon is convex, `false` otherwise.
        ZY_INLINE constexpr Bool IsConvex() const
        {
            if (!IsValid())
            {
                return false;
            }

            Bool Negative = false;
            Bool Positive = false;

            for (UInt Index = 0; Index < mVertices.GetSize(); ++Index)
            {
                ConstRef<Vector2> First  = mVertices[Index];
                ConstRef<Vector2> Second = mVertices[(Index + 1) % mVertices.GetSize()];
                ConstRef<Vector2> Third  = mVertices[(Index + 2) % mVertices.GetSize()];

                if (const Real32 Turn = Vector2::Cross(Second - First, Third - Second); Turn < 0.0f)
                {
                    Negative = true;
                }
                else if (Turn > 0.0f)
                {
                    Positive = true;
                }

                if (Negative && Positive)
                {
                    return false;
                }
            }
            return true;
        }

    public:

        /// \brief Transform a polygon using an affine transformation matrix.
        ///
        /// \param Source The polygon to transform.
        /// \param Matrix The affine transformation matrix to apply.
        /// \return A polygon resulting from transforming every vertex with the matrix.
        ZY_INLINE static AnyPolygon Transform(ConstRef<AnyPolygon> Source, ConstRef<Matrix4x3> Matrix)
        {
            AnyPolygon Result = Source;
            Matrix4x3::Project(Matrix, Source.GetVertices(), Result.mVertices);

            return Result;
        }

        /// \brief Transform a polygon using a 3x2 transformation matrix.
        ///
        /// \param Source The polygon to transform.
        /// \param Matrix The 3x2 transformation matrix to apply.
        /// \return A polygon resulting from transforming every vertex with the matrix.
        ZY_INLINE static AnyPolygon Transform(ConstRef<AnyPolygon> Source, ConstRef<Matrix3x2> Matrix)
        {
            AnyPolygon Result;
            Result.Reserve(Source.GetSize());

            for (UInt Index = 0; Index < Source.GetSize(); ++Index)
            {
                Result.Append(Matrix3x2::Project(Matrix, Source.mVertices[Index]));
            }
            return Result;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Vector2, Count> mVertices;
    };

    /// \brief Represents a 2D polygon whose ring grows on the heap.
    using Polygon = AnyPolygon<>;
}
