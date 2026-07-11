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
/// \def ZY_ARCH_X86_64
/// \brief Defined when targeting 64-bit x86-64 architecture.

/// \def ZY_ARCH_ARM64
/// \brief Defined when targeting 64-bit ARM or ARM64 architecture.

/// \def ZY_ARCH_WASM32
/// \brief Defined when targeting 32-bit WebAssembly.

/// \def ZY_ARCH_WASM64
/// \brief Defined when targeting 64-bit WebAssembly.
#if   defined(__x86_64__)  || defined(_M_X64)
#   define ZY_ARCH_X86_64 1
#elif defined(__aarch64__) || defined(_M_ARM64)
#   define ZY_ARCH_ARM64  1
#elif defined(__wasm64__)
#   define ZY_ARCH_WASM64 1
#elif defined(__wasm32__) || defined(__wasm__)
#   define ZY_ARCH_WASM32 1
#else
#   error "Unsupported Architecture"
#endif

/// \def ZY_PLATFORM_WINDOWS
/// \brief Defined when targeting the Windows platform.

/// \def ZY_PLATFORM_WEB
/// \brief Defined when targeting the Web platform.

/// \def ZY_PLATFORM_ANDROID
/// \brief Defined when targeting the Android platform.

/// \def ZY_PLATFORM_LINUX
/// \brief Defined when targeting the Linux platform (excluding Android).

/// \def ZY_PLATFORM_MACOS
/// \brief Defined when targeting the macOS platform.
#if   defined(_WIN32)
#   define ZY_PLATFORM_WINDOWS
#elif defined(__EMSCRIPTEN__)
#   define ZY_PLATFORM_WEB
#   define ZY_PLATFORM_POSIX
#elif defined(__ANDROID__)
#   define ZY_PLATFORM_ANDROID
#   define ZY_PLATFORM_POSIX
#   error "Android is not supported yet."
#elif defined(__linux__)
#   define ZY_PLATFORM_LINUX
#   define ZY_PLATFORM_POSIX
#elif defined(__APPLE__) && defined(__MACH__)
#   define ZY_PLATFORM_MACOS
#   define ZY_PLATFORM_POSIX
#else
#   error "Unsupported Platform"
#endif

/// \def ZY_HAS_THREADS
/// \brief Defined when the platform supports threads.
#if !defined(ZY_PLATFORM_WEB) || defined(__EMSCRIPTEN_PTHREADS__)
#   define ZY_HAS_THREADS
#endif

/// \def ZY_COMPILER_MSVC
/// \brief Defined when compiling with Microsoft Visual C++.

/// \def ZY_COMPILER_CLANG
/// \brief Defined when compiling with Clang.

/// \def ZY_COMPILER_GCC
/// \brief Defined when compiling with GCC.
#if   defined(_MSC_VER)
#   define ZY_COMPILER_MSVC
#elif defined(__clang__)
#   define ZY_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
#   define ZY_COMPILER_GCC
#else
#   error "Unsupported Compiler"
#endif

/// \def ZY_INLINE
/// \brief Forces the compiler to inline the decorated function unconditionally.
#if   defined(ZY_COMPILER_MSVC)
#   define ZY_INLINE __forceinline
#elif defined(ZY_COMPILER_GCC) || defined(ZY_COMPILER_CLANG)
#   define ZY_INLINE inline __attribute__((always_inline))
#else
#   define ZY_INLINE inline
#endif

/// \def ZY_COMPRESSED
/// \brief Allows a member to share its address with another member, enabling zero-size optimization for empty types.
#if   defined(ZY_COMPILER_MSVC)
#   define ZY_COMPRESSED [[msvc::no_unique_address]]
#elif defined(ZY_COMPILER_GCC) || defined(ZY_COMPILER_CLANG)
#   define ZY_COMPRESSED [[no_unique_address]]
#else
#   define ZY_COMPRESSED
#endif

/// \def ZY_ALIGN
/// \brief Applies a compiler-specific alignment requirement to the decorated declaration.
#define ZY_ALIGN(Alignment) alignas(Alignment)

/// \def ZY_ALIGN_CPU
/// \brief Aligns the decorated declaration to the platform pointer-alignment boundary.
#define ZY_ALIGN_CPU        alignas(alignof(void *))

/// \def ZY_DEBUG_BREAK
/// \brief Triggers an unconditional debug breakpoint; has no effect in a non-debug build.
#if   defined(ZY_COMPILER_MSVC)
#   define ZY_DEBUG_BREAK() __debugbreak()
#elif defined(ZY_COMPILER_GCC) || defined(ZY_COMPILER_CLANG)
#   define ZY_DEBUG_BREAK() __builtin_trap()
#else
#   define ZY_DEBUG_BREAK()
#endif

/// \def ZY_ASSERT
/// \brief Evaluates a condition in debug mode and triggers a debug break if it is false.
#if defined(NDEBUG)
#   define ZY_ASSERT(Condition, Message) ((void)0)
#else
#   define ZY_ASSERT(Condition, Message) ((Condition) ? (void)0 : (ZY_DEBUG_BREAK(), (void)0))
#endif
}