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

#include <type_traits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Selects one of two types based on a boolean condition.
    ///
    /// \tparam Result The boolean condition to evaluate.
    /// \tparam True   The type to select if \a Result is `true`.
    /// \tparam False  The type to select if \a Result is `false`.
    template<bool Result, class True, class False>
    using Switch       = std::conditional_t<Result, True, False>;

    /// \brief Removes reference qualifiers from a type.
    ///
    /// \tparam Type The type from which to remove reference qualifiers.
    template<typename Type>
    using StripRef     = std::remove_reference_t<Type>;

    /// \brief Removes pointer qualifiers from a type.
    ///
    /// \tparam Type The type from which to remove pointer qualifiers.
    template<typename Type>
    using StripPtr     = std::remove_pointer_t<Type>;

    /// \brief Removes all pointer and reference qualifiers, as well as const/volatile qualifiers from a type.
    template<typename Type>
    using StripAll     = std::remove_cv_t<StripPtr<StripRef<Type>>>;

    /// \brief Checks if \a From is convertible to \a To.
    template<class Base, class Derived>
    concept IsCastable = std::is_convertible_v<Base, Derived>;

    /// \brief Checks if \a Type is a derived class of \a Base.
    template<class Base, class Derived>
    concept IsDerived  = std::is_base_of_v<Base, Derived>;

    /// \brief Checks if \a Type is a enum class.
    template<class Type>
    concept IsEnum      = std::is_enum_v<Type>;

    /// \brief Checks if \a Type and \a Types are equal.
    template<class Type, class ...Types>
    concept IsEqual     = (std::is_same_v<Type, Types> || ...);

    /// \brief Checks if \a Type is immutable (const).
    template<class Type>
    concept IsImmutable = std::is_const_v<Type>;

    /// \brief Checks if \a Type is an integral type.
    template<class Type>
    concept IsIntegral  = std::is_integral_v<Type>;

    /// \brief Checks if \a Type is a pointer.
    template<class Type>
    concept IsPointer   = std::is_pointer_v<Type>;

    /// \brief Checks if \a Type is a real (floating-point) type.
    template<class Type>
    concept IsReal      = std::is_floating_point_v<Type>;

    /// \brief Checks if \a Type is a trivially copyable type.
    template<class Type>
    concept IsTrivial   = std::is_trivially_copyable_v<Type>;

    /// \brief Checks if \a Type is unsigned integer.
    template<class Type>
    concept IsUnsigned  = std::is_unsigned_v<Type>;
}