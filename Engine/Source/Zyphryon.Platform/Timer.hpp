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

namespace Platform
{
    /// \brief High-precision timer for measuring elapsed time across all platforms.
    class Timer final
    {
    public:

        /// \brief Constructs and initializes the platform timer.
        Timer();

        /// \brief Resets the timer's starting point to the current time.
        ZY_INLINE void Reset()
        {
            mStart = GetTime();
        }

        /// \brief Gets the elapsed time since the last reset and atomically resets the timer.
        ///
        /// \return Elapsed time in seconds since the last reset.
        ZY_INLINE Real64 Step()
        {
            const Real64 Current = GetTime();
            const Real64 Elapsed = Current - mStart;
            mStart = Current;

            return Elapsed;
        }

        /// \brief Gets the current time in seconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in seconds.
        ZY_INLINE Real64 GetSeconds() const
        {
            return GetTime();
        }

        /// \brief Gets the current time in milliseconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in milliseconds.
        ZY_INLINE UInt64 GetMilliseconds() const
        {
            return static_cast<UInt64>(GetTime() * 1000.0);
        }

        /// \brief Gets the current time in microseconds since an arbitrary starting point.
        ///
        /// \return Monotonic time in microseconds.
        ZY_INLINE UInt64 GetMicroseconds() const
        {
            return static_cast<UInt64>(GetTime() * 1'000'000.0);
        }

    private:

        /// \brief Gets the current raw time value from the platform.
        ///
        /// \return Raw time value in seconds.
        Real64 GetTime() const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-


        Real64 mStart;
        Real64 mFrequency;
    };
}