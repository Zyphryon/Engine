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

#include "Zyphryon.Base/Primitive.hpp"

#if defined(ZY_PROFILE_BACKEND_TRACY)
#   include <tracy/Tracy.hpp>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
 /// \def ZY_PROFILE_CALLSTACK_DEPTH
/// \brief Maximum number of stack frames recorded in profiling.
#define ZY_PROFILE_CALLSTACK_DEPTH 15

/// \def ZY_PROFILE
/// \brief Marks a profiling zone in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE ZoneScopedS(ZY_PROFILE_CALLSTACK_DEPTH)
#else
#   define ZY_PROFILE
#endif

/// \def ZY_PROFILE_INTEGER(Name, Value)
/// \brief Records a named whole number in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE_INTEGER(Name, Value) TracyPlot(Name, static_cast<SInt64>(Value))
#else
#   define ZY_PROFILE_INTEGER(Name, Value) ((void)(Value))
#endif

/// \def ZY_PROFILE_REAL(Name, Value)
/// \brief Records a named real number in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE_REAL(Name, Value) TracyPlot(Name, static_cast<Real64>(Value))
#else
#   define ZY_PROFILE_REAL(Name, Value) ((void)(Value))
#endif

/// \def ZY_PROFILE_SCOPE(Name)
/// \brief Marks a named profiling zone in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE_SCOPE(Name) ZoneScopedNS(Name, ZY_PROFILE_CALLSTACK_DEPTH)
#else
#   define ZY_PROFILE_SCOPE(Name)
#endif

/// \def ZY_PROFILE_FRAME(x)
/// \brief Marks the beginning of a new frame in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE_FRAME FrameMark
#else
#   define ZY_PROFILE_FRAME
#endif

/// \def ZY_PROFILE_THREAD(x)
/// \brief Sets the name of the current thread in the profiler.
#if defined(ZY_PROFILE_BACKEND_TRACY)
#   define ZY_PROFILE_THREAD(Name) tracy::SetThreadName(Name)
#else
#   define ZY_PROFILE_THREAD(Name)
#endif
}