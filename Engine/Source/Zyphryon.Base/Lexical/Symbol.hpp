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

#include "Text.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A compile-time fixed-capacity string symbol for efficient string interning and comparison.
    ///
    /// \tparam Capacity The total capacity including null terminator (must be >= 1).
    template<UInt Capacity>
    class Symbol final
    {
    public:

        /// \brief Constructs a symbol from a string literal.
        ///
        /// \param Literal A string literal array of exactly \p Capacity characters (including null terminator).
        ZY_INLINE constexpr Symbol(const Char (& Literal)[Capacity])
        {
            ::Copy(_mData, Capacity, Literal);
        }

        /// \brief Returns a pointer to the symbol's character data.
        ///
        /// \return A pointer to the first character (includes null terminator at index GetSize()).
        ZY_INLINE constexpr ConstPtr<Char> GetData() const
        {
            return _mData;
        }

        /// \brief Gets the number of characters in the symbol (excluding null terminator).
        ///
        /// \return The character count, equal to Capacity - 1.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return Capacity - 1;
        }

        /// \brief Gets the total capacity of the symbol including null terminator.
        ///
        /// \return The compile-time capacity value.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return Capacity;
        }

        /// \brief Checks if the symbol is empty (capacity of 1 means only null terminator).
        ///
        /// \return `true` if the symbol contains no characters, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return Capacity <= 1;
        }

        /// \brief Computes a 64-bit FNV-1a hash of the symbol.
        ///
        /// \return A 64-bit hash of the character data.
        ZY_INLINE constexpr UInt64 Hash() const
        {
            return ::Hash(_mData, GetSize());
        }

        /// \brief Accesses a character at the specified index.
        ///
        /// \param Index The zero-based index of the character.
        /// \return The character at the specified index.
        ZY_INLINE constexpr Char operator[](UInt Index) const
        {
            ZY_ASSERT(Index < GetSize(), "Symbol index out of bounds");

            return _mData[Index];
        }

        /// \brief Checks if this symbol is equal to another symbol.
        ///
        /// \param Other The symbol to compare with.
        /// \return `true` if both symbols have the same capacity and content, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Symbol> Other) const
        {
            if constexpr (Capacity != Other.GetCapacity())
            {
                return false;
            }
            return Compare(_mData, Other._mData, Capacity);
        }

        /// \brief Checks if this symbol is not equal to another symbol.
        ///
        /// \param Other The symbol to compare with.
        /// \return `true` if symbols differ in capacity or content, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(ConstRef<Symbol> Other) const
        {
            return !(*this == Other);
        }

        /// \brief Checks if this symbol is equal to a text view.
        ///
        /// \param Other The text view to compare with.
        /// \return `true` if the symbol content matches the text view, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Text Other) const
        {
            return GetSize() == Other.GetSize() && Compare(_mData, Other.GetData(), GetSize());
        }

        /// \brief Checks if this symbol is not equal to a text view.
        ///
        /// \param Other The text view to compare with.
        /// \return `true` if the symbol content differs from the text view, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(Text Other) const
        {
            return !(*this == Other);
        }

        /// \brief Converts the symbol to a text view.
        ///
        /// \return A text view referencing the symbol's character data (excluding null terminator).
        ZY_INLINE constexpr explicit operator Text() const
        {
            return Text(_mData, GetSize());
        }

    public:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Char _mData[Capacity] = { };
    };
}

