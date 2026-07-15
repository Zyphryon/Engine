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

#include <concepts>
#include <type_traits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Selects between \p True and \p False at compile time based on a boolean \p Result.
    template<bool Result, class True, class False>
    using Select        = std::conditional_t<Result, True, False>;

    /// \brief Strips lvalue and rvalue reference qualifiers from \p Type, yielding the bare value type.
    template<typename Type>
    using StripRef      = std::remove_reference_t<Type>;

    /// \brief Strips the pointer qualifier from \p Type, yielding the pointed-to type.
    template<typename Type>
    using StripPtr      = std::remove_pointer_t<Type>;

    /// \brief Removes all pointer and reference qualifiers, as well as const/volatile qualifiers from a type.
    template<typename Type>
    using StripAll      = std::decay_t<Type>;

    /// \brief Selects \p Min when \p Limit fits within the bit width of \p Min, otherwise selects \p Max.
    template<auto Limit, typename Min, typename Max>
    using Smallest      = Select<(Limit < (1ULL << (sizeof(Min) * 8))), Min, Max>;

    /// \brief Computes the common type to which all \p Arguments can be implicitly converted.
    template<typename... Arguments>
    using Common        = std::common_type_t<Arguments...>;

    /// \brief Produces the underlying integral type of enumeration.
    template<typename Type>
    using Underlying    = std::underlying_type_t<Type>;

    /// \brief Produces a constant value of an integral type.
    template<auto Value>
    using Constant      = std::integral_constant<decltype(Value), Value>;

    /// \brief Produces the unsigned counterpart of an integral type.
    template<typename Type>
    using Unsigned      = std::make_unsigned_t<Type>;

    /// \brief Selects the smallest unsigned integer type that can represent values up to \p Limit.
    template<auto Limit>
    using Integer       = Smallest<Limit, uint8_t, Smallest<Limit, uint16_t, Smallest<Limit, uint32_t, uint64_t>>>;

    /// \brief Concept that is satisfied when \p From is implicitly convertible to \p To.
    template<class From, class To>
    concept IsCastable  = std::is_convertible_v<From, To>;

    /// \brief Concept that is satisfied when \p Base is a base class of \p Derived.
    template<class Base, class Derived>
    concept IsDerived   = std::is_base_of_v<Base, Derived>;

    /// \brief Concept that is satisfied when \p Type is a scoped or unscoped enumeration.
    template<class Type>
    concept IsEnum      = std::is_enum_v<Type>;

    /// \brief Concept that is satisfied when \p Type is the same as at least one type in \p Types.
    template<class Type, class ...Types>
    concept IsAnyOf     = (std::same_as<Type, Types> || ...);

    /// \brief Concept that is satisfied when \p Type is const-qualified.
    template<class Type>
    concept IsImmutable = std::is_const_v<Type>;

    /// \brief Concept that is satisfied when \p Type is an integral type.
    template<class Type>
    concept IsIntegral  = std::is_integral_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a pointer type.
    template<class Type>
    concept IsPointer   = std::is_pointer_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a floating-point type.
    template<class Type>
    concept IsReal      = std::is_floating_point_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a numeric type (integral or floating-point).
    template<class Type>
    concept IsNumeric   = IsIntegral<Type> || IsReal<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially copyable.
    template<class Type>
    concept IsTriviallyCopyable = std::is_trivially_copyable_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially constructible.
    template<class Type>
    concept IsTriviallyConstructible = std::is_trivially_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially destructible.
    template<class Type>
    concept IsTriviallyDestructible = std::is_trivially_destructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type is an signed arithmetic type.
    template<class Type>
    concept IsSigned    = std::is_signed_v<Type>;

    /// \brief Concept that is satisfied when \p Type is an unsigned arithmetic type.
    template<class Type>
    concept IsUnsigned  = std::is_unsigned_v<Type>;

    /// \brief Concept that is satisfied when \p Type is `void`.
    template<class Type>
    concept IsVoid      = std::is_void_v<Type>;

    /// \brief Identifies the type at the specified index in the parameter pack.
    template<auto Index, typename First, typename... Rest>
    struct Identify
    {
        using Type = Identify<Index - 1, Rest...>::Type;
    };

    /// \brief Identifies the type at the specified index in the parameter pack.
    template<typename First, typename... Rest>
    struct Identify<0, First, Rest...>
    {
        using Type = First;
    };

    /// \brief Holds a compile-time pack of integral values of type \p Type.
    template<typename Type, Type... Values>
    struct IntegerSequence
    {
        static constexpr auto Count = sizeof...(Values);
    };

    /// \brief Recursively prepends indices to build an \c IntegerSequence of `[0, Count)`.
    template<typename Type, auto Count, Type... Values>
    struct MakeIntegerSequenceBuilder
    {
        using Result = MakeIntegerSequenceBuilder<Type, Count - 1, static_cast<Type>(Count - 1), Values...>::Result;
    };

    /// \brief Terminates the recursive expansion once every index has been prepended.
    template<typename Type, Type... Values>
    struct MakeIntegerSequenceBuilder<Type, 0, Values...>
    {
        using Result = IntegerSequence<Type, Values...>;
    };

    /// \brief Produces an \c IntegerSequence containing every value in `[0, Count)`, in ascending order.
    template<typename Type, auto Count>
    using MakeIntegerSequence = MakeIntegerSequenceBuilder<Type, Count>::Result;
}