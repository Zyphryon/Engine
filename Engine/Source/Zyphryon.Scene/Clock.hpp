// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the scene clock, holding both an accumulating absolute time and a per-tick delta.
    class Clock final
    {
    public:

        /// The lead over a leader, in seconds, past which \ref Follow jumps back instead of easing.
        static constexpr Real64 kRewind = 2.0;

        /// The lag behind a leader, in seconds, past which \ref Follow jumps forward instead of easing.
        static constexpr Real64 kLeap   = 0.5;

        /// The fraction of the gap \ref Follow eases away when the clock is behind its leader.
        static constexpr Real64 kCatch  = 0.5;

        /// The fraction of the gap \ref Follow eases away when the clock is ahead of its leader.
        static constexpr Real64 kEase   = 0.1;

    public:

        /// \brief Constructs a clock starting at zero with a neutral multiplier of \c 1.
        ZY_INLINE Clock()
            : mAbsolute   { 0.0 },
              mDelta      { 0.0 },
              mElapsed    { 0.0 },
              mStep       { 0.0 },
              mMultiplier { 1.0 },
              mSlew       { 0.0 }
        {
        }

        /// \brief Constructs a clock with explicit initial values.
        ///
        /// \param Absolute   The initial accumulated time in seconds.
        /// \param Delta      The initial delta time in seconds.
        /// \param Multiplier The initial time scale multiplier.
        ZY_INLINE Clock(Real64 Absolute, Real64 Delta, Real32 Multiplier)
            : mAbsolute   { Absolute },
              mDelta      { Delta },
              mElapsed    { 0.0 },
              mStep       { 0.0 },
              mMultiplier { Multiplier },
              mSlew       { 0.0 }
        {
        }

        /// \brief Advances the clock by the given real-time delta, scaled by the multiplier and eased by any slew.
        ///
        /// \param Delta The elapsed real time in seconds since the last tick.
        ZY_INLINE void Tick(Real64 Delta)
        {
            mElapsed += Delta;
            mStep     = Delta;

            const Real64 Scale = Delta * mMultiplier;
            const Real64 Room  = Scale * 0.05;
            const Real64 Step  = Clamp(mSlew, -Room, Room);

            mSlew     -= Step;
            mAbsolute += Scale + Step;
            mDelta     = Scale + Step;
        }

        /// \brief Follows another clock, adopting its multiplier and closing the gap to its time.
        ///
        /// \param Leader The clock to follow, usually as last received from the publisher.
        ZY_INLINE void Follow(ConstRef<Clock> Leader)
        {
            mMultiplier = Leader.mMultiplier;

            const Real64 Behind = Leader.mAbsolute - mAbsolute;

            // Jumping back breaks timers already set, so it is kept for a clock that starts far ahead.
            if (Behind > kLeap || Behind < -kRewind)
            {
                mAbsolute = Leader.mAbsolute;
                mSlew     = 0.0;
                return;
            }

            // A late message only makes the clock look ahead, so a lead closes slower than a lag.
            mSlew = Behind * (Behind > 0.0 ? kCatch : kEase);
        }

        /// \brief Checks whether the last tick crossed a multiple of a real-time period, even while paused.
        ///
        /// \param Period The period in seconds of real time.
        /// \return `true` if the last tick crossed a multiple of the period, `false` otherwise.
        ZY_INLINE Bool IsEvery(Real64 Period) const
        {
            return Floor(mElapsed / Period) != Floor((mElapsed - mStep) / Period);
        }

        /// \brief Sets the timescale multiplier applied to each tick, where \c 0 pauses the clock.
        ///
        /// \param Multiplier The new timescale multiplier, never negative.
        ZY_INLINE void SetMultiplier(Real32 Multiplier)
        {
            ZY_ASSERT(Multiplier >= 0.0f, "Multiplier cannot be negative");

            mMultiplier = Multiplier;
        }

        /// \brief Gets the current timescale multiplier.
        ///
        /// \return The timescale multiplier applied to each tick.
        ZY_INLINE Real32 GetMultiplier() const
        {
            return mMultiplier;
        }

        /// \brief Gets the unscaled time accumulated since the clock was created.
        ///
        /// \return The elapsed real time in seconds, unaffected by the multiplier.
        ZY_INLINE Real64 GetElapsed() const
        {
            return mElapsed;
        }

        /// \brief Gets the total scaled time accumulated since the clock was created.
        ///
        /// \return The absolute elapsed time in seconds.
        ZY_INLINE Real64 GetAbsolute() const
        {
            return mAbsolute;
        }

        /// \brief Gets the scaled time elapsed during the last tick.
        ///
        /// \return The delta time in seconds for the most recent frame.
        ZY_INLINE Real64 GetDelta() const
        {
            return mDelta;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mAbsolute;
        Real64 mDelta;
        Real64 mElapsed;
        Real64 mStep;
        Real32 mMultiplier;
        Real64 mSlew;
    };
}