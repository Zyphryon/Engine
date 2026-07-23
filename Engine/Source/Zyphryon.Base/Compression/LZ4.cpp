// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "LZ4.hpp"
#include "Zyphryon.Base/Scalar.hpp"
#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    namespace
    {
        // The stride used by the wild-copy fast paths, in bytes.
        constexpr UInt32 kLZ4WildStride      = 16;

        // The smallest match the format can encode; shorter runs stay as literals.
        constexpr UInt32 kLZ4MinMatch        = 4;

        // The number of trailing bytes always emitted as literals (format requirement).
        constexpr UInt32 kLZ4LastLiterals    = 5;

        // No match may start within this many bytes of the end, guaranteeing a trailing literal run.
        constexpr UInt32 kLZ4MatchFindMargin = 12;

        // The largest back-reference distance a two-byte offset can encode.
        constexpr UInt32 kLZ4MaxOffset       = 0xFFFF;

        // The base-two size of the match-finder hash table, in entries.
        constexpr UInt32 kLZ4HashLog         = 12;

        // Governs how aggressively the match finder skips ahead over incompressible input.
        constexpr UInt32 kLZ4SkipTrigger     = 6;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LZ4CopyWild(Ptr<Byte> Destination, ConstPtr<Byte> Source, UInt32 Count)
    {
        Ptr<Byte>       Cursor = Destination;
        const Ptr<Byte> Finish = Destination + Count;

        do
        {
            Base::Blit(Cursor, kLZ4WildStride, Source);
            Cursor += kLZ4WildStride;
            Source += kLZ4WildStride;
        }
        while (Cursor < Finish);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LZ4CopyOverlap(Ptr<Byte> Destination, ConstPtr<Byte> Match, UInt32 Length, UInt32 Offset)
    {
        if (Offset == 1)
        {
            Base::Fill(Destination, Length, * Match);
            return;
        }

        Ptr<Byte>       Cursor = Destination;
        const Ptr<Byte> Finish = Destination + Length;

        do
        {
            const UInt32 Available = static_cast<UInt32>(Cursor - Match);
            const UInt32 Chunk     = Base::Min(Available, static_cast<UInt32>(Finish - Cursor));

            Base::Blit(Cursor, Chunk, Match);
            Cursor += Chunk;
        }
        while (Cursor < Finish);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 LZ4Read32(ConstPtr<Byte> Pointer)
    {
        UInt32 Value;
        Base::Blit(AddressOf(Value), sizeof(Value), Pointer);
        return Value;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 LZ4Hash(ConstPtr<Byte> Pointer)
    {
        return (LZ4Read32(Pointer) * 2654435761u) >> (32 - kLZ4HashLog);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LZ4WriteLength(Ref<Ptr<Byte>> Output, UInt32 Remainder)
    {
        while (Remainder >= 255)
        {
            * Output++ = 255;
            Remainder -= 255;
        }
        * Output++ = static_cast<Byte>(Remainder);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 LZ4Encode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity)
    {
        ZY_ASSERT(Capacity >= LZ4Bound(Source.GetSize()), "LZ4Encode requires Capacity >= LZ4Bound(Source size)");

        const ConstPtr<Byte> Origin    = Source.GetData();
        const UInt32         Length    = static_cast<UInt32>(Source.GetSize());
        const ConstPtr<Byte> InputEnd  = Origin + Length;

        Ptr<Byte>            Output    = Destination;
        ConstPtr<Byte>       Anchor    = Origin;
        ConstPtr<Byte>       Input     = Origin;

        // Inputs too small to hold a match are emitted verbatim as a single literal run.
        if (Length >= kLZ4MatchFindMargin + 1)
        {
            UInt32 Table[1 << kLZ4HashLog] { };

            const ConstPtr<Byte> MatchFindLimit = InputEnd - kLZ4MatchFindMargin;
            const ConstPtr<Byte> MatchLimit     = InputEnd - kLZ4LastLiterals;

            Table[LZ4Hash(Origin)] = 0;
            Input = Origin + 1;

            for (;;)
            {
                // Locate the next match, skipping ahead faster the longer we go without one.
                ConstPtr<Byte> Match;
                {
                    UInt32         Search  = 1 << kLZ4SkipTrigger;
                    ConstPtr<Byte> Forward = Input;

                    for (;;)
                    {
                        Input = Forward;

                        if (Input > MatchFindLimit)
                        {
                            goto Trailing;
                        }

                        const UInt32 Hash = LZ4Hash(Input);
                        Match       = Origin + Table[Hash];
                        Table[Hash] = static_cast<UInt32>(Input - Origin);

                        Forward = Input + (Search++ >> kLZ4SkipTrigger);

                        if (static_cast<UInt32>(Input - Match) <= kLZ4MaxOffset && LZ4Read32(Match) == LZ4Read32(Input))
                        {
                            break;
                        }
                    }
                }

                // Extend the match backward over identical bytes preceding both cursors.
                while (Input > Anchor && Match > Origin && Input[-1] == Match[-1])
                {
                    --Input;
                    --Match;
                }

                // Emit the pending literals, prefixed by the sequence token.
                const UInt32 Literals = static_cast<UInt32>(Input - Anchor);

                const Ptr<Byte> Token = Output++;

                if (Literals >= 15)
                {
                    LZ4WriteLength(Output, Literals - 15);
                    (* Token) = 0xF0;
                }
                else
                {
                    (* Token) = static_cast<Byte>(Literals << 4);
                }

                Blit(Output, Literals, Anchor);
                Output += Literals;

                // Emit the two-byte little-endian back-reference offset.
                const UInt32 Offset = static_cast<UInt32>(Input - Match);

                (* Output++) = static_cast<Byte>(Offset & 0xFF);
                (* Output++) = static_cast<Byte>(Offset >> 8);

                // Count the match length past the guaranteed minimum, then fold it into the token.
                ConstPtr<Byte> Cursor    = Input + kLZ4MinMatch;
                ConstPtr<Byte> Reference = Match + kLZ4MinMatch;

                while (Cursor < MatchLimit && * Cursor == * Reference)
                {
                    ++Cursor;
                    ++Reference;
                }

                const UInt32 MatchLength = static_cast<UInt32>(Cursor - Input) - kLZ4MinMatch;

                if (MatchLength >= 15)
                {
                    LZ4WriteLength(Output, MatchLength - 15);
                    (* Token) |= 0x0F;
                }
                else
                {
                    (* Token) |= static_cast<Byte>(MatchLength);
                }

                // Resume searching immediately past the match.
                Anchor = Cursor;
                Input  = Cursor;
            }
        }

        // Emit the trailing literal run; the decoder stops here without reading a match.
    Trailing:
        {
            const UInt32 Literals = static_cast<UInt32>(InputEnd - Anchor);

            const Ptr<Byte> Token = Output++;

            if (Literals >= 15)
            {
                LZ4WriteLength(Output, Literals - 15);
                (* Token) = 0xF0;
            }
            else
            {
                (* Token) = static_cast<Byte>(Literals << 4);
            }

            Blit(Output, Literals, Anchor);
            Output += Literals;
        }

        return static_cast<UInt32>(Output - Destination);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 LZ4Decode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity)
    {
        ConstPtr<Byte>       Input         = Source.GetData();
        const ConstPtr<Byte> InputEnd      = Source.GetData() + Source.GetSize();
        const ConstPtr<Byte> InputWildEnd  = Source.GetSize() >= kLZ4WildStride ? InputEnd  - kLZ4WildStride : Input;

        Ptr<Byte>            Output        = Destination;
        const Ptr<Byte>      OutputEnd     = Destination + Capacity;
        const Ptr<Byte>      OutputWildEnd = Capacity >= kLZ4WildStride ? OutputEnd - kLZ4WildStride : Output;

        while (Input < InputEnd)
        {
            const UInt32 Token = * Input++;

            UInt32 Literals = Token >> 4;

            if (Literals == 15)
            {
                UInt8 Extra;

                do
                {
                    if (Input >= InputEnd)
                    {
                        return 0;
                    }
                    Extra     = * Input++;
                    Literals += Extra;
                }
                while (Extra == 255);
            }

            if (Literals > static_cast<UInt32>(InputEnd - Input) || Literals > static_cast<UInt32>(OutputEnd - Output))
            {
                return 0;
            }

            if (Literals && Input + Literals <= InputWildEnd && Output + Literals <= OutputWildEnd)
            {
                LZ4CopyWild(Output, Input, Literals);
            }
            else
            {
                Blit(Output, Literals, Input);
            }
            Output += Literals;
            Input  += Literals;

            if (Input >= InputEnd)
            {
                break;
            }

            if (InputEnd - Input < 2)
            {
                return 0;
            }

            const UInt32 Offset = static_cast<UInt32>(Input[0]) | (static_cast<UInt32>(Input[1]) << 8);
            Input += 2;

            if (Offset == 0 || Offset > static_cast<UInt32>(Output - Destination))
            {
                return 0;
            }

            UInt32 Length = (Token & 0x0F) + 4;

            if ((Token & 0x0F) == 15)
            {
                UInt8 Extra;

                do
                {
                    if (Input >= InputEnd)
                    {
                        return 0;
                    }
                    Extra   = * Input++;
                    Length += Extra;
                }
                while (Extra == 255);
            }

            if (Length > static_cast<UInt32>(OutputEnd - Output))
            {
                return 0;
            }

            const ConstPtr<Byte> Match = Output - Offset;

            if (Offset < Length)
            {
                LZ4CopyOverlap(Output, Match, Length, Offset);
            }
            else if (Offset >= kLZ4WildStride && Output + Length <= OutputWildEnd)
            {
                LZ4CopyWild(Output, Match, Length);
            }
            else
            {
                Blit(Output, Length, Match);
            }
            Output += Length;
        }
        return static_cast<UInt32>(Output - Destination);
    }
}
