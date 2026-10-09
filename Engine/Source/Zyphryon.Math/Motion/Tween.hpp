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
            : Tween(Type(), Type(), 0.0)
        {
        }

        /// \brief Constructs a tween with specified parameters.
        ///
        /// \param Start    The starting value of the tween.
        /// \param End      The target value of the tween.
        /// \param Time     The total duration of the tween in seconds.
        /// \param Function The easing function to use (default: Linear).
        /// \param Delay    The time the tween holds at its start before it moves, in seconds.
        ZY_INLINE Tween(Type Start, Type End, Real64 Time, Easing Function = Easing::Linear, Real64 Delay = 0.0)
            : mStart       { Start },
              mEnd         { End },
              mTime        { Time },
              mDelay       { Delay },
              mAccumulator { 0 },
              mEasing      { Function }
        {
        }

        /// \brief Aims the tween at a new value, starting over from wherever it now stands.
        ///
        /// \note The delay is kept, so the tween holds where it stands for the whole delay again before it moves.
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

        /// \brief Moves the value the tween is heading for, leaving where it started and how far it has come alone.
        ///
        /// \param End The target value of the tween.
        ZY_INLINE void Rearm(Type End)
        {
            mEnd = End;
        }

        /// \brief Advances the tween by the specified time delta.
        ///
        /// \param Delta The time elapsed since last update in seconds.
        /// \return The current interpolated value.
        ZY_INLINE Type Tick(Real64 Delta)
        {
            mAccumulator = Min(mAccumulator + Delta, mDelay + mTime);

            return GetValue();
        }

        /// \brief Gets the value the tween stands at, leaving it where it is.
        ///
        /// \return The current interpolated value.
        ZY_INLINE Type GetValue() const
        {
            const Real64 Elapsed  = mAccumulator - mDelay;
            const Real64 Share    = mTime > 0.0 ? Clamp(Elapsed / mTime, 0.0, 1.0) : (Elapsed >= mTime ? 1.0 : 0.0);
            const Real32 Progress = static_cast<Real32>(Share);
            const Real32 Eased    = Ease(mEasing, Progress);

            if constexpr (IsLerpable<Type>)
            {
                return Type::Lerp(mStart, mEnd, Eased);
            }
            else
            {
                return Lerp<Type>(mStart, mEnd, Eased);
            }
        }

        /// \brief Checks if the tween hasn't started yet.
        ///
        /// \return `true` if the tween is idling, `false` otherwise.
        ZY_INLINE Bool IsIdle() const
        {
            return IsAlmostZero(mTime) && IsAlmostZero(mDelay);
        }

        /// \brief Checks if the tween has completed.
        ///
        /// \return `true` if the tween has reached its end value, `false` otherwise.
        ZY_INLINE Bool IsComplete() const
        {
            return IsAlmostEqual(mAccumulator, mDelay + mTime);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type   mStart;
        Type   mEnd;
        Real64 mTime;
        Real64 mDelay;
        Real64 mAccumulator;
        Easing mEasing;
    };
}