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

#include "Profile.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef ZY_PROFILE_BACKEND_TRACY

Ptr<void> operator new(std::size_t Count)
{
    Ptr<void> Pointer = std::malloc(Count);
    TracyAllocS(Pointer, Count, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new(std::size_t Count, std::align_val_t Alignment)
{
#ifdef ZY_COMPILER_MSVC
    Ptr<void> Pointer = _aligned_malloc(static_cast<std::size_t>(Alignment), Count);
#else
    Ptr<void> Pointer = std::aligned_alloc(static_cast<std::size_t>(Alignment), Count);
#endif

    TracyAllocS(Pointer, Count, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new[](std::size_t Count)
{
    Ptr<void> Pointer = std::malloc(Count);
    TracyAllocS(Pointer, Count, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new[](std::size_t Count, std::align_val_t Alignment)
{
#ifdef ZY_COMPILER_MSVC
    Ptr<void> Pointer = _aligned_malloc(static_cast<std::size_t>(Alignment), Count);
#else
    Ptr<void> Pointer = std::aligned_alloc(static_cast<std::size_t>(Alignment), Count);
#endif

    TracyAllocS(Pointer, Count, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

void operator delete(Ptr<void> Pointer) noexcept
{
    TracyFreeS(Pointer, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(Pointer);
}

void operator delete(Ptr<void> Pointer, std::align_val_t) noexcept
{
    TracyFreeS(Pointer, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(Pointer);
}

void operator delete[](Ptr<void> Pointer) noexcept
{
    TracyFreeS(Pointer, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(Pointer);
}

void operator delete[](Ptr<void> Pointer, std::align_val_t) noexcept
{
    TracyFreeS(Pointer, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(Pointer);
}

#endif // ZY_PROFILE_BACKEND_TRACY