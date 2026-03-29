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

#include "Primitive.hpp"
#include <SDL3/SDL_timer.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents simulation time data.
    class Time final
    {
    public:

        /// \brief Default constructor.
        ZYPHRYON_INLINE Time()
            : mAbsolute { 0 },
              mDelta    { 0 }
        {
        }

        /// \brief Constructs a new time instance with specified absolute and delta values.
        ///
        /// \param Absolute The absolute time, in seconds.
        /// \param Delta    The delta time, in seconds.
        ZYPHRYON_INLINE Time(Real64 Absolute, Real64 Delta)
            : mAbsolute { Absolute },
              mDelta    { Delta }
        {
        }

        /// \brief Updates the absolute time and computes the delta since last update.
        /// 
        /// \param Absolute The new absolute time, in seconds.
        ZYPHRYON_INLINE void SetAbsolute(Real64 Absolute)
        {
            mDelta    = Absolute - mAbsolute;
            mAbsolute = Absolute;
        }

        /// \brief Retrieves the last set absolute time.
        /// 
        /// \return The absolute time in seconds.
        ZYPHRYON_INLINE Real64 GetAbsolute() const
        {
            return mAbsolute;
        }

        /// \brief Retrieves the time difference since the last call to \ref SetAbsolute.
        /// 
        /// \return The delta time in seconds.
        ZYPHRYON_INLINE Real64 GetDelta() const
        {
            return mDelta;
        }

    public:

        /// \brief Retrieves the system uptime in nanoseconds.
        ///
        /// \return The uptime in nanoseconds.
        ZYPHRYON_INLINE static UInt64 GetUptimeInNanoseconds()
        {
            return SDL_GetTicksNS();
        }

        /// \brief Retrieves the system uptime in seconds.
        ///
        /// \return The uptime in seconds.
        ZYPHRYON_INLINE static Real64 GetUptimeInSeconds()
        {
            return static_cast<Real64>(GetUptimeInNanoseconds()) * (1.0 / SDL_NS_PER_SECOND);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mAbsolute;
        Real64 mDelta;
    };
}