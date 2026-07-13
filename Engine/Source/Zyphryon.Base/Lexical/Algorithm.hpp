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
    /// \brief Calculates the length of the given string, in characters.
    ///
    /// \param Value The string to calculate the length of.
    /// \return The length of the string, in characters.
    template<typename Type = Char>
    constexpr UInt StrLength(ConstPtr<Type> Value)
    {
        UInt Result = 0;

        while (Value[Result] != '\0')
        {
            ++Result;
        }
        return Result;
    }

    /// \brief Converts a null-terminated string to a text view.
    ///
    /// \param Value The null-terminated string to convert.
    /// \return A Text view referencing the input string.
    constexpr Text StrConvert(ConstPtr<Char> Value)
    {
        return Text(Value, StrLength(Value));
    }

    /// \brief Converts a null-terminated wide string to a text view.
    ///
    /// \param Value The null-terminated wide string to convert.
    /// \return A span view referencing the input wide string.
    constexpr ConstSpan<Wide> StrConvert(ConstPtr<Wide> Value)
    {
        return ConstSpan(Value, StrLength(Value));
    }

    /// \brief Counts the number of occurrences of \p Search within \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The character to count.
    /// \return A count of how many times \p Search appears in \p Value.
    constexpr UInt StrCount(Text Value, Char Search)
    {
        return Count(Value.GetData(), Value.GetSize(), Search);
    }

    /// \brief Checks whether \p Value begins with the given prefix.
    ///
    /// \param Value  The text to examine.
    /// \param Prefix The prefix to search for.
    /// \return `true` if \p Value starts with \p Prefix, `false` otherwise.
    constexpr Bool StrStartsWith(Text Value, Text Prefix)
    {
        if (Prefix.GetSize() > Value.GetSize())
        {
            return false;
        }
        return Compare(Value.GetData(), Prefix.GetData(), Prefix.GetSize());
    }

    /// \brief Checks whether \p Value ends with the given suffix.
    ///
    /// \param Value  The text to examine.
    /// \param Suffix The suffix to search for.
    /// \return `true` if \p Value ends with \p Suffix, `false` otherwise.
    constexpr Bool StrEndsWith(Text Value, Text Suffix)
    {
        if (Suffix.GetSize() > Value.GetSize())
        {
            return false;
        }

        const UInt Offset = Value.GetSize() - Suffix.GetSize();
        return Compare(Value.GetData() + Offset, Suffix.GetData(), Suffix.GetSize());
    }

    /// \brief Returns the index of the first occurrence of a character in \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The character to look for.
    /// \return A zero-based index of the first match, or `-1` if not found.
    constexpr SInt StrFind(Text Value, Char Search)
    {
        const ConstPtr<Char> Result = Find(Value.GetData(), Value.GetSize(), Search);
        return Result ? Result - Value.GetData() : -1;
    }

    /// \brief Returns the index of the first occurrence of \p Search within \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The substring to look for.
    /// \return A zero-based index of the first match, or `-1` if not found.
    constexpr SInt StrFind(Text Value, Text Search)
    {
        if (Search.IsEmpty())
        {
            return 0;
        }
        if (Search.GetSize() > Value.GetSize())
        {
            return -1;
        }

        const ConstPtr<Char> Head  = Search.GetData();
        const UInt           Size  = Search.GetSize();
        const ConstPtr<Char> Limit = Value.GetData() + Value.GetSize() - Size + 1;

        ConstPtr<Char> Cursor = Value.GetData();

        while ((Cursor = Find(Cursor, static_cast<UInt>(Limit - Cursor), Head[0])) != nullptr)
        {
            if (Compare(Cursor, Head, Size))
            {
                return Cursor - Value.GetData();
            }
            if (++Cursor >= Limit)
            {
                break;
            }
        }
        return -1;
    }

    /// \brief Returns the index of the first occurrence of any character from \p Search in \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The set of characters to look for.
    /// \param Start  The starting position to search from (default 0).
    /// \return A zero-based index of the first match, or `-1` if not found.
    constexpr SInt StrFindAny(Text Value, Text Search, UInt Start = 0)
    {
        if (Search.IsEmpty() || Start >= Value.GetSize())
        {
            return -1;
        }

        const UInt Size  = Value.GetSize();
        const UInt Count = Search.GetSize();

        for (UInt Index = Start; Index < Size; ++Index)
        {
            const Char Current = Value[Index];

            for (UInt J = 0; J < Count; ++J)
            {
                if (Current == Search[J])
                {
                    return static_cast<SInt>(Index);
                }
            }
        }
        return -1;
    }

    /// \brief Returns the index of the last occurrence of a character in \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The character to look for.
    /// \return A zero-based index of the last match, or `-1` if not found.
    constexpr SInt StrFindLast(Text Value, Char Search)
    {
        const ConstPtr<Char> Result = FindLast(Value.GetData(), Value.GetSize(), Search);
        return Result ? Result - Value.GetData() : -1;
    }

    /// \brief Returns the index of the last occurrence of a character in \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The substring to look for.
    /// \return A zero-based index of the first match, or `-1` if not found.
    constexpr SInt StrFindLast(Text Value, Text Search)
    {
        if (Search.IsEmpty())
        {
            return static_cast<SInt>(Value.GetSize());
        }
        if (Search.GetSize() > Value.GetSize())
        {
            return -1;
        }

        const ConstPtr<Char> Head  = Search.GetData();
        const UInt           Size  = Search.GetSize();

        ConstPtr<Char> Cursor = Value.GetData() + Value.GetSize() - Size;

        while ((Cursor = FindLast(Cursor, static_cast<UInt>(Cursor - Value.GetData() + 1), Head[0])) != nullptr)
        {
            if (Compare(Cursor, Head, Size))
            {
                return Cursor - Value.GetData();
            }

            if (Cursor == Value.GetData())
            {
                break;
            }
            --Cursor;
        }
        return -1;
    }

    /// \brief Returns a sub-view with leading whitespace removed.
    ///
    /// \param Value The text to trim.
    /// \return A sub-view of \p Value with no leading whitespace characters.
    constexpr Text StrTrimLeft(Text Value)
    {
        UInt Start = 0;

        while (Start < Value.GetSize() && (Value[Start] == ' ' || Value[Start] == '\t'))
        {
            ++Start;
        }
        return Value.Slice(Start);
    }

    /// \brief Returns a sub-view with trailing whitespace removed.
    ///
    /// \param Value The text to trim.
    /// \return A sub-view of \p Value with no trailing whitespace characters.
    constexpr Text StrTrimRight(Text Value)
    {
        UInt End = Value.GetSize();

        while (End > 0 && (Value[End - 1] == ' ' || Value[End - 1] == '\t'))
        {
            --End;
        }
        return Value.Slice(0, End);
    }

    /// \brief Returns a sub-view with both leading and trailing whitespace removed.
    ///
    /// \param Value The text to trim.
    /// \return A sub-view of \p Value with no surrounding whitespace characters.
    constexpr Text StrTrim(Text Value)
    {
        return StrTrimRight(StrTrimLeft(Value));
    }

    /// \brief Checks whether \p Search appears anywhere within \p Value.
    ///
    /// \param Value  The text to search within.
    /// \param Search The substring to look for.
    /// \return `true` if \p Search is found inside \p Value, `false` otherwise.
    constexpr Bool StrContains(Text Value, Text Search)
    {
        return StrFind(Value, Search) != -1;
    }

    /// \brief Splits \p Value at the first occurrence of \p Delimiter and returns the part before it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value     The text to split.
    /// \param Delimiter The character at which to split.
    /// \return A sub-view of \p Value up to (but not including) the first \p Delimiter.
    constexpr Text StrBefore(Text Value, Char Delimiter)
    {
        const SInt Index = StrFind(Value, Delimiter);
        return (Index != -1) ? Value.Slice(0, static_cast<UInt>(Index)) : Text::Empty();
    }

    /// \brief Splits \p Value at the first occurrence of \p Search and returns the part before it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value  The text to split.
    /// \param Search The substring to search for.
    /// \return A sub-view of \p Value up to (but not including) the first \p Search.
    constexpr Text StrBefore(Text Value, Text Search)
    {
        const SInt Index = StrFind(Value, Search);
        return (Index != -1) ? Value.Slice(0, static_cast<UInt>(Index)) : Text::Empty();
    }

    /// \brief Splits \p Value at the last occurrence of \p Delimiter and returns the part before it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value     The text to split.
    /// \param Delimiter The character at which to split.
    /// \return A sub-view of \p Value up to (but not including) the last \p Delimiter.
    constexpr Text StrBeforeLast(Text Value, Char Delimiter)
    {
        const SInt Index = StrFindLast(Value, Delimiter);
        return (Index != -1) ? Value.Slice(0, static_cast<UInt>(Index)) : Text::Empty();
    }

    /// \brief Splits \p Value at the last occurrence of \p Search and returns the part before it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value  The text to split.
    /// \param Search The substring to search for.
    /// \return A sub-view of \p Value up to (but not including) the last \p Search.
    constexpr Text StrBeforeLast(Text Value, Text Search)
    {
        const SInt Index = StrFindLast(Value, Search);
        return (Index != -1) ? Value.Slice(0, static_cast<UInt>(Index)) : Text::Empty();
    }

    /// \brief Splits \p Value at the first occurrence of \p Delimiter and returns the part after it.
    ///
    /// \note If \p Delimiter is not found an empty view is returned.
    ///
    /// \param Value     The text to split.
    /// \param Delimiter The character at which to split.
    /// \return A sub-view of \p Value after the first \p Delimiter, or an empty view if not found.
    constexpr Text StrAfter(Text Value, Char Delimiter)
    {
        const SInt Index = StrFind(Value, Delimiter);
        return (Index != -1) ? Value.Slice(static_cast<UInt>(Index) + 1) : Text::Empty();
    }

    /// \brief Splits \p Value at the first occurrence of \p Search and returns the part after it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value  The text to split.
    /// \param Search The substring to search for.
    /// \return A sub-view of \p Value after the first \p Search, or an empty view if not found.
    constexpr Text StrAfter(Text Value, Text Search)
    {
        const SInt Index = StrFind(Value, Search);
        return (Index != -1) ? Value.Slice(static_cast<UInt>(Index) + Search.GetSize()) : Text::Empty();
    }

    /// \brief Splits \p Value at the last occurrence of \p Delimiter and returns the part after it.
    ///
    /// \note If \p Delimiter is not found an empty view is returned.
    ///
    /// \param Value     The text to split.
    /// \param Delimiter The character at which to split.
    /// \return A sub-view of \p Value after the last \p Delimiter, or an empty view if not found.
    constexpr Text StrAfterLast(Text Value, Char Delimiter)
    {
        const SInt Index = StrFindLast(Value, Delimiter);
        return (Index != -1) ? Value.Slice(static_cast<UInt>(Index) + 1) : Text::Empty();
    }

    /// \brief Splits \p Value at the last occurrence of \p Search and returns the part after it.
    ///
    /// \note If \p Search is not found an empty view is returned.
    ///
    /// \param Value  The text to split.
    /// \param Search The substring to search for.
    /// \return A sub-view of \p Value after the last \p Search, or an empty view if not found.
    constexpr Text StrAfterLast(Text Value, Text Search)
    {
        const SInt Index = StrFindLast(Value, Search);
        return (Index != -1) ? Value.Slice(static_cast<UInt>(Index) + Search.GetSize()) : Text::Empty();
    }

    /// \brief Checks whether two text views are equal.
    ///
    /// \param Left  The first text to compare.
    /// \param Right The second text to compare.
    /// \return `true` if both views contain the same characters regardless of case, `false` otherwise.
    constexpr Bool StrEqualCase(Text Left, Text Right)
    {
        if (Left.GetSize() != Right.GetSize())
        {
            return false;
        }

        for (UInt Index = 0; Index < Left.GetSize(); ++Index)
        {
            if (Left[Index] != Right[Index])
            {
                return false;
            }
        }
        return true;
    }

    /// \brief Checks whether two text views are equal using case-insensitive comparison.
    ///
    /// \param Left  The first text to compare.
    /// \param Right The second text to compare.
    /// \return `true` if both views contain the same characters regardless of case, `false` otherwise.
    constexpr Bool StrEqualCaseInsensitive(Text Left, Text Right)
    {
        if (Left.GetSize() != Right.GetSize())
        {
            return false;
        }

        for (UInt Index = 0; Index < Left.GetSize(); ++Index)
        {
            if ((Left[Index] | 0x20) != (Right[Index] | 0x20))
            {
                return false;
            }
        }
        return true;
    }

    /// \brief Invokes \p Callback with a `Text` view for each token in \p Value delimited by \p Delimiter.
    ///
    /// \note Tokens are yielded left to right without allocation. If \p Callback returns `false` iteration stops early.
    ///
    /// \param Value     The text to split.
    /// \param Delimiter The character used as the token boundary.
    /// \param Callback  The callable invoked with each token. Must be invocable with a single `Text` argument and return a value convertible to `Bool`.
    template<typename Callable>
    constexpr void StrSplit(Text Value, Char Delimiter, AnyRef<Callable> Callback)
    {
        Text Remaining = Value;

        while (!Remaining.IsEmpty())
        {
            const SInt Index = StrFind(Remaining, Delimiter);

            if (Index == -1)
            {
                Callback(Remaining);
                break;
            }

            if (!Callback(Remaining.Slice(0, static_cast<UInt>(Index))))
            {
                break;
            }

            Remaining = Remaining.Slice(static_cast<UInt>(Index) + 1);
        }
    }

    /// \brief Converts a character to lowercase.
    ///
    /// \param Character The character to convert.
    /// \return The lowercase equivalent of the character, or the character unchanged if not an uppercase letter.
    constexpr Char StrLowercase(Char Character)
    {
        return (Character >= 'A' && Character <= 'Z') ? static_cast<Char>(Character | 0x20) : Character;
    }

    /// \brief Converts a character to uppercase.
    ///
    /// \param Character The character to convert.
    /// \return The uppercase equivalent of the character, or the character unchanged if not a lowercase letter.
    constexpr Char StrUppercase(Char Character)
    {
        return (Character >= 'a' && Character <= 'z') ? static_cast<Char>(Character & ~0x20) : Character;
    }

    /// \brief Checks whether a character is a digit (0-9).
    ///
    /// \param Character The character to check.
    /// \return `true` if the character is a digit, `false` otherwise.
    constexpr Bool StrIsDigit(Char Character)
    {
        return Character >= '0' && Character <= '9';
    }

    /// \brief Checks whether a character is a whitespace character.
    ///
    /// \param Character The character to check.
    /// \return `true` if the character is a whitespace character, `false` otherwise.
    constexpr Bool StrIsSpace(Char Character)
    {
        return Character == ' '  ||
               Character == '\t' ||
               Character == '\n' ||
               Character == '\r' ||
               Character == '\v' ||
               Character == '\f';
    }

    /// \brief Checks whether a character is an alphabetic letter (A-Z, a-z).
    ///
    /// \param Character The character to check.
    /// \return `true` if the character is an alphabetic letter, `false` otherwise.
    constexpr bool StrIsAlphabetic(Char Character)
    {
        return (Character >= 'A' && Character <= 'Z') || (Character >= 'a' && Character <= 'z');
    }

    /// \brief Checks whether a character is alphanumeric (A-Z, a-z, 0-9).
    ///
    /// \param Character The character to check.
    /// \return `true` if the character is alphanumeric, `false` otherwise.
    constexpr bool StrIsAlphanumeric(Char Character)
    {
        return StrIsAlphabetic (Character) || StrIsDigit(Character);
    }

    /// \brief Skips characters in \p Content starting at \p Cursor while they match \p Predicate.
    ///
    /// \param Content  The text to scan.
    /// \param Cursor   The current position, advanced past matching characters.
    /// \param Predicate A callable taking a Char and returning Bool.
    /// \return The number of characters skipped.
    template<typename Callable>
    constexpr UInt StrSkip(Text Content, Ref<UInt> Cursor, AnyRef<Callable> Predicate)
    {
        const UInt Start = Cursor;

        while (Cursor < Content.GetSize() && Predicate(Content[Cursor]))
        {
            ++Cursor;
        }
        return Cursor - Start;
    }

    /// \brief Skips whitespace characters (space, tab) starting at \p Cursor.
    ///
    /// \param Content The text to scan.
    /// \param Cursor  The current position, advanced past whitespace.
    /// \return The number of whitespace characters skipped.
    constexpr UInt StrSkipWhitespace(Text Content, Ref<UInt> Cursor)
    {
        return StrSkip(Content, Cursor, [](Char Character)
        {
            return Character == ' ' || Character == '\t' || Character == '\n' || Character == '\r';
        });
    }

    /// \brief Consumes a single expected character from the text, advancing the cursor if matched.
    ///
    /// \param Content  The text to read from.
    /// \param Cursor   The current position, advanced by 1 if the character matches.
    /// \param Expected The character to match.
    /// \return `true` if the character was found and consumed, `false` otherwise.
    constexpr Bool StrConsume(Text Content, Ref<UInt> Cursor, Char Expected)
    {
        if (Content.GetSize() > Cursor && Content[Cursor] == Expected)
        {
            ++Cursor;
            return true;
        }
        return false;
    }

    /// \brief Extracts a single character from the text at the current cursor position.
    ///
    /// \param Content The text to read from.
    /// \param Cursor  The current position, advanced by 1 if a character was extracted.
    /// \return The extracted character, or '\0' if the cursor is at the end of the text.
    constexpr Char StrExtractChar(Text Content, Ref<UInt> Cursor)
    {
        Char Result = '\0';

        if (Cursor < Content.GetSize())
        {
            Result = Content[Cursor];
            ++Cursor;
        }
        return Result;
    }

    /// \brief Extracts an integral number from the text at the current cursor position.
    ///
    /// \note Parses digits in the specified base, optionally handling a leading minus sign for signed types.
    ///
    /// \tparam Base   The numeric base to use for parsing (default is 10).
    /// \param Content The text to parse from.
    /// \param Cursor  The current position, advanced past the consumed digits.
    /// \return The extracted integral value.
    template<UInt Base = 10, typename Type>
    constexpr Type StrExtractNumber(Text Content, Ref<UInt> Cursor)
        requires (IsIntegral<Type>)
    {
        constexpr UInt kMaxDigits = CountDigits<Base, Type>(kMaximum<Type>);

        Bool IsNegative = false;

        if constexpr (IsSigned<Type>)
        {
            IsNegative = StrConsume(Content, Cursor, '-');
        }

        Type Result = 0;
        UInt Taken  = 0;

        while (Cursor < Content.GetSize() && Taken < kMaxDigits)
        {
            const Char  Character = Content[Cursor];
            UInt        Digit;

            if (Character >= '0' && Character <= '9')
            {
                Digit = Character - '0';
            }
            else if (Character >= 'a' && Character <= 'f')
            {
                Digit = Character - 'a' + 10;
            }
            else if (Character >= 'A' && Character <= 'F')
            {
                Digit = Character - 'A' + 10;
            }
            else
            {
                break;
            }

            if (Digit < Base)
            {
                Result = Result * Base + Digit;
                ++Cursor;
                ++Taken;
            }
            else
            {
                break;
            }
        }

        if constexpr (IsSigned<Type>)
        {
            if (IsNegative)
            {
                Result = -Result;
            }
        }
        return Result;
    }

    /// \brief Extracts a boolean value from the text at the current cursor position.
    ///
    /// Recognizes "0", "1", "true" (case-insensitive) and "false" (case-insensitive).
    ///
    /// \param Content The text to parse from.
    /// \param Cursor  The current position, advanced past the consumed characters if matched.
    /// \return The extracted boolean value, or `false` if no match is found.
    constexpr Bool StrExtractBool(Text Content, Ref<UInt> Cursor)
    {
        switch (StrExtractChar(Content, Cursor))
        {
        case '0':
            return false;
        case '1':
            return true;
        case 't': case 'T':
            if (Content.GetSize() >= Cursor + 3
                && (Content[Cursor    ] | 0x20) == 'r'
                && (Content[Cursor + 1] | 0x20) == 'u'
                && (Content[Cursor + 2] | 0x20) == 'e')
            {
                Cursor += 3;
                return true;
            }
            break;

        case 'f': case 'F':
            if (Content.GetSize() >= Cursor + 4
                && (Content[Cursor    ] | 0x20) == 'a'
                && (Content[Cursor + 1] | 0x20) == 'l'
                && (Content[Cursor + 2] | 0x20) == 's'
                && (Content[Cursor + 3] | 0x20) == 'e')
            {
                Cursor += 4;
                return false;
            }
            break;
        default:
            break;
        }
        --Cursor;
        return false;
    }

    /// \brief Extracts text from the current cursor position up to a delimiter.
    ///
    /// \param Content   The text to parse from.
    /// \param Cursor    The current position, advanced past the extracted text and delimiter.
    /// \param Delimiter The character that terminates the extraction.
    /// \return The extracted text view.
    constexpr Text StrExtractText(Text Content, Ref<UInt> Cursor, Char Delimiter)
    {
        const Text Result = StrBefore(Content.Slice(Cursor), Delimiter);
        Cursor += Result.GetSize();
        return Result;
    }

    /// \brief Extracts a floating-point number from the text at the current cursor position.
    ///
    /// \tparam Type   The floating-point type to extract.
    /// \param Content The text to parse from.
    /// \param Cursor  The current position, advanced past the consumed characters.
    /// \return The extracted floating-point value.
    template<typename Type>
    constexpr Type StrExtractNumber(Text Content, Ref<UInt> Cursor)
        requires (IsReal<Type>)
    {
        Type Result = 0.0f;

        const SInt64 IntPart = StrExtractNumber<10, SInt64>(Content, Cursor);

        if (StrConsume(Content, Cursor, '.'))
        {
            const UInt   Start   = Cursor;
            const UInt64 IntFrac = StrExtractNumber<10, UInt64>(Content, Cursor);
            Result = static_cast<Type>(IntFrac) / static_cast<Type>(Pow10(Cursor - Start));
        }
        return static_cast<Type>(IntPart) + (IntPart < 0 ? -Result : Result);
    }

    /// \brief Extracts a UTF-8 codepoint from the text at the current cursor position.
    ///
    /// \param Content The text to parse from.
    /// \param Cursor  The current position, advanced past the consumed bytes.
    /// \return The extracted Unicode codepoint, or 0 if the sequence is invalid.
    constexpr UInt32 StrExtractUTF8(Text Content, Ref<UInt> Cursor)
    {
        const Char First = Content[Cursor++];

        if ((First & 0x80) == 0)
        {
            return First;
        }

        if ((First & 0xE0) == 0xC0)
        {
            const Char B1 = Content[Cursor++];
            return ((First & 0x1F) << 6) | (B1 & 0x3F);
        }

        if ((First & 0xF0) == 0xE0)
        {
            const Char B1 = Content[Cursor++];
            const Char B2 = Content[Cursor++];
            return ((First & 0x0F) << 12) | ((B1 & 0x3F) << 6) | (B2 & 0x3F);
        }

        if ((First & 0xF8) == 0xF0)
        {
            const Char B1 = Content[Cursor++];
            const Char B2 = Content[Cursor++];
            const Char B3 = Content[Cursor++];
            return ((First & 0x07) << 18) | ((B1 & 0x3F) << 12) | ((B2 & 0x3F) << 6) | (B3 & 0x3F);
        }
        return 0;
    }

    /// \brief Iterates over each UTF-8 codepoint in the text and invokes the callback.
    ///
    /// \param Content  The UTF-8 text to iterate over.
    /// \param Callback A callable invoked for each decoded codepoint.
    template<typename Callable>
    constexpr void StrIterateUTF8(Text Content, AnyRef<Callable> Callback)
    {
        UInt Cursor = 0;

        while (Cursor < Content.GetSize())
        {
            const UInt32 Codepoint = StrExtractUTF8(Content, Cursor);
            Callback(Codepoint);
        }
    }

    /// \brief Converts a UTF-8 text view to a UTF-16 sequence.
    ///
    /// \note Decodes each UTF-8 codepoint from \p Content and encodes it as one or two UTF-16 code units.
    ///
    /// \param Content The UTF-8 text to convert.
    /// \return A sequence of UTF-16 code units representing the converted text.
    template<UInt Capacity = 0>
    constexpr Sequence<Wide, Capacity> StrConvertUTF16(Text Content)
    {
        Sequence<Wide, Capacity> Result;

        if constexpr (Capacity == 0)
        {
            Result.Reserve(Content.GetSize() + 1);
        }

        for (UInt Index = 0; Index < Content.GetSize(); )
        {
            if (UInt32 Codepoint = StrExtractUTF8(Content, Index); Codepoint <= 0xFFFF)
            {
                Result.Append(static_cast<Wide>(Codepoint));
            }
            else
            {
                Codepoint -= 0x10000;
                Result.Append(static_cast<Wide>(0xD800 | (Codepoint >> 10)));
                Result.Append(static_cast<Wide>(0xDC00 | (Codepoint & 0x3FF)));
            }
        }

        Result.GetData()[Result.GetSize()] = '\0';
        return Result;
    }
}