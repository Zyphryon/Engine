// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Timer.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include <windows.h>
#elif defined(ZY_PLATFORM_LINUX) || defined(ZY_PLATFORM_ANDROID)
#include <time.h>
#elif defined(ZY_PLATFORM_MACOS)
#include <mach/mach_time.h>
#elif defined(ZY_PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Timer::Timer()
    {
#if defined(ZY_PLATFORM_WINDOWS)

        LARGE_INTEGER Frequency;
        QueryPerformanceFrequency(AddressOf(Frequency));
        mFrequency = 1.0 / static_cast<Real64>(Frequency.QuadPart);

#elif defined(ZY_PLATFORM_MACOS)

        mach_timebase_info_data_t Timebase;
        mach_timebase_info(AddressOf(Timebase));
        mFrequency = static_cast<Real64>(Timebase.numer) / static_cast<Real64>(Timebase.denom) / 1'000'000'000.0;

#endif

        Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real64 Timer::GetTime() const
    {
#if defined(ZY_PLATFORM_WINDOWS)

        LARGE_INTEGER Counter;
        QueryPerformanceCounter(AddressOf(Counter));
        return static_cast<Real64>(Counter.QuadPart) * mFrequency;

#elif defined(ZY_PLATFORM_LINUX) || defined(ZY_PLATFORM_ANDROID)

        struct timespec Timestamp;
        clock_gettime(CLOCK_MONOTONIC, AddressOf(Timestamp));
        return static_cast<Real64>(Timestamp.tv_sec) + static_cast<Real64>(Timestamp.tv_nsec) / 1'000'000'000.0;

#elif defined(ZY_PLATFORM_MACOS)

        const UInt64 Time = mach_absolute_time();
        return static_cast<Real64>(Time) * mFrequency;

#elif defined(ZY_PLATFORM_WEB)

        return emscripten_get_now() / 1000.0;

#endif
    }
}