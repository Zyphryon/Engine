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

#include "Primitive.hpp"
#include <type_traits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Resolves to `Type` if `Result` is `true`, `Other` otherwise.
    /// 
    /// \tparam Result A compile-time boolean condition.
    /// \tparam Type   The type to select if `Result` is `true`.
    /// \tparam Other  The type to select if `Result` is `false`.
    /// \return The selected type.
    template<Bool Result, class Type, class Other>
    using Switch       = std::conditional_t<Result, Type, Other>;

    /// \brief Removes any reference qualifier from a type.
    ///
    /// \tparam Type The type to transform.
    /// \return The underlying type with no reference qualifier.
    template<typename Type>
    using UnRef        = std::remove_reference_t<Type>;

    /// \brief Removes any pointer qualifier from a type.
    ///
    /// \tparam Type The type to transform.
    /// \return The underlying type with no pointer qualifier.
    template<typename Type>
    using UnPtr        = std::remove_pointer_t<Type>;

    /// \brief Checks if \a Type and \a Types are equal.
    template<class Type, class ...Types>
    concept IsEqual    = (std::is_same_v<Type, Types> || ...);

    /// \brief Checks if \a Type is a boolean.
    template<class Type>
    concept IsBool     = IsEqual<Type, Bool>;

    /// \brief Checks if \a Type is float point number.
    template<class Type>
    concept IsReal     = IsEqual<Type, Real32, Real64>;

    /// \brief Checks if \a Type is an integer.
    template<class Type>
    concept IsInteger  = IsEqual<Type, UInt8, SInt8, UInt16, SInt16, UInt32, SInt32, UInt64, SInt64>;

    /// \brief Checks if \a Type is an enumeration.
    template<class Type>
    concept IsEnum     = std::is_enum_v<Type>;

    /// \brief Checks if \a Type is a text.
    template<class Type>
    concept IsText     = IsEqual<Type, Str8, ConstStr8>;

    /// \brief Checks if \a Type is a pointer.
    template<class Type>
    concept IsPointer  = std::is_pointer_v<Type>;

    /// \brief Checks if \a Type is a derived class of \a Base.
    template<class Base, class Derived>
    concept IsDerived  = std::is_base_of_v<Base, Derived>;

    /// \brief Checks if \a Type is mutable (not const).
    template<class Type>
    concept IsMutable  = !std::is_const_v<Type>;

    /// \brief Checks if \a Type is unsigned integer.
    template<class Type>
    concept IsUnsigned = std::is_unsigned_v<Type>;
}