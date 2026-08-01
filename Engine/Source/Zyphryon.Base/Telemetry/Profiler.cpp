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

#include "Profiler.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#if defined(ZY_PROFILE_BACKEND_TRACY)

Ptr<void> operator new(std::size_t _Size)
{
    const Ptr<void> Pointer = std::malloc(_Size);
    TracyAllocS(Pointer, _Size, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new(std::size_t _Size, std::align_val_t _Al)
{
#if defined(ZY_COMPILER_MSVC)
    const Ptr<void> Pointer = _aligned_malloc(_Size, static_cast<std::size_t>(_Al));
#else
    const Ptr<void> Pointer = std::aligned_alloc(_Size, static_cast<std::size_t>(_Al));
#endif

    TracyAllocS(Pointer, _Size, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new[](std::size_t _Size)
{
    const Ptr<void> Pointer = std::malloc(_Size);
    TracyAllocS(Pointer, _Size, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

Ptr<void> operator new[](std::size_t _Size, std::align_val_t _Al)
{
#if defined(ZY_COMPILER_MSVC)
    Ptr<void> Pointer = _aligned_malloc(_Size, static_cast<std::size_t>(_Al));
#else
    Ptr<void> Pointer = std::aligned_alloc(_Size, static_cast<std::size_t>(_Al));
#endif

    TracyAllocS(Pointer, _Size, ZY_PROFILE_CALLSTACK_DEPTH);
    return Pointer;
}

void operator delete(Ptr<void> _Block) noexcept
{
    TracyFreeS(_Block, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(_Block);
}

void operator delete(Ptr<void> _Block, std::align_val_t) noexcept
{
    TracyFreeS(_Block, ZY_PROFILE_CALLSTACK_DEPTH);

#if defined(ZY_COMPILER_MSVC)
    _aligned_free(_Block);
#else
    std::free(_Block);
#endif
}

void operator delete[](Ptr<void> _Block) noexcept
{
    TracyFreeS(_Block, ZY_PROFILE_CALLSTACK_DEPTH);
    std::free(_Block);
}

void operator delete[](Ptr<void> _Block, std::align_val_t) noexcept
{
    TracyFreeS(_Block, ZY_PROFILE_CALLSTACK_DEPTH);

#if defined(ZY_COMPILER_MSVC)
    _aligned_free(_Block);
#else
    std::free(_Block);
#endif
}

#endif