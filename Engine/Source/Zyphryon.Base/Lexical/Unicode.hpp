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

#include "Algorithm.hpp"
#include "Zyphryon.Base/Container/Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    template<UInt Capacity>
    class String;

    /// \brief Represents the UTF-8 encoding: decoding and encoding codepoints, and stepping through text by character.
    ///
    /// \note Every offset is a byte of the text, and every step lands on the first byte of a character.
    class Unicode final
    {
    public:

        /// \brief Decodes the codepoint at a cursor, moving the cursor past it.
        ///
        /// \param Content The text to decode from.
        /// \param Cursor  The offset of the codepoint, advanced past the bytes it takes.
        /// \return The codepoint, or `0` when the sequence is broken, in which case one byte is consumed.
        ZY_INLINE static constexpr UInt32 Decode(Text Content, Ref<UInt> Cursor)
        {
            const UInt8 First = static_cast<UInt8>(Content[Cursor++]);

            if (First < 0x80)
            {
                return First;
            }

            const UInt Length = GetSize(static_cast<Char>(First));

            // A stray continuation byte, or a sequence the end of the text cuts short, is never read past.
            if (Length == 0 || Cursor + Length - 1 > Content.GetSize())
            {
                return 0;
            }

            UInt32 Codepoint = First & (0x7F >> Length);

            for (UInt Index = 0; Index < Length - 1; ++Index)
            {
                const Char Next = Content[Cursor + Index];

                if (!IsContinuation(Next))
                {
                    return 0;
                }
                Codepoint = (Codepoint << 6) | (static_cast<UInt8>(Next) & 0x3F);
            }

            Cursor += Length - 1;
            return Codepoint;
        }

        /// \brief Decodes the codepoint at an offset, without moving past it.
        ///
        /// \param Content The text to decode from.
        /// \param Offset  The offset of the codepoint, which must stand before the end of the text.
        /// \return The codepoint, or `0` when the sequence is broken.
        ZY_INLINE static constexpr UInt32 Peek(Text Content, UInt Offset)
        {
            return Decode(Content, Offset);
        }

        /// \brief Encodes a codepoint as UTF-8.
        ///
        /// \param Codepoint The codepoint to encode.
        /// \param Output    Receives the bytes, with room for as many as `GetLength` gives.
        /// \return The number of bytes written, from one to four.
        ZY_INLINE static constexpr UInt Encode(UInt32 Codepoint, Ptr<Char> Output)
        {
            const UInt Length = GetLength(Codepoint);

            switch (Length)
            {
            case 1:
            {
                Output[0] = static_cast<Char>(Codepoint);
                break;
            }
            case 2:
            {
                Output[0] = static_cast<Char>(0xC0 | (Codepoint >> 6));
                Output[1] = static_cast<Char>(0x80 | (Codepoint & 0x3F));
                break;
            }
            case 3:
            {
                Output[0] = static_cast<Char>(0xE0 |  (Codepoint >> 12));
                Output[1] = static_cast<Char>(0x80 | ((Codepoint >>  6) & 0x3F));
                Output[2] = static_cast<Char>(0x80 |  (Codepoint        & 0x3F));
                break;
            }
            default:
            {
                Output[0] = static_cast<Char>(0xF0 |  (Codepoint >> 18));
                Output[1] = static_cast<Char>(0x80 | ((Codepoint >> 12) & 0x3F));
                Output[2] = static_cast<Char>(0x80 | ((Codepoint >>  6) & 0x3F));
                Output[3] = static_cast<Char>(0x80 |  (Codepoint        & 0x3F));
                break;
            }
            }
            return Length;
        }

        /// \brief Decodes each codepoint of a text in turn.
        ///
        /// \param Content  The text to decode.
        /// \param Callback The callable invoked with each codepoint.
        template<typename Callable>
        ZY_INLINE static constexpr void Iterate(Text Content, AnyRef<Callable> Callback)
        {
            UInt Cursor = 0;

            while (Cursor < Content.GetSize())
            {
                Callback(Decode(Content, Cursor));
            }
        }

        /// \brief Converts a text to a null-terminated sequence of UTF-16 code units.
        ///
        /// \note Encodes each codepoint of \p Content as one or two UTF-16 code units. A fixed-capacity result spends
        ///       one slot on the terminator, so at most `Capacity - 1` code units of text fit. Content that would not
        ///       fit yields an empty sequence rather than a truncated one: a shortened path still names a real,
        ///       different file, and silently acting on the wrong one is worse than failing.
        ///
        /// \param Content The text to convert.
        /// \return A sequence of UTF-16 code units, or an empty sequence when \p Content does not fit.
        template<UInt Capacity = 0>
        ZY_INLINE static constexpr Sequence<Wide, Capacity> Widen(Text Content)
        {
            Sequence<Wide, Capacity> Result;

            if constexpr (Capacity == 0)
            {
                Result.Reserve(Content.GetSize() + 1);
            }

            for (UInt Cursor = 0; Cursor < Content.GetSize();)
            {
                UInt32     Codepoint = Decode(Content, Cursor);
                const UInt Units     = (Codepoint <= 0xFFFF) ? 1 : 2;

                if constexpr (Capacity > 0)
                {
                    if (Result.GetSize() + Units >= Capacity)
                    {
                        Result.Clear();
                        break;
                    }
                }

                if (Units == 1)
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

        /// \brief Writes typed or pasted text, dropping control characters and broken sequences.
        ///
        /// \param Content The text to write.
        /// \param Lines   `true` to keep line breaks, `false` to write each as a space.
        /// \param Room    The most characters to write.
        /// \param Output  Receives the characters kept, appended to what it holds.
        /// \return The number of characters written.
        template<UInt Capacity>
        ZY_INLINE static constexpr UInt Clean(Text Content, Bool Lines, UInt Room, Ref<String<Capacity>> Output)
        {
            UInt Added  = 0;
            UInt Cursor = 0;

            while (Cursor < Content.GetSize() && Added < Room)
            {
                const UInt Length = GetSize(Content[Cursor]);

                // A broken sequence is skipped a byte at a time, so the rest of the text still arrives.
                if (Length == 0 || Cursor + Length > Content.GetSize())
                {
                    ++Cursor;
                    continue;
                }

                const UInt32 Codepoint = Peek(Content, Cursor);
                const Bool   Control   = Codepoint < 0x20 || (Codepoint >= 0x7F && Codepoint <= 0x9F);

                // Control characters are never text; a line break is kept where lines are, and a space elsewhere.
                if (Codepoint == '\n')
                {
                    Output.Append(Lines ? '\n' : ' ');
                    ++Added;
                }
                else if (!Control)
                {
                    Output.Append(Content.Slice(Cursor, Length));
                    ++Added;
                }
                Cursor += Length;
            }
            return Added;
        }

        /// \brief Counts the characters of a text.
        ///
        /// \param Content The text to count.
        /// \return The number of characters, each byte that continues none counted as one.
        ZY_INLINE static constexpr UInt Count(Text Content)
        {
            UInt Result = 0;

            for (UInt Index = 0; Index < Content.GetSize(); ++Index)
            {
                Result += IsContinuation(Content[Index]) ? 0 : 1;
            }
            return Result;
        }

        /// \brief Gets the offset after the character at an offset.
        ///
        /// \param Content The text.
        /// \param Offset  The offset of the character.
        /// \return The offset after it, at most the size of the text.
        ZY_INLINE static constexpr UInt Next(Text Content, UInt Offset)
        {
            if (Offset >= Content.GetSize())
            {
                return Content.GetSize();
            }

            // The character runs on over the continuation bytes after its first.
            ++Offset;

            while (Offset < Content.GetSize() && IsContinuation(Content[Offset]))
            {
                ++Offset;
            }
            return Offset;
        }

        /// \brief Gets the offset of the character before an offset.
        ///
        /// \param Content The text.
        /// \param Offset  The offset after the character.
        /// \return The offset it starts at, or `0` at the start of the text.
        ZY_INLINE static constexpr UInt Previous(Text Content, UInt Offset)
        {
            if (Offset == 0)
            {
                return 0;
            }

            // The character before starts at the last byte before the offset that is not a continuation byte.
            Offset = Min(Offset, Content.GetSize()) - 1;

            while (Offset > 0 && IsContinuation(Content[Offset]))
            {
                --Offset;
            }
            return Offset;
        }

        /// \brief Takes an offset back to the start of the character it falls inside, so it never splits one.
        ///
        /// \param Content The text.
        /// \param Offset  The offset.
        /// \return The offset of the character's first byte, at most the size of the text.
        ZY_INLINE static constexpr UInt Snap(Text Content, UInt Offset)
        {
            Offset = Min(Offset, Content.GetSize());

            while (Offset > 0 && Offset < Content.GetSize() && IsContinuation(Content[Offset]))
            {
                --Offset;
            }
            return Offset;
        }

        /// \brief Gets the start of the next word, past the rest of the word and the gap after it.
        ///
        /// \param Content The text.
        /// \param Offset  The offset to step from.
        /// \return The offset, or the size of the text past the last word.
        ZY_INLINE static constexpr UInt Forward(Text Content, UInt Offset)
        {
            UInt Cursor = Min(Offset, Content.GetSize());

            while (Cursor < Content.GetSize() && IsWord(Peek(Content, Cursor)))
            {
                Cursor = Next(Content, Cursor);
            }

            while (Cursor < Content.GetSize() && !IsWord(Peek(Content, Cursor)))
            {
                Cursor = Next(Content, Cursor);
            }
            return Cursor;
        }

        /// \brief Gets the start of the word before an offset, past the gap before it.
        ///
        /// \param Content The text.
        /// \param Offset  The offset to step from.
        /// \return The offset, or `0` before the first word.
        ZY_INLINE static constexpr UInt Backward(Text Content, UInt Offset)
        {
            UInt Cursor = Min(Offset, Content.GetSize());

            while (Cursor > 0 && !IsWord(Peek(Content, Previous(Content, Cursor))))
            {
                Cursor = Previous(Content, Cursor);
            }

            while (Cursor > 0 && IsWord(Peek(Content, Previous(Content, Cursor))))
            {
                Cursor = Previous(Content, Cursor);
            }
            return Cursor;
        }

        /// \brief Finds the run of word characters, or of the others, holding the character at an offset.
        ///
        /// \param Content The text.
        /// \param Offset  The offset of the character; past the end, the last character is taken.
        /// \param Start   Receives where the run starts.
        /// \param End     Receives where the run ends.
        ZY_INLINE static constexpr void Word(Text Content, UInt Offset, Ref<UInt> Start, Ref<UInt> End)
        {
            if (Content.IsEmpty())
            {
                Start = 0;
                End   = 0;
                return;
            }

            // Past the end, the run holding the last character is taken.
            const UInt At   = Offset < Content.GetSize()
                ? Previous(Content, Next(Content, Offset))
                : Previous(Content, Content.GetSize());
            const Bool Kind = IsWord(Peek(Content, At));

            Start = At;
            End   = Next(Content, At);

            while (Start > 0 && IsWord(Peek(Content, Previous(Content, Start))) == Kind)
            {
                Start = Previous(Content, Start);
            }

            while (End < Content.GetSize() && IsWord(Peek(Content, End)) == Kind)
            {
                End = Next(Content, End);
            }
        }

        /// \brief Gets how many bytes the sequence a byte leads takes.
        ///
        /// \param Lead The byte.
        /// \return The count, from one to four, or `0` when the byte cannot lead a sequence.
        ZY_INLINE static constexpr UInt GetSize(Char Lead)
        {
            const UInt8 Byte = static_cast<UInt8>(Lead);

            if ((Byte & 0x80) == 0x00)
            {
                return 1;
            }
            if ((Byte & 0xE0) == 0xC0)
            {
                return 2;
            }
            if ((Byte & 0xF0) == 0xE0)
            {
                return 3;
            }
            if ((Byte & 0xF8) == 0xF0)
            {
                return 4;
            }
            return 0;
        }

        /// \brief Gets how many bytes a codepoint takes once encoded.
        ///
        /// \param Codepoint The codepoint.
        /// \return The count, from one to four.
        ZY_INLINE static constexpr UInt GetLength(UInt32 Codepoint)
        {
            return Codepoint <= 0x7F ? 1 : (Codepoint <= 0x7FF ? 2 : (Codepoint <= 0xFFFF ? 3 : 4));
        }

        /// \brief Checks whether a byte continues a sequence, rather than leading one.
        ///
        /// \param Byte The byte.
        /// \return `true` if it continues a sequence, `false` otherwise.
        ZY_INLINE static constexpr Bool IsContinuation(Char Byte)
        {
            return (static_cast<UInt8>(Byte) & 0xC0) == 0x80;
        }

        /// \brief Checks whether a codepoint belongs to a word, as a letter, a digit or an underscore does.
        ///
        /// \param Codepoint The codepoint.
        /// \return `true` if it belongs to a word, `false` otherwise.
        ZY_INLINE static constexpr Bool IsWord(UInt32 Codepoint)
        {
            // Past ASCII, everything but the spaces and the general punctuation is taken as a letter.
            if (Codepoint >= 0x80)
            {
                return Codepoint != 0xA0 && !(Codepoint >= 0x2000 && Codepoint <= 0x206F) && Codepoint != 0x3000;
            }
            return StrIsIdentifier(static_cast<Char>(Codepoint));
        }
    };
}