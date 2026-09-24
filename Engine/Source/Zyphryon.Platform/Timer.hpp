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

namespace ZyPlatform
{
    /// \brief Represents a high-precision monotonic clock that measures the time between steps.
    class ZY_API Timer final
    {
    public:

        /// \brief Constructs and initializes the platform timer.
        Timer();

        /// \brief Resets the timer's starting point to the current time.
        ZY_INLINE void Reset()
        {
            mStart = GetSeconds();
        }

        /// \brief Gets the time elapsed since the last reset or step, and starts measuring again from now.
        ///
        /// \return Elapsed time in seconds since the last reset or step.
        ZY_INLINE Real64 Step()
        {
            const Real64 Current = GetSeconds();
            const Real64 Elapsed = Current - mStart;
            mStart = Current;

            return Elapsed;
        }

        /// \brief Gets the current time in seconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in seconds.
        Real64 GetSeconds() const;

        /// \brief Gets the current time in milliseconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in milliseconds.
        ZY_INLINE UInt64 GetMilliseconds() const
        {
            return static_cast<UInt64>(GetSeconds() * 1000.0);
        }

        /// \brief Gets the current time in microseconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in microseconds.
        ZY_INLINE UInt64 GetMicroseconds() const
        {
            return static_cast<UInt64>(GetSeconds() * 1'000'000.0);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mStart;
        Real64 mFrequency;
    };
}