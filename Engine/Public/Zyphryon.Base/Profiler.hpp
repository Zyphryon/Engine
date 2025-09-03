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

#include <Tracy/Tracy.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
/// \def ZYPHRYON_PROFILE_CALLSTACK_DEPTH
/// \brief Maximum number of stack frames recorded in profiling.
#define ZYPHRYON_PROFILE_CALLSTACK_DEPTH          15

/// \def ZYPHRYON_PROFILE
/// \brief Marks a profiling zone in the profiler.
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_PROFILE ZoneScopedS(ZYPHRYON_PROFILE_CALLSTACK_DEPTH)
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
    #define ZYPHRYON_PROFILE_FRAME FrameMark
#else
    #define ZYPHRYON_PROFILE_FRAME
#endif // ZYPHRYON_PROFILE_MODE

/// \def ZYPHRYON_THREAD_NAME(x)
/// \brief
#ifdef    ZYPHRYON_PROFILE_MODE
    #define ZYPHRYON_THREAD_NAME(Name) tracy::SetThreadName(Name)
#else
    #define ZYPHRYON_THREAD_NAME(Name)
#endif // ZYPHRYON_PROFILE_MODE

}