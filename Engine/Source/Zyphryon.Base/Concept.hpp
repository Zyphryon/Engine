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
#include "Primitive.hpp"
#include <concepts>
#include <type_traits>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Selects between \p True and \p False at compile time based on a boolean \p Result.
    template<Bool Result, typename True, typename False>
    using Select        = std::conditional_t<Result, True, False>;

    /// \brief Strips lvalue and rvalue reference qualifiers from \p Type, yielding the bare value type.
    template<typename Type>
    using StripRef      = std::remove_reference_t<Type>;

    /// \brief Strips the pointer qualifier from \p Type, yielding the pointed-to type.
    template<typename Type>
    using StripPtr      = std::remove_pointer_t<Type>;

    /// \brief Strips references and const/volatile qualifiers from \p Type, decaying arrays and functions to pointers.
    template<typename Type>
    using StripAll      = std::decay_t<Type>;

    /// \brief Selects \p Min when \p Limit fits within the bit width of \p Min, otherwise selects \p Max.
    template<auto Limit, typename Min, typename Max>
    using Smallest      = Select<(Limit < (1ULL << (sizeof(Min) * 8))), Min, Max>;

    /// \brief Computes the common type to which all \p Arguments can be implicitly converted.
    template<typename... Arguments>
    using Common        = std::common_type_t<Arguments...>;

    /// \brief Produces the underlying integral type of an enumeration.
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
    using Integer       = Smallest<Limit, UInt8, Smallest<Limit, UInt16, Smallest<Limit, UInt32, UInt64>>>;

    /// \brief Yields `void` for any well-formed \p Type, used to detect valid types in SFINAE contexts.
    template<typename Type>
    using Void          = std::void_t<Type>;

    /// \brief Produces a pointer type for \p Type.
    template<typename Type>
    using Pointer       = std::add_pointer_t<Type>;

    /// \brief Concept that is satisfied when \p From is implicitly convertible to \p To.
    template<typename From, typename To>
    concept IsCastable  = std::is_convertible_v<From, To>;

    /// \brief Concept that is satisfied when \p Base is a base class of \p Derived.
    template<typename Base, typename Derived>
    concept IsDerived   = std::is_base_of_v<Base, Derived>;

    /// \brief Concept that is satisfied when \p Type is a scoped or unscoped enumeration.
    template<typename Type>
    concept IsEnum      = std::is_enum_v<Type>;

    /// \brief Concept that is satisfied when \p Type is the same as at least one type in \p Types.
    template<typename Type, typename... Types>
    concept IsAnyOf     = (std::same_as<Type, Types> || ...);

    /// \brief Concept that is satisfied when \p Type can be invoked with \p Arguments.
    template<typename Type, typename... Arguments>
    concept IsInvocable = std::is_invocable_v<Type, Arguments...>;

    /// \brief Concept that is satisfied when \p Type is const-qualified.
    template<typename Type>
    concept IsImmutable = std::is_const_v<Type>;

    /// \brief Concept that is satisfied when \p Type is an integral type.
    template<typename Type>
    concept IsIntegral  = std::is_integral_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a pointer type.
    template<typename Type>
    concept IsPointer   = std::is_pointer_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a floating-point type.
    template<typename Type>
    concept IsReal      = std::is_floating_point_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a numeric type (integral or floating-point).
    template<typename Type>
    concept IsNumeric   = IsIntegral<Type> || IsReal<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially copyable.
    template<typename Type>
    concept IsTriviallyCopyable = std::is_trivially_copyable_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially constructible.
    template<typename Type>
    concept IsTriviallyConstructible = std::is_trivially_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially destructible.
    template<typename Type>
    concept IsTriviallyDestructible = std::is_trivially_destructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type can be constructed without arguments.
    template<typename Type>
    concept IsDefaultConstructible = std::is_default_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type can be constructed from another instance of itself.
    template<typename Type>
    concept IsCopyConstructible = std::is_copy_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type can be assigned from another instance of itself.
    template<typename Type>
    concept IsCopyAssignable = std::is_copy_assignable_v<Type>;

    /// \brief Concept that is satisfied when \p Type can be constructed by taking over another instance.
    template<typename Type>
    concept IsMoveConstructible = std::is_move_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type can be assigned by taking over another instance.
    template<typename Type>
    concept IsMoveAssignable = std::is_move_assignable_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially move constructible.
    template<typename Type>
    concept IsTriviallyMoveConstructible = std::is_trivially_move_constructible_v<Type>;

    /// \brief Concept that is satisfied when \p Type is trivially move assignable.
    template<typename Type>
    concept IsTriviallyMoveAssignable = std::is_trivially_move_assignable_v<Type>;

    /// \brief Concept that is satisfied when \p Type is a signed arithmetic type.
    template<typename Type>
    concept IsSigned    = std::is_signed_v<Type>;

    /// \brief Concept that is satisfied when \p Type is an unsigned arithmetic type.
    template<typename Type>
    concept IsUnsigned  = std::is_unsigned_v<Type>;

    /// \brief Concept that is satisfied when \p Type is `void`.
    template<typename Type>
    concept IsVoid      = std::is_void_v<Type>;

    /// \brief Concept that is satisfied when \p Type is empty.
    template<typename Type>
    concept IsEmpty     = std::is_empty_v<Type>;

    /// \brief Represents an empty type used as a dummy value.
    struct Empty
    {
    };

    /// \brief Provides the type at the specified index in the parameter pack.
    template<UInt Index, typename First, typename... Rest>
    struct Identify
    {
        using Type = Identify<Index - 1, Rest...>::Type;
    };

    /// \brief Provides the first type in the parameter pack.
    template<typename First, typename... Rest>
    struct Identify<0, First, Rest...>
    {
        using Type = First;
    };

    /// \brief Holds a compile-time pack of integral values of type \p Type.
    template<typename Type, Type... Values>
    struct IntegerSequence
    {
        static constexpr UInt Count = sizeof...(Values);
    };

    /// \brief Produces an \c IntegerSequence containing every value in `[0, Count)`, in ascending order.
#if   defined(ZY_COMPILER_MSVC) || defined(ZY_COMPILER_CLANG)
    template<typename Type, UInt Count>
    using MakeIntegerSequence = __make_integer_seq<IntegerSequence, Type, static_cast<Type>(Count)>;
#else
    template<typename Type, auto Count>
    using MakeIntegerSequence = IntegerSequence<Type, __integer_pack(Count)...>;
#endif
}