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

#include "Array.hpp"
#include "Zyphryon.Base/Bit.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A fixed-size bitset backed by an inline array of platform-width words; no heap allocation is performed.
    template<UInt Size, typename Word = UInt>
    class Bitset final
    {
        static_assert(Size > 0, "Bitset size must be greater than zero");

        /// \brief All bits set in a word (all ones).
        static constexpr Word kWordFull  = ~Word(0);

        /// \brief The number of bits in a single storage word.
        static constexpr UInt kWordBits  = sizeof(Word) * 8u;

        /// \brief The number of words required to store \p Size bits.
        static constexpr UInt kWordCount = (Size + kWordBits - 1u) / kWordBits;

    public:

        /// \brief Constructs a bitset with all bits cleared.
        ZY_INLINE constexpr Bitset()
            : mData { }
        {
        }

        /// \brief Tests whether the bit at the given index is set.
        ///
        /// \param Index The zero-based bit index to test.
        /// \return `true` if the bit is set, otherwise `false`.
        ZY_INLINE constexpr Bool Test(UInt Index) const
        {
            ZY_ASSERT(Index < Size, "Bit index is out of range");

            return (mData[Index / kWordBits] >> (Index % kWordBits)) & Word(1);
        }

        /// \brief Sets the bit at the given index.
        ///
        /// \param Index The zero-based bit index to set.
        ZY_INLINE constexpr void Set(UInt Index)
        {
            ZY_ASSERT(Index < Size, "Bit index is out of range");

            mData[Index / kWordBits] |= (Word(1) << (Index % kWordBits));
        }

        /// \brief Clears the bit at the given index.
        ///
        /// \param Index The zero-based bit index to clear.
        ZY_INLINE constexpr void Reset(UInt Index)
        {
            ZY_ASSERT(Index < Size, "Bit index is out of range");

            mData[Index / kWordBits] &= ~(Word(1) << (Index % kWordBits));
        }

        /// \brief Toggles the bit at the given index.
        ///
        /// \param Index The zero-based bit index to toggle.
        ZY_INLINE constexpr void Toggle(UInt Index)
        {
            ZY_ASSERT(Index < Size, "Bit index is out of range");

            mData[Index / kWordBits] ^= (Word(1) << (Index % kWordBits));
        }

        /// \brief Clears all bits in the bitset.
        ZY_INLINE constexpr void Reset()
        {
            mData.Fill(Word(0));
        }

        /// \brief Gets the number of bits currently set.
        ///
        /// \return The total count of set bits.
        ZY_INLINE constexpr UInt Count() const
        {
            UInt Total = 0;

            for (UInt Index = 0; Index < kWordCount; ++Index)
            {
                Total += CountBits(mData[Index]);
            }
            return Total;
        }

        /// \brief Checks whether at least one bit is set.
        ///
        /// \return `true` if any bit is set, otherwise `false`.
        ZY_INLINE constexpr Bool Any() const
        {
            for (UInt Index = 0; Index < kWordCount; ++Index)
            {
                if (mData[Index] != 0)
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Checks whether no bits are set.
        ///
        /// \return `true` if all bits are clear, otherwise `false`.
        ZY_INLINE constexpr Bool None() const
        {
            return !Any();
        }

        /// \brief Checks whether all bits are set.
        ///
        /// \return `true` if every bit is set, otherwise `false`.
        ZY_INLINE constexpr Bool All() const
        {
            for (UInt Index = 0u; Index < kWordCount - 1; ++Index)
            {
                if (mData[Index] != kWordFull)
                {
                    return false;
                }
            }

            // Check the last (possibly partial) word.
            constexpr UInt kRemainder = Size % kWordBits;
            constexpr Word kLastMask  = kRemainder ? ((Word(1) << kRemainder) - 1u) : kWordFull;
            return (mData[kWordCount - 1] & kLastMask) == kLastMask;
        }

        /// \brief Gets the index of the lowest set bit.
        ///
        /// \return The zero-based index of the first set bit, or \p Size if no bit is set.
        ZY_INLINE constexpr UInt FindFirstSet() const
        {
            for (UInt Index = 0u; Index < kWordCount; ++Index)
            {
                if (mData[Index] != 0u)
                {
                    return Index * kWordBits + CountTrailingZeros(mData[Index]);
                }
            }
            return Size;
        }

        /// \brief Gets the index of the lowest clear bit.
        ///
        /// \return The zero-based index of the first clear bit, or \p Size if all bits are set.
        ZY_INLINE constexpr UInt FindFirstClear() const
        {
            constexpr UInt kRemainder = Size % kWordBits;
            constexpr Word kLastMask  = kRemainder ? ((Word(1u) << kRemainder) - 1u) : kWordFull;

            for (UInt Index = 0u; Index < kWordCount; ++Index)
            {
                const Word Mask = (Index == kWordCount - 1) ? kLastMask : kWordFull;

                if (const Word Inverted = ~mData[Index] & Mask; Inverted != 0u)
                {
                    const UInt Bit = Index * kWordBits + CountTrailingZeros(Inverted);
                    return Bit < Size ? Bit : Size;
                }
            }
            return Size;
        }

        /// \brief Gets the index of the first clear bit at or after the given offset.
        ///
        /// \param Offset The zero-based bit index to start searching from.
        /// \return The zero-based index of the first clear bit at or after \p Offset, or \p Size if no such bit exists.
        ZY_INLINE constexpr UInt FindFirstClear(UInt Offset) const
        {
            const UInt StartWord = Offset / kWordBits;
            const UInt StartBit  = Offset % kWordBits;

            constexpr UInt kRemainder = Size % kWordBits;
            constexpr Word kLastMask  = kRemainder ? ((Word(1u) << kRemainder) - 1u) : kWordFull;

            for (UInt Index = StartWord; Index < kWordCount; ++Index)
            {
                const Word Mask     = (Index == kWordCount - 1) ? kLastMask : kWordFull;
                const Word Inverted = ~mData[Index] & Mask;
                const Word Filtered = (Index == StartWord) ? (Inverted & (kWordFull << StartBit)) : Inverted;

                if (Filtered != 0u)
                {
                    const UInt Bit = Index * kWordBits + CountTrailingZeros(Filtered);
                    return Bit < Size ? Bit : Size;
                }
            }
            return Size;
        }

        /// \brief Gets the index of the highest set bit.
        ///
        /// \return The zero-based index of the last set bit, or \p Size if no bit is set.
        ZY_INLINE constexpr UInt FindLastSet() const
        {
            for (UInt Index = kWordCount; Index-- > 0;)
            {
                if (mData[Index] != 0)
                {
                    return Index * kWordBits + (kWordBits - 1 - CountLeadingZeros(mData[Index]));
                }
            }
            return Size;
        }

        /// \brief Gets the index of the highest set bit at or before the given offset.
        ///
        /// \param Offset The zero-based bit index to start searching from (inclusive).
        /// \return The zero-based index of the last set bit at or before \p Offset, or \p Size if no such bit exists.
        ZY_INLINE constexpr UInt FindLastSet(UInt Offset) const
        {
            constexpr UInt kRemainder = Size % kWordBits;
            constexpr Word kLastMask  = kRemainder ? ((Word(1u) << kRemainder) - 1u) : kWordFull;

            const UInt StartWord = Offset / kWordBits;

            for (UInt Index = StartWord + 1; Index-- > 0;)
            {
                const Word Clamp = (Index == kWordCount - 1) ? kLastMask : kWordFull;
                const Word Mask  = (Index == StartWord) ? ((Word(1) << ((Offset % kWordBits) + 1)) - 1u) : kWordFull;

                if (const Word Masked = mData[Index] & Clamp & Mask)
                {
                    return Index * kWordBits + (kWordBits - 1 - CountLeadingZeros(Masked));
                }
            }
            return Size;
        }

        /// \brief Gets the index of the highest clear bit.
        ///
        /// \return The zero-based index of the last clear bit, or \p Size if all bits are set.
        ZY_INLINE constexpr UInt FindLastClear() const
        {
            constexpr UInt kRemainder = Size % kWordBits;
            constexpr Word kLastMask  = kRemainder ? ((Word(1u) << kRemainder) - 1u) : kWordFull;

            for (UInt Index = kWordCount; Index-- > 0;)
            {
                const Word Mask = (Index == kWordCount - 1) ? kLastMask : kWordFull;

                if (const Word Inverted = ~mData[Index] & Mask; Inverted != 0u)
                {
                    const UInt Bit = Index * kWordBits + (kWordBits - 1 - CountLeadingZeros(Inverted));
                    return Bit < Size ? Bit : Size;
                }
            }
            return Size;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Word, kWordCount> mData;
    };
}