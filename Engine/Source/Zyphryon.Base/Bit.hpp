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

#include "Concept.hpp"
#include "Primitive.hpp"
#include <bit>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Sets one or more bits in a field.
    ///
    /// \param Field The original field value to modify.
    /// \param Mask  The bitmask indicating which bits to set.
    /// \return A new value with the specified bits set.
    template<typename Type, typename Value>
    constexpr Type SetBit(Type Field, Value Mask)
    {
        return Field | static_cast<Type>(Mask);
    }

    /// \brief Clears one or more bits in a field.
    ///
    /// \param Field The original field value to modify.
    /// \param Mask  The bitmask indicating which bits to clear.
    /// \return A new value with the specified bits cleared.
    template<typename Type, typename Value>
    constexpr Type ClearBit(Type Field, Value Mask)
    {
        return Field & ~static_cast<Type>(Mask);
    }

    /// \brief Conditionally sets or clears bits in a field based on a boolean flag.
    ///
    /// \param Field  The original field value to modify.
    /// \param Mask   The bitmask indicating which bits to set or clear.
    /// \param Enable If `true`, the bits specified by the mask are set, otherwise are cleared.
    /// \return A new value with the specified bits either set or cleared based on the condition.
    template<typename Type, typename Value>
    constexpr Type SetOrClearBit(Type Field, Value Mask, Bool Enable)
    {
        return (Enable ? SetBit(Field, Mask) : ClearBit(Field, Mask));
    }

    /// \brief Checks whether all bits in the given mask are set in the field.
    ///
    /// \param Field The field to check.
    /// \param Mask  The bitmask indicating which bits to compare.
    /// \return `true` if all bits in the mask are set in the field, `false` otherwise.
    template<typename Type, typename Value>
    constexpr Bool HasBit(Type Field, Value Mask)
    {
        return (Field & static_cast<Type>(Mask)) == static_cast<Type>(Mask);
    }

    /// \brief Returns the number of set bits in \p Word.
    ///
    /// \param Word The value to inspect.
    /// \return A count of bits set to `1` in \p Word.
    template<typename Type>
    constexpr Type CountBits(Type Word)
    {
        return static_cast<Type>(std::popcount(static_cast<Unsigned<Type>>(Word)));
    }

    /// \brief Returns the number of trailing zero bits in a non-zero \p Word.
    ///
    /// For a non-zero value this equals the zero-based index of the lowest set bit.
    ///
    /// \param Word The non-zero value to inspect.
    /// \return A count of trailing zero bits in \p Word.
    template<typename Type>
    constexpr Type CountTrailingZeros(Type Word)
    {
        ZY_ASSERT(Word != 0, "CountTrailingZeros requires a non-zero value");

        return static_cast<Type>(std::countr_zero(static_cast<Unsigned<Type>>(Word)));
    }

    /// \brief Returns the number of leading zero bits in a non-zero \p Word.
    ///
    /// \param Word The non-zero value to inspect.
    /// \return A count of leading zero bits in \p Word.
    template<typename Type>
    constexpr Type CountLeadingZeros(Type Word)
    {
        ZY_ASSERT(Word != 0, "CountLeadingZeros requires a non-zero value");

        return static_cast<Type>(std::countl_zero(static_cast<Unsigned<Type>>(Word)));
    }

    /// \brief Returns the smallest power of two greater than or equal to \p Value.
    ///
    /// \param Value The value to round up to the next power of two.
    /// \return The smallest power of two that is greater than or equal to \p Value.
    template<typename Type>
    constexpr Type CeilBit(Type Value)
    {
        return static_cast<Type>(std::bit_ceil(static_cast<Unsigned<Type>>(Value)));
    }

    /// \brief Reinterprets the bits of a value as another type.
    ///
    /// This function performs a bitwise cast from one type to another, preserving the underlying bit pattern.
    ///
    /// \param Value The value to cast.
    /// \return The value reinterpreted as the target type.
    template<typename Type, typename Source>
    constexpr Type CastBit(ConstRef<Source> Value)
    {
        return std::bit_cast<Type>(Value);
    }
}