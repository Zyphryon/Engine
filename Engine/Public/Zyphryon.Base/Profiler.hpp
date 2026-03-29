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

#ifdef    ZYPHRYON_PROFILE_MODE
    #include <Tracy/Tracy.hpp>
#endif // ZYPHRYON_PROFILE_MODE

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
/// \def ZYPHRYON_PROFILE_CALLSTACK_DEPTH
/// \brief Maximum number of stack frames recorded in profiling.
#define ZYPHRYON_PROFILE_CALLSTACK_DEPTH 15

/// \def ZYPHRYON_PROFILE
/// \brief Marks a profiling zone in the profiler.
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_PROFILE             ZoneScopedS(ZYPHRYON_PROFILE_CALLSTACK_DEPTH)
#else
    #define ZYPHRYON_PROFILE
#endif // ZYPHRYON_PROFILE_MODE

/// \def ZYPHRYON_PROFILE_SCOPE(Name)
/// \brief Marks a named profiling zone in the profiler.
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_PROFILE_SCOPE(Name) ZoneScopedNS(Name, ZYPHRYON_PROFILE_CALLSTACK_DEPTH)
#else
    #define ZYPHRYON_PROFILE_SCOPE(Name)
#endif // ZYPHRYON_PROFILE_MODE

/// \def ZYPHRYON_PROFILE_FRAME(x)
/// \brief Marks the beginning of a new frame in the profiler.
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_PROFILE_FRAME       FrameMark
#else
    #define ZYPHRYON_PROFILE_FRAME
#endif // ZYPHRYON_PROFILE_MODE

/// \def ZYPHRYON_PROFILE_THREAD(x)
/// \brief Sets the name of the current thread in the profiler.
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_PROFILE_THREAD(Name) tracy::SetThreadName(Name)
#else
    #define ZYPHRYON_PROFILE_THREAD(Name)
#endif // ZYPHRYON_PROFILE_MODE
}