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

#include "Zyphryon.Base/Hash.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a compile-time UTF-8 string literal for use as a non-type template parameter.
    ///
    /// Enables passing string literals as template arguments for metaprogramming, tagging, or compile-time hashing.
    ///
    /// \tparam Capacity The number of characters in the string literal, excluding the null terminator.
    template<UInt32 Capacity>
    class Symbol final
    {
    public:

        /// \brief The UTF-8 string data, including the null terminator.
        Char Data[Capacity] = { };

    public:

        /// \brief Constructs the symbol from a string literal.
        ///
        /// \param Literal The string literal to store (must include null terminator).
        ZYPHRYON_INLINE consteval Symbol(const Char (&Literal)[Capacity])
        {
            for (UInt32 Index = 0; Index < Capacity; ++Index)
            {
                Data[Index] = Literal[Index];
            }
        }

        /// \brief Compares this string to another for equality.
        ///
        /// \param Other The other compile-time string to compare.
        /// \return `true` if all characters (including null terminator) are equal, otherwise `false`.
        ZYPHRYON_INLINE consteval Bool operator==(ConstRef<Symbol> Other) const
        {
            for (UInt32 Index = 0; Index < Capacity; ++Index)
            {
                if (Data[Index] != Other.Data[Index])
                {
                    return false;
                }
            }
            return true;
        }

        /// \brief Converts the symbol to a constant string view.
        ///
        /// \return A string-view of the symbol's string data.
        ZYPHRYON_INLINE consteval operator ConstStr8() const
        {
            return ConstStr8(Data, Capacity - 1);
        }

        /// \brief Computes a compile-time hash of the symbol's string data.
        ///
        /// \return The computed 64-bit hash value.
        ZYPHRYON_INLINE consteval UInt64 Hash() const
        {
            return HashFNV1(Data);
        }
    };
}