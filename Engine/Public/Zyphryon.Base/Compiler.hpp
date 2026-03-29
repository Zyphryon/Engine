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

inline namespace Base
{
/// \def ZYPHRYON_CPU_BIT
/// \brief Defines the operating system architecture bitness (32 or 64).
#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
    #define ZYPHRYON_CPU_BIT 64
#else
    #define ZYPHRYON_CPU_BIT 32
#endif

/// \def ZYPHRYON_PLATFORM_WINDOWS
/// \def ZYPHRYON_PLATFORM_LINUX
/// \def ZYPHRYON_PLATFORM_MACOS
/// \brief Defines the platform being targeted for the current build.
#if   defined(_WIN32)
    #define ZYPHRYON_PLATFORM_WINDOWS
#elif defined(__linux__) && !defined(__ANDROID__)
    #define ZYPHRYON_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
    #define ZYPHRYON_PLATFORM_MACOS
#else
    #error "Unsupported Platform"
#endif

/// \def ZYPHRYON_COMPILER_MSVC
/// \def ZYPHRYON_COMPILER_CLANG
/// \def ZYPHRYON_COMPILER_GCC
/// \brief Defines the compiler being used for the current build.
#if   defined(_MSC_VER)
    #define ZYPHRYON_COMPILER_MSVC
#elif defined(__clang__)
    #define ZYPHRYON_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define ZYPHRYON_COMPILER_GCC
#else
    #error "Unsupported Compiler"
#endif

/// \def ZYPHRYON_INLINE
/// \brief Defines a cross-platform inline directive with compiler-specific behavior.
#if   defined(ZYPHRYON_COMPILER_MSVC)
    #define ZYPHRYON_INLINE __forceinline
#elif defined(ZYPHRYON_COMPILER_GCC) || defined(ZYPHRYON_COMPILER_CLANG)
    #define ZYPHRYON_INLINE inline __attribute__((always_inline))
#else
    #define ZYPHRYON_INLINE inline
#endif

/// \def ZYPHRYON_ALIGN
/// \brief Defines a cross-platform alignment directive for data structures.
#if   defined(ZYPHRYON_COMPILER_MSVC)
    #define ZYPHRYON_ALIGN(Alignment) __declspec(align(Alignment))
#elif defined(ZYPHRYON_COMPILER_GCC) || defined(ZYPHRYON_COMPILER_CLANG)
    #define ZYPHRYON_ALIGN(Alignment) __attribute__((aligned(Alignment)))
#else
    #define ZYPHRYON_ALIGN(Alignment)
#endif

/// \def ZYPHRYON_ALIGN_CPU
/// \brief Alignment value suitable for CPU data structures.
#define ZYPHRYON_ALIGN_CPU ZYPHRYON_ALIGN(alignof(void *))

/// \def ZYPHRYON_DEBUG_BREAK
/// \brief Triggers a debug breakpoint in the debugger.
#if   defined(ZYPHRYON_COMPILER_MSVC)
    #define ZYPHRYON_DEBUG_BREAK() __debugbreak()
#elif defined(ZYPHRYON_COMPILER_GCC) || defined(ZYPHRYON_COMPILER_CLANG)
    #define ZYPHRYON_DEBUG_BREAK() __builtin_trap()
#endif
}