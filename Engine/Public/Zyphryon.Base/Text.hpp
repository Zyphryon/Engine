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

#include "Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Converts a UTF-16 encoded string to UTF-8.
    /// 
    /// \param Value The UTF-16 encoded string to convert.
    /// \return A UTF-8 encoded string containing the converted text.
    Str8 ConvertUTF16ToUTF8(std::wstring_view Value);

    /// \brief Represents a compile-time UTF-8 string literal for use as a non-type template parameter.
    ///
    /// Enables passing string literals as template arguments for metaprogramming, tagging, or compile-time hashing.
    /// The string is stored as a fixed-size array including the null terminator.
    ///
    /// \tparam Size The number of characters in the string literal, including the null terminator.
    template <UInt32 Size>
    struct ConstExprStr8
    {
        /// \brief The UTF-8 string data, including the null terminator.
        Char Data[Size] = { };

        /// \brief Constructs the compile-time string from a string literal.
        ///
        /// \param Str The string literal to store (must include null terminator).
        ZYPHRYON_INLINE constexpr ConstExprStr8(const Char (&Str)[Size])
        {
            for (UInt32 Index = 0; Index < Size; ++Index)
            {
                Data[Index] = Str[Index];
            }
        }

        /// \brief Compares this string to another for equality.
        ///
        /// \param Other The other compile-time string to compare.
        /// \return `true` if all characters (including null terminator) are equal, otherwise `false`.
        ZYPHRYON_INLINE constexpr Bool operator==(ConstRef<ConstExprStr8> Other) const
        {
            for (UInt32 Index = 0; Index < Size; ++Index)
            {
                if (Data[Index] != Other.Data[Index])
                {
                    return false;
                }
            }
            return true;
        }

        /// \brief Returns a string view of the compile-time string.
        ///
        /// \return A view of the string data (excluding the null terminator).
        ZYPHRYON_INLINE constexpr operator ConstStr8() const
        {
            return ConstStr8(Data, Size - 1);
        }

        /// \brief Computes a hash value for the compile-time string.
        ///
        /// \return A hash value uniquely representing the current state of the string.
        ZYPHRYON_INLINE constexpr UInt64 Hash() const
        {
            return HashTextType(ConstStr8(Data, Size));
        }
    };
}