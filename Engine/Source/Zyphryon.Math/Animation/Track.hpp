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

#include "Keyframe.hpp"
#include "Tween.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Determines how a track interpolates between adjacent keyframes.
    enum class Interpolation : UInt8
    {
        Step,    ///< Hold the previous keyframe's value (no interpolation).
        Linear,  ///< Linearly interpolate between adjacent keyframes.
        Cubic,   ///< Smoothly interpolate with a Catmull-Rom spline through the neighboring keyframes.
    };

    /// \brief Concept satisfied by value types that form a vector space, enabling spline interpolation.
    template<typename Type>
    concept IsSplinable = requires (Type Value, Real32 Scalar)
    {
        Value + Value;
        Value - Value;
        Value * Scalar;
    };

    /// \brief A time-ordered sequence of keyframes sampled to produce a value at any point in time.
    template<typename Type>
    class Track final
    {
    public:

        /// \brief Constructs an empty track with linear interpolation.
        ZY_INLINE Track()
            : mInterpolation { Interpolation::Linear }
        {
        }

        /// \brief Appends a keyframe; keyframes must be added in ascending time order.
        ///
        /// \param Time  The keyframe's time, in seconds.
        /// \param Value The keyframe's value.
        ZY_INLINE void Add(Real64 Time, AnyRef<Type> Value)
        {
            ZY_ASSERT(mKeyframes.IsEmpty() || Time >= mKeyframes[mKeyframes.GetSize() - 1].Time, "Keyframes must be added in ascending time order");

            mKeyframes.Append(Keyframe<Type> { Time, Forward<Type>(Value) });
        }

        /// \brief Sets the interpolation mode.
        ///
        /// \param Mode The interpolation mode to apply when sampling.
        ZY_INLINE void SetInterpolation(Interpolation Mode)
        {
            mInterpolation = Mode;
        }

        /// \brief Gets the track's duration (the time of its last keyframe).
        ///
        /// \return The duration in seconds, or `0` when the track is empty.
        ZY_INLINE Real64 GetDuration() const
        {
            return mKeyframes.IsEmpty() ? 0.0 : mKeyframes[mKeyframes.GetSize() - 1].Time;
        }

        /// \brief Samples the track at the given time, clamping to the endpoints outside the keyframe range.
        ///
        /// \param Time The time to sample at, in seconds.
        /// \return The interpolated value, or a default-constructed value when the track is empty.
        Type Sample(Real64 Time) const
        {
            if (mKeyframes.IsEmpty())
            {
                return Type { };
            }

            if (ConstRef<Keyframe<Type>> Keyframe = mKeyframes.GetFront(); Time <= Keyframe.Time)
            {
                return Keyframe.Value;
            }

            if (ConstRef<Keyframe<Type>> Keyframe = mKeyframes.GetBack(); Time >= Keyframe.Time)
            {
                return Keyframe.Value;
            }

            const UInt Count = mKeyframes.GetSize();
            UInt       Index = 1;

            while (Index < Count && mKeyframes[Index].Time < Time)
            {
                ++Index;
            }

            ConstRef<Keyframe<Type>> Prev = mKeyframes[Index - 1];

            if (mInterpolation == Interpolation::Step)
            {
                return Prev.Value;
            }

            ConstRef<Keyframe<Type>> Next  = mKeyframes[Index];
            const Real32             Delta = static_cast<Real32>((Time - Prev.Time) / (Next.Time - Prev.Time));

            if constexpr (IsSplinable<Type>)
            {
                if (mInterpolation == Interpolation::Cubic)
                {
                    // Catmull-Rom through the four keyframes around the segment (endpoints duplicated at the bounds).
                    ConstRef<Type> P0 = mKeyframes[Index >= 2 ? Index - 2 : 0].Value;
                    ConstRef<Type> P3 = mKeyframes[Index + 1 < Count ? Index + 1 : Count - 1].Value;

                    return Catmull(P0, Prev.Value, Next.Value, P3, Delta);
                }
            }

            if constexpr (IsLerpable<Type>)
            {
                return Type::Lerp(Prev.Value, Next.Value, Delta);
            }
            else
            {
                return Lerp<Type>(Prev.Value, Next.Value, Delta);
            }
        }

    private:

        /// \brief Evaluates a uniform Catmull-Rom spline segment.
        ///
        /// \param Previous The value before the segment start.
        /// \param Start    The value at the segment start.
        /// \param End      The value at the segment end.
        /// \param Next     The value after the segment end.
        /// \param Time     The normalized position within the segment, in [0, 1].
        /// \return The interpolated value.
        ZY_INLINE static Type Catmull(ConstRef<Type> Previous, ConstRef<Type> Start, ConstRef<Type> End, ConstRef<Type> Next, Real32 Time)
        {
            const Real32 TimeSquared = Time * Time;

            return (Start * 2.0f
                 + (End - Previous) * Time
                 + (Previous * 2.0f - Start * 5.0f + End * 4.0f - Next) * TimeSquared
                 + (Start * 3.0f - End * 3.0f + Next - Previous) * (TimeSquared * Time)) * 0.5f;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Keyframe<Type>> mKeyframes;
        Interpolation            mInterpolation;
    };
}
