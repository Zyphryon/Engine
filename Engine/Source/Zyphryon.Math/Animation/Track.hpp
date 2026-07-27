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
#include "Zyphryon.Math/Bezier.hpp"

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

            mKeyframes.Append(Time, Forward<Type>(Value));
        }

        /// \brief Sets the interpolation mode.
        ///
        /// \param Mode The interpolation mode to apply when sampling.
        ZY_INLINE void SetInterpolation(Interpolation Mode)
        {
            mInterpolation = Mode;
        }

        /// \brief Checks whether the track carries any keyframe at all.
        ///
        /// \return `true` when the track is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mKeyframes.IsEmpty();
        }

        /// \brief Gets the number of keyframes the track carries.
        ///
        /// \return The keyframe count.
        ZY_INLINE UInt GetSize() const
        {
            return mKeyframes.GetSize();
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

            if constexpr (IsSlerpable<Type>)
            {
                return Type::Slerp(Prev.Value, Next.Value, Delta);
            }
            else
            {
                if constexpr (IsSplinable<Type>)
                {
                    if (mInterpolation == Interpolation::Cubic)
                    {
                        // Catmull-Rom through the four keyframes around the segment (endpoints duplicated at the bounds).
                        ConstRef<Type> P0 = mKeyframes[Index >= 2 ? Index - 2 : 0].Value;
                        ConstRef<Type> P3 = mKeyframes[Index + 1 < Count ? Index + 1 : Count - 1].Value;

                        return AnyBezier<Type>::FromCatmullRom(P0, Prev.Value, Next.Value, P3).Point(Delta);
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
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Keyframe<Type>> mKeyframes;
        Interpolation            mInterpolation;
    };
}
