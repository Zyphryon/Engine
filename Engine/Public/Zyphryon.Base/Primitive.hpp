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

#include "Compiler.hpp"
#include <span>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Boolean value (`true` or `false`).
    using Bool      = bool;

    /// \brief Byte type (typically 8-bit).
    using Byte      = uint8_t;

    /// \brief Character type (typically 8-bit).
    using Char      = char;

    /// \brief Signed integer type with the same size as a pointer.
    using SInt      = intmax_t;

    /// \brief Unsigned integer type with the same size as a pointer.
    using UInt      = uintmax_t;

    /// \brief 8-bit signed integer (-128 to 127).
    using SInt8     = int8_t;

    /// \brief 8-bit unsigned integer (0 to 255).
    using UInt8     = uint8_t;

    /// \brief 16-bit signed integer (-32,768 to 32,767).
    using SInt16    = int16_t;

    /// \brief 16-bit unsigned integer (0 to 65,535).
    using UInt16    = uint16_t;

    /// \brief 32-bit signed integer (-2,147,483,648 to 2,147,483,647).
    using SInt32    = int32_t;

    /// \brief 32-bit unsigned integer (0 to 4,294,967,295).
    using UInt32    = uint32_t;

    /// \brief 64-bit signed integer (-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807).
    using SInt64    = int64_t;

    /// \brief 64-bit unsigned integer (0 to 18,446,744,073,709,551,615).
    using UInt64    = uint64_t;

    /// \brief Single-precision floating point number (typically 32-bit).
    using Real32    = float;

    /// \brief Double-precision floating point number (typically 64-bit).
    using Real64    = double;

    /// \brief Raw pointer to a mutable object of type `Type`.
    template<typename Type>
    using Ptr       = Type *;

    /// \brief Raw pointer to a const object of type `Type`.
    template<typename Type>
    using ConstPtr  = const Type *;

    /// \brief Non-owning reference to a mutable object of type `Type`.
    template<typename Type>
    using Ref       = Type &;

    /// \brief Non-owning reference to a const object of type `Type`.
    template<typename Type>
    using ConstRef  = const Type &;

    /// \brief Universal (forwarding) reference for perfect forwarding.
    template<typename Type>
    using AnyRef    = Type &&;

    /// \brief A lightweight view over a contiguous sequence of elements.
    template<typename Type>
    using Span      = std::span<Type>;

    /// \brief A lightweight view over a contiguous sequence of immutable elements.
    template<typename Type>
    using ConstSpan = std::span<const Type>;
}