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
#include "Zyphryon.Base/Container/Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A non-owning view of a contiguous sequence of characters.
    class Text final
    {
    public:

        /// \brief Constructs an empty text view.
        ZY_INLINE constexpr Text()
            : mData { nullptr },
              mSize { 0u }
        {
        }

        /// \brief Constructs a text view from a pointer and size.
        ///
        /// \param Data The pointer to the first character.
        /// \param Size The number of characters in the view.
        ZY_INLINE constexpr Text(ConstPtr<Char> Data, UInt Size)
            : mData { Data },
              mSize { Size }
        {
        }

        /// \brief Constructs a text view from a string literal.
        ///
        /// \param Literal The string literal array of exactly \p Count characters (excluding the null terminator).
        template<UInt Count>
        ZY_INLINE constexpr Text(const Char (& Literal)[Count])
            : mData { Literal },
              mSize { Count - 1 }
        {
        }

        /// \brief Constructs a text view from a pointer range.
        ///
        /// \param First The pointer to the first character.
        /// \param Last  The pointer to one past the last character.
        ZY_INLINE constexpr Text(ConstPtr<Char> First, ConstPtr<Char> Last)
            : mData { First },
              mSize { static_cast<UInt>(Last - First) }
        {
        }

        /// \brief Constructs a text view from any type satisfying the \c IsText concept.
        ///
        /// \param Value The text-like object to view.
        template<typename Type>
        ZY_INLINE constexpr Text(ConstRef<Type> Value)
            requires IsContiguousOf<Type, Char>
            : mData { Value.GetData() },
              mSize { Value.GetSize() }
        {
        }

        /// \brief Checks if the text view is empty.
        ///
        /// \return `true` if the view contains no characters, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mSize == 0u;
        }

        /// \brief Returns the number of characters in the view.
        ///
        /// \return A count of characters in the view.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Returns a pointer to the first character in the view.
        ///
        /// \return A pointer to the character data, or `nullptr` if the view is empty.
        ZY_INLINE constexpr ConstPtr<Char> GetData() const
        {
            return mData;
        }

        /// \brief Returns the first character in the view.
        ///
        /// \return A copy of the first character in the view.
        ZY_INLINE constexpr Char GetFront() const
        {
            ZY_ASSERT(mSize > 0, "Text view is empty");

            return mData[0];
        }

        /// \brief Returns the last character in the view.
        ///
        /// \return A copy of the last character in the view.
        ZY_INLINE constexpr Char GetBack() const
        {
            ZY_ASSERT(mSize > 0, "Text view is empty");

            return mData[mSize - 1];
        }

        /// \brief Creates a subview of this text starting at the given offset.
        ///
        /// \param Offset The starting position of the subview.
        /// \param Count  The number of characters to include. If zero, includes all remaining characters.
        /// \return A new text view representing the subview.
        ZY_INLINE constexpr Text Slice(UInt Offset, UInt Count = 0u) const
        {
            const UInt Length = (Count == 0u ? mSize - Offset : Count);
            ZY_ASSERT(Offset + Length <= mSize, "Slice range or offset exceeds text size");

            return Text(mData + Offset, Length);
        }

        /// \brief Computes a 64-bit hash of the text.
        ///
        /// \return A 64-bit hash of the character data, or zero if the view is empty.
        ZY_INLINE constexpr UInt64 Hash() const
        {
            return (mSize > 0) ? ::Hash(mData, mSize) : 0;
        }

        /// \brief Accesses a character at the specified index without bounds checking.
        ///
        /// \param Index The zero-based index of the character.
        /// \return A copy of the character at the specified index.
        ZY_INLINE constexpr Char operator[](UInt Index) const
        {
            ZY_ASSERT(Index < mSize, "Text index is out of bounds");

            return mData[Index];
        }

        /// \brief Checks whether this view and \p Other contain the same characters.
        ///
        /// \param Other The text to compare against.
        /// \return `true` if both views have equal size and identical contents, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Text Other) const
        {
            return mSize == Other.mSize && ::Compare(mData, Other.mData, mSize);
        }

        /// \brief Checks whether this view and \p Other differ in size or contents.
        ///
        /// \param Other The text to compare against.
        /// \return `true` if the views differ in size or contents, `false` otherwise.
        ZY_INLINE constexpr Bool operator!=(Text Other) const
        {
            return !(*this == Other);
        }

    public:

        /// \brief Gets an empty text view.
        ///
        /// \return A text view representing an empty string.
        ZY_INLINE static constexpr Text Empty()
        {
            return Text();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<Char> mData;
        UInt           mSize;
    };

    /// \brief Compile-time string literal to text view conversion operator.
    ///
    /// \param Data The pointer to the first character of the string literal.
    /// \param Size The number of characters in the literal (excluding null terminator).
    /// \return A \c Text view representing the string literal.
    consteval auto operator ""_Text(ConstPtr<Char> Data, size_t Size)
    {
        return Text(Data, Size);
    }

    /// \brief Compile-time string literal to hash conversion operator.
    ///
    /// \param Data The pointer to the first character of the string literal.
    /// \param Size The number of characters in the literal (excluding null terminator).
    /// \return The hash of the string literal, guaranteed to be computed at compile time.
    consteval auto operator ""_Hash(ConstPtr<Char> Data, size_t Size)
    {
        return Hash(Data, Size);
    }
}