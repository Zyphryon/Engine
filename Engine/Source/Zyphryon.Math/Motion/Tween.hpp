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

#include "Easing.hpp"
#include "Interpolate.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Provides smooth interpolation between two values over time.
    template<typename Type>
    class Tween final
    {
    public:

        /// \brief Creates a tween that stands still at nothing.
        ZY_INLINE Tween()
            : mStart       { },
              mEnd         { },
              mTime        { 0 },
              mAccumulator { 0 },
              mEasing      { Easing::Linear }
        {
        }

        /// \brief Constructs a tween with specified parameters.
        ///
        /// \param Start    The starting value of the tween.
        /// \param End      The target value of the tween.
        /// \param Time     The total duration of the tween in seconds.
        /// \param Function The easing function to use (default: Linear).
        ZY_INLINE Tween(Type Start, Type End, Real64 Time, Easing Function = Easing::Linear)
            : mStart       { Start },
              mEnd         { End },
              mTime        { Time },
              mAccumulator { 0 },
              mEasing      { Function }
        {
        }

        /// \brief Aims the tween at a new value, starting over from wherever it now stands.
        ///
        /// \param End  The target value of the tween.
        /// \param Time The total duration of the tween in seconds.
        ZY_INLINE void Aim(Type End, Real64 Time)
        {
            mStart       = GetValue();
            mEnd         = End;
            mTime        = Time;
            mAccumulator = 0;
        }

        /// \brief Advances the tween by the specified time delta.
        ///
        /// \param Delta The time elapsed since last update in seconds.
        /// \return The current interpolated value.
        ZY_INLINE Type Tick(Real64 Delta)
        {
            mAccumulator = Min(mAccumulator + Delta, mTime);

            return GetValue();
        }

        /// \brief Gets the value the tween stands at, leaving it where it is.
        ///
        /// \return The current interpolated value.
        ZY_INLINE Type GetValue() const
        {
            const Real32 Progress = mTime > 0.0 ? static_cast<Real32>(mAccumulator / mTime) : 1.0f;

            if constexpr (IsLerpable<Type>)
            {
                return Type::Lerp(mStart, mEnd, Ease(mEasing, Progress));
            }
            else
            {
                return Lerp<Type>(mStart, mEnd, Ease(mEasing, Progress));
            }
        }

        /// \brief Checks if the tween hasn't started yet.
        ///
        /// \return `true` if the tween is idling, `false` otherwise.
        ZY_INLINE Bool IsIdle() const
        {
            return IsAlmostZero(mTime);
        }

        /// \brief Checks if the tween has completed.
        ///
        /// \return `true` if the tween has reached its end value, `false` otherwise.
        ZY_INLINE Bool IsComplete() const
        {
            return IsAlmostEqual(mAccumulator, mTime);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type   mStart;
        Type   mEnd;
        Real64 mTime;
        Real64 mAccumulator;
        Easing mEasing;
    };
}