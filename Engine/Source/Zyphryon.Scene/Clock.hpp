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

namespace Scene
{
    /// \brief Encapsulate the scene clock, providing both an accumulating absolute time and a per-tick delta.
    class Clock final
    {
    public:

        /// \brief Constructs a clock starting at zero with a neutral multiplier of \c 1.
        ZY_INLINE Clock()
            : mAbsolute   { 0.0 },
              mDelta      { 0.0 },
              mMultiplier { 1.0 }
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
              mMultiplier { Multiplier }
        {
        }

        /// \brief Advances the clock by the given real-time delta, scaled by the current multiplier.
        ///
        /// \param Delta The elapsed real time in seconds since the last tick.
        ZY_INLINE void Tick(Real64 Delta)
        {
            const Real64 Scale = Delta * mMultiplier;
            mAbsolute += Scale;
            mDelta     = Scale;
        }

        /// \brief Returns the total scaled time accumulated since the clock was created.
        ///
        /// \return The absolute elapsed time in seconds.
        ZY_INLINE Real64 GetAbsolute() const
        {
            return mAbsolute;
        }

        /// \brief Returns the scaled time elapsed during the last tick.
        ///
        /// \return The delta time in seconds for the most recent frame.
        ZY_INLINE Real64 GetDelta() const
        {
            return mDelta;
        }

        /// \brief Sets the timescale multiplier applied to each tick.
        ///
        /// A value of \c 1.0 runs at normal speed, values above \c 1.0 accelerate time,
        /// and \c 0.0 pauses the clock.
        ///
        /// \param Multiplier The new timescale multiplier. Must be greater than or equal to \c 0.
        ZY_INLINE void SetMultiplier(Real32 Multiplier)
        {
            ZY_ASSERT(Multiplier >= 0.0f, "Multiplier cannot be negative");

            mMultiplier = Multiplier;
        }

        /// \brief Returns the current timescale multiplier.
        ///
        /// \return The timescale multiplier applied to each tick.
        ZY_INLINE Real32 GetMultiplier() const
        {
            return mMultiplier;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mAbsolute;
        Real64 mDelta;
        Real32 mMultiplier;
    };
}