// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a timer within the ECS (Entity-Component System).
    class Timer final
    {
    public:

        /// \brief Underlying handle type used to represent the timer internally.
        using Handle = flecs::timer;

    public:

        /// \brief Constructs an invalid timer.
        ZYPHRYON_INLINE Timer()
            : mHandle (Handle::null())
        {
        }

        /// \brief Constructs a timer from an existing handle.
        ///
        /// \param Handle The handle of this timer.
        ZYPHRYON_INLINE Timer(Handle Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Returns the internal handle representing this timer.
        ///
        /// \return The timer handle.
        ZYPHRYON_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Resumes the timer, starting or continuing its countdown.
        ZYPHRYON_INLINE void Resume()
        {
            mHandle.start();
        }

        /// \brief Pauses the timer, stopping its countdown.
        ZYPHRYON_INLINE void Pause()
        {
            mHandle.stop();
        }

        /// \brief Sets the interval of the timer in seconds.
        ///
        /// \param Interval The new interval for the timer in seconds.
        ZYPHRYON_INLINE Ref<Timer> SetInterval(Real32 Interval)
        {
            mHandle.interval(Interval);
            return (* this);
        }

        /// \brief Retrieves the current interval of the timer in seconds.
        ///
        /// \return The timer's interval in seconds.
        ZYPHRYON_INLINE Real32 GetInterval() const
        {
            return mHandle.interval();
        }

        /// \brief Sets the timeout of the timer in seconds.
        ///
        /// \param Timeout The new timeout for the timer in seconds.
        ZYPHRYON_INLINE Ref<Timer> SetTimeout(Real32 Timeout)
        {
            mHandle.timeout(Timeout);
            return (* this);
        }

        /// \brief Retrieves the current timeout of the timer in seconds.
        ///
        /// \return The timer's timeout in seconds.
        ZYPHRYON_INLINE Real32 GetTimeout() const
        {
            return mHandle.timeout();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        mutable Handle mHandle;
    };
}