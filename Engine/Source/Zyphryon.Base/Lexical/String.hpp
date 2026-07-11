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
#include "Zyphryon.Base/Container/Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A mutable string container with optional inline storage.
    ///
    /// When \p Capacity is zero, the string uses heap allocation and grows dynamically.
    /// When \p Capacity is greater than zero, the string uses stack storage with a fixed upper bound.
    template<UInt Capacity>
    class String final
    {
    public:

        /// \brief Constructs an empty string.
        ZY_INLINE constexpr String()
            : mBuffer { }
        {
        }


        /// \brief Constructs a string with pre-allocated storage for \p Length characters.
        ///
        /// \param Length The initial length of the string. Must be less than or equal to \p Capacity.
        ZY_INLINE constexpr String(UInt Length)
            : mBuffer { }
        {
            mBuffer.Reserve(Length);
        }

        /// \brief Constructs a string from a text view.
        ///
        /// \param Content The text to copy into the string.
        ZY_INLINE constexpr String(Text Content)
            : mBuffer { }
        {
            Assign(Content);
        }

        /// \brief Constructs a string with a repeated character.
        ///
        /// \param Character The character to fill the string with.
        ZY_INLINE constexpr String(Char Character)
            : mBuffer { }
        {
            mBuffer.Fill(Character, GetCapacity());
        }

        /// \brief Constructs a string from a null-terminated string literal.
        ///
        /// \param Literal The string literal to copy into the string.
        template<UInt Count>
        ZY_INLINE constexpr String(const Char (& Literal)[Count])
            : String { Text(Literal) }
        {
        }

        /// \brief Copy constructor that creates a new string from another.
        ///
        /// \param Other The string to copy from.
        ZY_INLINE constexpr String(ConstRef<String> Other)
            : mBuffer { Other.mBuffer }
        {
        }

        /// \brief Move constructor that transfers the contents from another string.
        ///
        /// \param Other The string to move from. Left empty after the transfer.
        ZY_INLINE constexpr String(AnyRef<String> Other)
            : mBuffer { Move(Other.mBuffer) }
        {
        }

        /// \brief Returns a pointer to the mutable character data.
        ///
        /// \return A pointer to the first character.
        ZY_INLINE constexpr Ptr<Char> GetData()
        {
            return mBuffer.GetData();
        }

        /// \brief Returns a pointer to the const character data.
        ///
        /// \return A read-only pointer to the first character.
        ZY_INLINE constexpr ConstPtr<Char> GetData() const
        {
            return mBuffer.GetData();
        }

        /// \brief Returns the number of characters in the string.
        ///
        /// \return A count of characters currently in the string.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mBuffer.GetSize();
        }

        /// \brief Returns the maximum number of characters the string can hold without reallocation.
        ///
        /// \return A maximum character count the string can hold without reallocation.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return mBuffer.GetCapacity();
        }

        /// \brief Checks if the string is empty.
        ///
        /// \return `true` if the string contains no characters, `false` otherwise.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mBuffer.IsEmpty();
        }

        /// \brief Checks if the string has reached its maximum capacity.
        ///
        /// \return `true` if the string is at capacity, `false` otherwise.
        ZY_INLINE constexpr Bool IsFull() const
        {
            return mBuffer.GetSize() >= Capacity;
        }

        /// \brief Gets the first element in the string.
        ///
        /// \return A reference to the first element.
        ZY_INLINE constexpr Ref<Char> GetFront()
        {
            return mBuffer.GetFront();
        }

        /// \brief Gets the first element in the string.
        ///
        /// \return A read-only reference to the first element.
        ZY_INLINE constexpr ConstRef<Char> GetFront() const
        {
            return mBuffer.GetFront();
        }

        /// \brief Gets the last element in the string.
        ///
        /// \return A reference to the last element.
        ZY_INLINE constexpr Ref<Char> GetBack()
        {
            return mBuffer.GetBack();
        }

        /// \brief Gets the last element in the string.
        ///
        /// \return A read-only reference to the last element.
        ZY_INLINE constexpr ConstRef<Char> GetBack() const
        {
            return mBuffer.GetBack();
        }

        /// \brief Removes all characters from the string.
        ZY_INLINE constexpr void Clear()
        {
            mBuffer.Clear();
        }

        /// \brief Reserves heap capacity for at least the given number of characters.
        ///
        /// \param Length The minimum number of characters to reserve space for.
        ZY_INLINE constexpr void Reserve(UInt Length)
        {
            if constexpr (Capacity > 0)
            {
                ZY_ASSERT(Capacity >= Length, "Requested reserve length exceeds fixed capacity");
            }
            else
            {
                mBuffer.Reserve(Length);
            }
        }

        /// \brief Appends a single character to the end of the string.
        ///
        /// \param Character The character to append.
        ZY_INLINE constexpr void Append(Char Character)
        {
            mBuffer.Append(Character);
        }

        /// \brief Appends a character repeated the specified number of times to the end of the string.
        ///
        /// \param Character The character to append.
        /// \param Count     The number of times to repeat the character.
        ZY_INLINE constexpr void Append(Char Character, UInt Count)
        {
            mBuffer.Fill(Character, Count);
        }

        /// \brief Appends text to the end of the string.
        ///
        /// \param Content The text to append.
        ZY_INLINE constexpr void Append(Text Content)
        {
            mBuffer.Append(Content);
        }

        /// \brief Appends an integral number to the end of the string.
        ///
        /// \param Number    The number to append.
        /// \param Digits    The number of digits to append.
        /// \param Base      The numerical base to use.
        /// \param Uppercase Whether to use uppercase letters for bases greater than 10.
        template<typename Type>
        ZY_INLINE constexpr void AppendInteger(Type Number, UInt Digits, UInt Base, Bool Uppercase)
            requires (IsIntegral<Type> && !IsAnyOf<Type, Bool>)
        {
            const UInt Offset = mBuffer.GetSize();
            mBuffer.Advance(Digits);

            if (Uppercase)
            {
                for (UInt Digit = Digits; Digit-- > 0; Number /= static_cast<Type>(Base))
                {
                    const UInt Remainder = static_cast<UInt>(Number % static_cast<Type>(Base));

                    mBuffer[Offset + Digit] = "0123456789ABCDEF"[Remainder];
                }
            }
            else
            {
                for (UInt Digit = Digits; Digit-- > 0; Number /= static_cast<Type>(Base))
                {
                    const UInt Remainder = static_cast<UInt>(Number % static_cast<Type>(Base));

                    mBuffer[Offset + Digit] = "0123456789abcdef"[Remainder];
                }
            }
        }

        /// \brief Appends a real number to the end of the string.
        ///
        /// \param Number    The number to append.
        /// \param Precision The number of decimal places to append.
        template<typename Type>
        ZY_INLINE constexpr void AppendReal(Type Number, UInt Precision)
            requires IsReal<Type>
        {
            if (IsNaN(Number))
            {
                Append("NaN");
                return;
            }

            if (IsInf(Number))
            {
                Append("Inf");
                return;
            }

            if (Precision > 0)
            {
                const UInt64 Scale    = Pow10(Precision);
                const UInt64 Scaled   = static_cast<UInt64>(Number * Scale + 0.5);
                const UInt64 Integer  = Scaled / Scale;
                const UInt64 Fraction = Scaled % Scale;

                AppendInteger(Integer, CountDigits<10>(Integer), 10, true);
                Append('.');
                AppendInteger(Fraction, Precision, 10, true);
            }
            else
            {
                const UInt64 Integer = static_cast<UInt64>(::Round(Number));
                AppendInteger(Integer, CountDigits<10>(Integer), 10, true);
            }
        }

        /// \brief Inserts a single character at the specified index.
        ///
        /// \param Index     The index at which to insert the character.
        /// \param Character The character to insert.
        ZY_INLINE constexpr void Insert(UInt Index, Char Character)
        {
            mBuffer.Insert(Index, Character);
        }

        /// \brief Inserts text at the specified index.
        ///
        /// \param Index   The index at which to insert the text.
        /// \param Content The text to insert.
        ZY_INLINE constexpr void Insert(UInt Index, Text Content)
        {
            mBuffer.Insert(Index, Content);
        }

        /// \brief Removes a range of characters from the string.
        ///
        /// \param Index  The starting index of the range to remove.
        /// \param Length The number of characters to remove.
        ZY_INLINE constexpr void Remove(UInt Index, UInt Length)
        {
            mBuffer.Remove(Index, Length);
        }

        /// \brief Removes the last character from the string.
        ZY_INLINE constexpr void RemoveLast()
        {
            mBuffer.RemoveLast();
        }

        /// \brief Removes up to \p Count characters from the end of the string.
        ///
        /// \param Count The maximum number of characters to remove from the end of the string.
        ZY_INLINE constexpr void RemoveLast(UInt Count)
        {
            mBuffer.Remove(mBuffer.GetSize() - Count, Count);
        }

        /// \brief Shrinks the heap capacity to match the current size.
        ZY_INLINE constexpr void Shrink()
            requires (Capacity == 0)
        {
            mBuffer.Shrink();
        }

        /// \brief Replaces all occurrences of one character with another.
        ///
        /// \param Search  The character to search for.
        /// \param Replace The character to replace with.
        /// \return A count of characters that were replaced.
        ZY_INLINE constexpr UInt Replace(Char Search, Char Replace)
        {
            UInt Count = 0;

            for (UInt Index = 0, Limit = GetSize(); Index < Limit; ++Index)
            {
                if (mBuffer[Index] == Search)
                {
                    mBuffer[Index] = Replace;
                    ++Count;
                }
            }
            return Count;
        }

        /// \brief Reverses the order of characters in the string.
        ZY_INLINE constexpr void Reverse()
        {
            UInt Left  = 0;
            UInt Right = GetSize() - 1;

            while (Left < Right)
            {
                Swap(mBuffer[Left++], mBuffer[Right--]);
            }
        }

        /// \brief Creates a subview of this string starting at the given offset.
        ///
        /// \param Offset The starting position of the subview.
        /// \param Count  The number of characters to include. If zero, includes all remaining characters.
        /// \return A new text view representing the subview.
        ZY_INLINE constexpr Text Slice(UInt Offset, UInt Count = 0u) const
        {
            ZY_ASSERT(Offset <= mBuffer.GetSize(), "Slice offset exceeds string size");

            const UInt Length = (Count == 0u ? mBuffer.GetSize() - Offset : Count);
            ZY_ASSERT(Offset + Length <= mBuffer.GetSize(), "Slice range exceeds string size");

            return Text(mBuffer.GetData() + Offset, Length);
        }

        /// \brief Computes a 64-bit hash of the string.
        ///
        /// Uses FNV-1a at compile time and the engine's runtime hash otherwise.
        ///
        /// \return A 64-bit hash of the character data, or zero if the view is empty.
        ZY_INLINE constexpr UInt64 Hash() const
        {
            if (const UInt Size = GetSize(); Size > 0)
            {
                return ::Hash(mBuffer.GetData(), Size);
            }
            return 0;
        }

        /// \brief Accesses a character at the specified index.
        ///
        /// \param Index The zero-based index of the character.
        /// \return A reference to the character at the specified index.
        ZY_INLINE constexpr Ref<Char> operator[](UInt Index)
        {
            return mBuffer[Index];
        }

        /// \brief Accesses a character at the specified index.
        ///
        /// \param Index The zero-based index of the character.
        /// \return A copy of the character at the specified index.
        ZY_INLINE constexpr Char operator[](UInt Index) const
        {
            return mBuffer[Index];
        }

        /// \brief Checks if this string is equal to the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if the strings are equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(Text Other) const
        {
            return GetSize() == Other.GetSize() && ::Compare(GetData(), Other.GetData(), GetSize());
        }

        /// \brief Checks if this string is equal to the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if the strings are equal, `false` otherwise.
        template<UInt Count>
        ZY_INLINE constexpr Bool operator==(const Char (& Literal)[Count]) const
        {
            return GetSize() == Count - 1 && ::Compare(GetData(), Literal, GetSize());
        }

        /// \brief Checks if this string is equal to another string.
        ///
        /// \param Other The string to compare with.
        /// \return `true` if the strings are equal, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<String> Other) const
        {
            return (* this) == Text(Other);
        }

        /// \brief Checks if this string is lexicographically less than the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if this string is less than the given text, `false` otherwise.
        ZY_INLINE constexpr Bool operator<(Text Other) const
        {
            return Compare(Other) < 0;
        }

        /// \brief Checks if this string is lexicographically less than or equal to the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if this string is less than or equal to the given text, `false` otherwise.
        ZY_INLINE constexpr Bool operator<=(Text Other) const
        {
            return Compare(Other) <= 0;
        }

        /// \brief Checks if this string is lexicographically greater than the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if this string is greater than the given text, `false` otherwise.
        ZY_INLINE constexpr Bool operator>(Text Other) const
        {
            return Compare(Other) > 0;
        }

        /// \brief Checks if this string is lexicographically greater than or equal to the given text.
        ///
        /// \param Other The text to compare with.
        /// \return `true` if this string is greater than or equal to the given text, `false` otherwise.
        ZY_INLINE constexpr Bool operator>=(Text Other) const
        {
            return Compare(Other) >= 0;
        }

        /// \brief Assigns a single character to the string, replacing its entire contents.
        ///
        /// \param Character The character to assign.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator=(Char Character)
        {
            mBuffer.Clear();
            mBuffer.Append(Character);
            return (* this);
        }

        /// \brief Assigns text to the string, replacing its entire contents.
        ///
        /// \param Content The text to assign.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator=(Text Content)
        {
            Assign(Content);
            return (* this);
        }

        /// \brief Assigns a null-terminated string literal to the string, replacing its entire contents.
        ///
        /// \param Literal The string literal to assign.
        /// \return A reference to this string.
        template<UInt Count>
        ZY_INLINE constexpr Ref<String> operator=(const Char (& Literal)[Count])
        {
            Assign(Literal);
            return (* this);
        }

        /// \brief Copy assignment operator that replaces the contents with another string.
        ///
        /// \param Other The string to copy from.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator=(ConstRef<String> Other)
        {
            mBuffer = Other.mBuffer;
            return (* this);
        }

        /// \brief Move assignment operator that transfers the contents from another string.
        ///
        /// \param Other The string to move from. Left empty after the transfer.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator=(AnyRef<String> Other)
        {
            mBuffer = Move(Other.mBuffer);
            return (* this);
        }

        /// \brief Appends a character to the end of the string.
        ///
        /// \param Character The character to append.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator+=(Char Character)
        {
            Append(Character);
            return (* this);
        }

        /// \brief Appends text to the end of the string.
        ///
        /// \param Content The text to append.
        /// \return A reference to this string.
        ZY_INLINE constexpr Ref<String> operator+=(Text Content)
        {
            Append(Content);
            return (* this);
        }

    public:

        /// \brief Joins two pieces of text with a delimiter in between.
        ///
        /// \param Left      The text to appear before the delimiter.
        /// \param Delimiter The character to insert between the two pieces of text.
        /// \param Right     The text to appear after the delimiter.
        /// \return A new string containing the joined text.
        ZY_INLINE static constexpr String Join(Text Left, Char Delimiter, Text Right)
        {
            String Buffer;
            Buffer.Reserve(Left.GetSize() + 1 + Right.GetSize());

            Buffer.Append(Left);
            Buffer.Append(Delimiter);
            Buffer.Append(Right);

            return Buffer;
        }

        /// \brief Creates a null-terminated string from the given text.
        ///
        /// \param Content The text to copy into the returned string.
        /// \return A string that contains \p Content followed by a trailing null character.
        ZY_INLINE static constexpr String CStr(Text Content)
        {
            String Buffer(Content);
            Buffer.Append('\0');
            return Buffer;
        }

    private:

        /// \brief Assigns text to the string, replacing its entire contents.
        ///
        /// \param Content The text to assign.
        ZY_INLINE constexpr void Assign(Text Content)
        {
            mBuffer.Clear();

            if (!Content.IsEmpty())
            {
                mBuffer.Append(Content);
            }
        }

        /// \brief Performs three-way lexicographic comparison with the given text.
        ///
        /// \param Other The text to compare with.
        /// \return A negative value if this string is less than \p Other, zero if equal, positive if greater.
        ZY_INLINE constexpr SInt Compare(Text Other) const
        {
            for (UInt Index = 0, Limit = Min(GetSize(), Other.GetSize()); Index < Limit; ++Index)
            {
                if (const SInt Diff = mBuffer[Index] - Other[Index]; Diff != 0)
                {
                    return Diff;
                }
            }
            return static_cast<SInt>(GetSize()) - static_cast<SInt>(Other.GetSize());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Char, Capacity> mBuffer;
    };

    /// \brief A heap-backed string with dynamic length and no inline storage limit.
    using Str   = String<0>;

    /// \brief A stack-backed string with a fixed capacity of 15 characters.
    using Str16 = String<15>;

    /// \brief A stack-backed string with a fixed capacity of 31 characters.
    using Str32 = String<31>;

    /// \brief A stack-backed string with a fixed capacity of 47 characters.
    using Str48 = String<47>;

    /// \brief A stack-backed string with a fixed capacity of 63 characters.
    using Str64 = String<63>;
}