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
        /// The stride used by the wild-copy fast paths, in bytes.
        constexpr UInt32 kLZ4WildStride      = 16;

        /// The smallest match the format can encode; shorter runs stay as literals.
        constexpr UInt32 kLZ4MinMatch        = 4;

        /// The number of trailing bytes always emitted as literals (format requirement).
        constexpr UInt32 kLZ4LastLiterals    = 5;

        /// No match may start within this many bytes of the end, guaranteeing a trailing literal run.
        constexpr UInt32 kLZ4MatchFindMargin = 12;

        /// The largest back-reference distance a two-byte offset can encode.
        constexpr UInt32 kLZ4MaxOffset       = 0xFFFF;

        /// The base-two size of the match-finder hash table, in entries.
        constexpr UInt32 kLZ4HashLog         = 12;

        /// Governs how aggressively the match finder skips ahead over incompressible input.
        constexpr UInt32 kLZ4SkipTrigger     = 6;

        /// The base-two size of the high-compression match-finder hash table, in entries.
        constexpr UInt32 kLZ4HighHashLog     = 15;

        /// The number of buckets the high-compression hash table holds.
        constexpr UInt32 kLZ4HighHashSize    = 1u << kLZ4HighHashLog;

        /// The chain spans the whole window a two-byte offset can reach, so no reachable match falls out of it.
        constexpr UInt32 kLZ4HighChainSize   = kLZ4MaxOffset + 1;

        /// Wraps a position onto its slot in the chain.
        constexpr UInt32 kLZ4HighChainMask   = kLZ4MaxOffset;

        /// Hash-chain match finder backing the high-compression path, searching every candidate at a hash for the longest match.
        struct LZ4HighFinder final
        {
            /// The first byte of the input, which every stored position is relative to.
            ConstPtr<Byte> Origin;

            /// Maps a hash to the most recent position carrying it, biased by one so zero reads as empty.
            Ptr<UInt32>    Table;

            /// Maps a position to the previous position sharing its hash, under the same bias.
            Ptr<UInt32>    Chain;

            /// The next position still to be filed into the table and chain.
            UInt32         Cursor;

            /// How many links of one chain to walk before settling for the best match found so far.
            UInt32         Attempts;
        };
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

    static Ptr<Byte> LZ4WriteLiterals(Ref<Ptr<Byte>> Output, ConstPtr<Byte> Anchor, UInt32 Literals)
    {
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

        return Token;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LZ4WriteMatch(Ref<Ptr<Byte>> Output, Ptr<Byte> Token, UInt32 Offset, UInt32 Excess)
    {
        (* Output++) = static_cast<Byte>(Offset & 0xFF);
        (* Output++) = static_cast<Byte>(Offset >> 8);

        if (Excess >= 15)
        {
            LZ4WriteLength(Output, Excess - 15);
            (* Token) |= 0x0F;
        }
        else
        {
            (* Token) |= static_cast<Byte>(Excess);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 LZ4HighHash(ConstPtr<Byte> Pointer)
    {
        return (LZ4Read32(Pointer) * 2654435761u) >> (32 - kLZ4HighHashLog);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void LZ4HighUpdate(Ref<LZ4HighFinder> Finder, UInt32 Position)
    {
        while (Finder.Cursor < Position)
        {
            const UInt32 Hash = LZ4HighHash(Finder.Origin + Finder.Cursor);

            Finder.Chain[Finder.Cursor & kLZ4HighChainMask] = Finder.Table[Hash];
            Finder.Table[Hash]                              = Finder.Cursor + 1;

            ++Finder.Cursor;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt32 LZ4HighFind(Ref<LZ4HighFinder> Finder, UInt32 Position, ConstPtr<Byte> MatchLimit, Ref<UInt32> Output)
    {
        LZ4HighUpdate(Finder, Position);

        const ConstPtr<Byte> Input     = Finder.Origin + Position;
        UInt32               Best      = 0;
        UInt32               Attempts  = Finder.Attempts;
        UInt32               Candidate = Finder.Table[LZ4HighHash(Input)];

        while (Candidate != 0 && Attempts-- > 0)
        {
            const UInt32 Match = Candidate - 1;

            // Chain slots are recycled every 64 KiB, so a link that fails to step backward is a stale entry
            // rather than a shorter distance, and walking it further would loop.
            if (Match >= Position || Position - Match > kLZ4MaxOffset)
            {
                break;
            }

            const ConstPtr<Byte> Reference = Finder.Origin + Match;

            if (LZ4Read32(Reference) == LZ4Read32(Input))
            {
                UInt32 Length = kLZ4MinMatch;

                while (Input + Length < MatchLimit && Input[Length] == Reference[Length])
                {
                    ++Length;
                }

                if (Length > Best)
                {
                    Best   = Length;
                    Output = Match;
                }
            }

            const UInt32 Next = Finder.Chain[Match & kLZ4HighChainMask];

            if (Next >= Candidate)
            {
                break;
            }
            Candidate = Next;
        }
        return Best;
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
                const Ptr<Byte> Token = LZ4WriteLiterals(Output, Anchor, static_cast<UInt32>(Input - Anchor));

                // Count the match length past the guaranteed minimum, then fold it into the token.
                ConstPtr<Byte> Cursor    = Input + kLZ4MinMatch;
                ConstPtr<Byte> Reference = Match + kLZ4MinMatch;

                while (Cursor < MatchLimit && * Cursor == * Reference)
                {
                    ++Cursor;
                    ++Reference;
                }

                LZ4WriteMatch(
                    Output,
                    Token,
                    static_cast<UInt32>(Input - Match),
                    static_cast<UInt32>(Cursor - Input) - kLZ4MinMatch);

                // Resume searching immediately past the match.
                Anchor = Cursor;
                Input  = Cursor;
            }
        }

        // Emit the trailing literal run; the decoder stops here without reading a match.
    Trailing:

        LZ4WriteLiterals(Output, Anchor, static_cast<UInt32>(InputEnd - Anchor));

        return static_cast<UInt32>(Output - Destination);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 LZ4Encode(ConstSpan<Byte> Source, Ptr<Byte> Destination, UInt32 Capacity, UInt32 Level)
    {
        ZY_ASSERT(Capacity >= LZ4Bound(Source.GetSize()), "LZ4Encode requires Capacity >= LZ4Bound(Source size)");

        const ConstPtr<Byte> Origin   = Source.GetData();
        const UInt32         Length   = static_cast<UInt32>(Source.GetSize());
        const ConstPtr<Byte> InputEnd = Origin + Length;

        Ptr<Byte>            Output   = Destination;
        ConstPtr<Byte>       Anchor   = Origin;

        // Inputs too small to hold a match are emitted verbatim as a single literal run.
        if (Length >= kLZ4MatchFindMargin + 1)
        {
            const ConstPtr<Byte> MatchFindLimit = InputEnd - kLZ4MatchFindMargin;
            const ConstPtr<Byte> MatchLimit     = InputEnd - kLZ4LastLiterals;

            // Table and chain together run to 384 KiB, far past what belongs on a stack.
            const Ptr<UInt32> Scratch = new UInt32[kLZ4HighHashSize + kLZ4HighChainSize] { };

            LZ4HighFinder Finder;
            Finder.Origin   = Origin;
            Finder.Table    = Scratch;
            Finder.Chain    = Scratch + kLZ4HighHashSize;
            Finder.Cursor   = 0;
            Finder.Attempts = 1u << (Clamp(Level, kLZ4LevelMin, kLZ4LevelMax) - 1);

            UInt32 Position = 0;

            while (Origin + Position <= MatchFindLimit)
            {
                UInt32 Match = 0;
                UInt32 Found = LZ4HighFind(Finder, Position, MatchLimit, Match);

                if (Found == 0)
                {
                    ++Position;
                    continue;
                }

                while (Origin + Position + 1 <= MatchFindLimit)
                {
                    UInt32       Later = 0;
                    const UInt32 Extra = LZ4HighFind(Finder, Position + 1, MatchLimit, Later);

                    if (Extra <= Found)
                    {
                        break;
                    }

                    ++Position;
                    Found = Extra;
                    Match = Later;
                }

                while (Position > static_cast<UInt32>(Anchor - Origin) && Match > 0
                    && Origin[Position - 1] == Origin[Match - 1])
                {
                    --Position;
                    --Match;
                    ++Found;
                }

                const Ptr<Byte> Token = LZ4WriteLiterals(Output, Anchor, static_cast<UInt32>((Origin + Position) - Anchor));

                LZ4WriteMatch(Output, Token, Position - Match, Found - kLZ4MinMatch);

                Position += Found;
                Anchor    = Origin + Position;
            }

            delete[] Scratch;
        }

        // Emit the trailing literal run; the decoder stops here without reading a match.
        LZ4WriteLiterals(Output, Anchor, static_cast<UInt32>(InputEnd - Anchor));

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

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob LZ4Expand(ConstSpan<Byte> Source, UInt32 Size)
    {
        if (Size == 0 || Source.IsEmpty())
        {
            return Blob();
        }

        Blob Result = Blob::Allocate<Byte>(Size);

        if (Size == Source.GetSize())
        {
            Result.Copy<Byte>(Source.GetData(), Size);
        }
        else if (LZ4Decode(Source, Result.GetData<Byte>(), Size) != Size)
        {
            return Blob();
        }
        return Result;
    }
}