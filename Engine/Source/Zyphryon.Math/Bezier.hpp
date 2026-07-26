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

#include "Geometry/Rect.hpp"
#include "Polynomial.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Concept satisfied by value types that form a vector space, enabling spline interpolation.
    template<typename Type>
    concept IsSplinable = requires (Type Value, Real32 Scalar)
    {
        Value + Value;
        Value - Value;
        Value * Scalar;
    };

    /// \brief A cubic parametric curve, stored in Bézier form.
    ///
    /// \tparam Type The value the curve carries, which may be a scalar, a point, or anything else splinable.
    template<IsSplinable Type>
    class AnyBezier final
    {
    public:

        /// \brief The number of control points a cubic is defined by.
        static constexpr UInt kOrder = 4;

    public:

        /// \brief Gets one of the curve's control points.
        ///
        /// \param Index The zero-based index of the control point.
        /// \return The control point.
        ZY_INLINE constexpr Type GetControl(UInt Index) const
        {
            return mControl[Index];
        }

        /// \brief Evaluates the curve.
        ///
        /// \param Time The curve parameter, normally within the unit range.
        /// \return The value at \p Time.
        ZY_INLINE constexpr Type Point(Real32 Time) const
        {
            const Real32 Inverse = 1.0f - Time;

            return mControl[0] * (Inverse * Inverse * Inverse)
                 + mControl[1] * (3.0f * Inverse * Inverse * Time)
                 + mControl[2] * (3.0f * Inverse * Time * Time)
                 + mControl[3] * (Time * Time * Time);
        }

        /// \brief Evaluates the curve's first derivative.
        ///
        /// \param Time The curve parameter, normally within the unit range.
        /// \return The tangent at \p Time, which is not normalized.
        ZY_INLINE constexpr Type Direction(Real32 Time) const
        {
            const Real32 Inverse = 1.0f - Time;

            return (mControl[1] - mControl[0]) * (3.0f * Inverse * Inverse)
                 + (mControl[2] - mControl[1]) * (6.0f * Inverse * Time)
                 + (mControl[3] - mControl[2]) * (3.0f * Time * Time);
        }

        /// \brief Reverses the direction the curve is traced in.
        ///
        /// \return The curve, traced from its end to its start.
        ZY_INLINE constexpr AnyBezier Reverse() const
        {
            return FromBezier(mControl[3], mControl[2], mControl[1], mControl[0]);
        }

        /// \brief Splits the curve in two at the given parameter.
        ///
        /// \param Time  The curve parameter to split at.
        /// \param Left  Receives the piece running from the start to \p Time.
        /// \param Right Receives the piece running from \p Time to the end.
        ZY_INLINE constexpr void Split(Real32 Time, Ref<AnyBezier> Left, Ref<AnyBezier> Right) const
        {
            const Type Q0     = Mix(mControl[0], mControl[1], Time);
            const Type Q1     = Mix(mControl[1], mControl[2], Time);
            const Type Q2     = Mix(mControl[2], mControl[3], Time);
            const Type R0     = Mix(Q0, Q1, Time);
            const Type R1     = Mix(Q1, Q2, Time);
            const Type Middle = Mix(R0, R1, Time);

            Left  = FromBezier(mControl[0], Q0, R0, Middle);
            Right = FromBezier(Middle, R1, Q2, mControl[3]);
        }

        /// \brief Computes the box that tightly encloses the curve.
        ///
        /// \return The bounding rectangle.
        ZY_INLINE Rect Measure() const
            requires IsAnyOf<Type, Vector2>
        {
            Real32 MinimumX = Min(mControl[0].GetX(), mControl[3].GetX());
            Real32 MinimumY = Min(mControl[0].GetY(), mControl[3].GetY());
            Real32 MaximumX = Max(mControl[0].GetX(), mControl[3].GetX());
            Real32 MaximumY = Max(mControl[0].GetY(), mControl[3].GetY());

            // The derivative of a cubic is a quadratic, so each axis turns at most twice inside the segment.
            const Vector2 A = mControl[3] - mControl[2] * 3.0f + mControl[1] * 3.0f - mControl[0];
            const Vector2 B = mControl[2] - mControl[1] * 2.0f + mControl[0];
            const Vector2 C = mControl[1] - mControl[0];

            Real32 Roots[2] { };

            const UInt32 AlongX = SolveQuadratic(Span(Roots, 2), A.GetX(), 2.0f * B.GetX(), C.GetX());

            for (UInt32 Index = 0; Index < AlongX; ++Index)
            {
                if (Roots[Index] > 0.0f && Roots[Index] < 1.0f)
                {
                    const Real32 Turn = Point(Roots[Index]).GetX();

                    MinimumX = Min(MinimumX, Turn);
                    MaximumX = Max(MaximumX, Turn);
                }
            }

            const UInt32 AlongY = SolveQuadratic(Span(Roots, 2), A.GetY(), 2.0f * B.GetY(), C.GetY());

            for (UInt32 Index = 0; Index < AlongY; ++Index)
            {
                if (Roots[Index] > 0.0f && Roots[Index] < 1.0f)
                {
                    const Real32 Turn = Point(Roots[Index]).GetY();

                    MinimumY = Min(MinimumY, Turn);
                    MaximumY = Max(MaximumY, Turn);
                }
            }
            return Rect(MinimumX, MinimumY, MaximumX, MaximumY);
        }

    private:

        /// \brief Interpolates between two values.
        ///
        /// \param Start The value at zero.
        /// \param End   The value at one.
        /// \param Time  The position between them.
        /// \return The interpolated value.
        ZY_INLINE static constexpr Type Mix(Type Start, Type End, Real32 Time)
        {
            return Start + (End - Start) * Time;
        }

    public:

        /// \brief Creates a curve from its Bézier control points.
        ///
        /// \param P0 The point the curve starts at.
        /// \param P1 The control point steering the start.
        /// \param P2 The control point steering the end.
        /// \param P3 The point the curve ends at.
        /// \return The curve.
        ZY_INLINE static constexpr AnyBezier FromBezier(Type P0, Type P1, Type P2, Type P3)
        {
            AnyBezier Result;
            Result.mControl[0] = P0;
            Result.mControl[1] = P1;
            Result.mControl[2] = P2;
            Result.mControl[3] = P3;
            return Result;
        }

        /// \brief Creates a curve from a line, raising it to cubic degree.
        ///
        /// \param P0 The point the line starts at.
        /// \param P1 The point the line ends at.
        /// \return The curve, which traces the same line.
        ZY_INLINE static constexpr AnyBezier FromLinear(Type P0, Type P1)
        {
            const Type Step = (P1 - P0) * (1.0f / 3.0f);

            return FromBezier(P0, P0 + Step, P1 - Step, P1);
        }

        /// \brief Creates a curve from a quadratic, raising it to cubic degree.
        ///
        /// \param P0 The point the quadratic starts at.
        /// \param P1 The quadratic's single control point.
        /// \param P2 The point the quadratic ends at.
        /// \return The curve, which traces the same shape.
        ZY_INLINE static constexpr AnyBezier FromQuadratic(Type P0, Type P1, Type P2)
        {
            constexpr Real32 kShare = 2.0f / 3.0f;

            return FromBezier(P0, P0 + (P1 - P0) * kShare, P2 + (P1 - P2) * kShare, P2);
        }

        /// \brief Creates a curve from a Hermite segment.
        ///
        /// \param Start        The point the curve starts at.
        /// \param End          The point the curve ends at.
        /// \param TangentStart The tangent leaving \p Start.
        /// \param TangentEnd   The tangent arriving at \p End.
        /// \return The curve.
        ZY_INLINE static constexpr AnyBezier FromHermite(Type Start, Type End, Type TangentStart, Type TangentEnd)
        {
            constexpr Real32 kHandle = 1.0f / 3.0f;

            return FromBezier(Start, Start + TangentStart * kHandle, End - TangentEnd * kHandle, End);
        }

        /// \brief Creates a curve from a uniform Catmull-Rom segment.
        ///
        /// \param Previous The value before the segment.
        /// \param Start    The value the segment starts at.
        /// \param End      The value the segment ends at.
        /// \param Next     The value after the segment.
        /// \return The curve.
        ZY_INLINE static constexpr AnyBezier FromCatmullRom(Type Previous, Type Start, Type End, Type Next)
        {
            constexpr Real32 kHandle = 1.0f / 6.0f;

            return FromBezier(Start, Start + (End - Previous) * kHandle, End - (Next - Start) * kHandle, End);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Type, kOrder> mControl;
    };

    /// \brief Represents a 2D cubic Bézier curve.
    using Bezier = AnyBezier<Vector2>;
}
