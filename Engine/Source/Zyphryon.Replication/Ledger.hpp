// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Represents a list of plain slots that marks each one it changes, so a delta carries only those.
    template<typename Type>
    class Ledger final
    {
        static_assert(IsTriviallyCopyable<Type>, "A slot is plain data");

    public:

        /// \brief Constructs a ledger holding no slot.
        ZY_INLINE Ledger()
            : mShrunk { false }
        {
        }

        /// \brief Gets the slots, read only, so the ledger views as a \ref ConstSpan.
        ///
        /// \return The first slot, or `nullptr` when there is none.
        ZY_INLINE ConstPtr<Type> GetData() const
        {
            return mSlots.GetData();
        }

        /// \brief Gets the number of slots.
        ///
        /// \return The number of slots, up to \ref kLimit.
        ZY_INLINE UInt32 GetSize() const
        {
            return static_cast<UInt32>(mSlots.GetSize());
        }

        /// \brief Checks whether anything changed since the last delta, a slot or the count.
        ///
        /// \return `true` if a delta would carry a slot or a shrink, `false` if it would tell nothing new.
        ZY_INLINE Bool IsDirty() const
        {
            if (mShrunk)
            {
                return true;
            }

            for (UInt32 Word = 0; Word < mMarks.GetSize(); ++Word)
            {
                if (GetMarks(Word) != 0)
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Sets a slot, marking it only when its bytes change.
        ///
        /// \param Index The place of the slot, below \ref GetSize.
        /// \param Value The value to set.
        ZY_INLINE void Set(UInt32 Index, ConstRef<Type> Value)
        {
            const ConstPtr<Byte> Held = reinterpret_cast<ConstPtr<Byte>>(AddressOf(mSlots[Index]));

            if (!Compare(Held, reinterpret_cast<ConstPtr<Byte>>(AddressOf(Value)), sizeof(Type)))
            {
                mSlots[Index] = Value;
                Mark(Index);
            }
        }

        /// \brief Gets a slot to change, marking it.
        ///
        /// \param Index The place of the slot, below \ref GetSize.
        /// \return The slot, which holds until the ledger grows or shrinks.
        ZY_INLINE Ref<Type> Edit(UInt32 Index)
        {
            Mark(Index);
            return mSlots[Index];
        }

        /// \brief Adds a slot at the end, marking it.
        ///
        /// \param Value The value of the slot.
        /// \return The place of the slot.
        ZY_INLINE UInt32 Append(ConstRef<Type> Value)
        {
            ZY_ASSERT(mSlots.GetSize() < kLimit, "A ledger holds at most kLimit slots");

            mSlots.Append(Value);
            Fit();
            Mark(GetSize() - 1);
            return GetSize() - 1;
        }

        /// \brief Removes a slot, moving the last one into its place and marking it.
        ///
        /// \param Index The place of the slot, below \ref GetSize.
        ZY_INLINE void Remove(UInt32 Index)
        {
            mSlots.RemoveFast(Index);
            mShrunk = true;

            if (Index < GetSize())
            {
                Mark(Index);
            }
        }

        /// \brief Changes the number of slots, keeping those below both counts and marking each new one.
        ///
        /// \param Count The number of slots, up to \ref kLimit.
        ZY_INLINE void Resize(UInt32 Count)
        {
            ZY_ASSERT(Count <= kLimit, "A ledger holds at most kLimit slots");

            const UInt32 Kept = GetSize();

            if (Count < Kept)
            {
                mSlots.Remove(Count, Kept - Count);
                mShrunk = true;
                return;
            }
            mSlots.Expand(Count);
            Fit();

            for (UInt32 Index = Kept; Index < Count; ++Index)
            {
                Mark(Index);
            }
        }

        /// \brief Writes or reads every slot, or the slots marked since the last delta.
        ///
        /// \param Archive The archive to write to or read from.
        /// \param Delta   `true` for the marked slots, which writing forgets, `false` for every slot.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive, Bool Delta = false)
        {
            if constexpr (Serializer::IsWriter)
            {
                Write(Archive, Delta);
            }
            else
            {
                Read(Archive, Delta);
            }
        }

        /// \brief Gets a slot, read only, since a change goes through \ref Set or \ref Edit to be marked.
        ///
        /// \param Index The place of the slot, below \ref GetSize.
        /// \return The slot.
        ZY_INLINE ConstRef<Type> operator[](UInt32 Index) const
        {
            return mSlots[Index];
        }

    private:

        /// \brief The most slots a ledger holds, which keeps a lying peer from growing one without end.
        static constexpr UInt32 kLimit = 4096;

        /// \brief The bits in one word of marks.
        static constexpr UInt32 kBits  = 64;

        /// \brief Writes the count, then every slot, or the marked slots behind their places.
        ///
        /// \param Archive The archive to write to.
        /// \param Delta   `true` for the marked slots, whose marks it forgets, `false` for every slot.
        template<typename Serializer>
        ZY_INLINE void Write(Serializer Archive, Bool Delta)
        {
            UInt16 Count = static_cast<UInt16>(GetSize());
            Archive.Serialize(Count);

            // A whole write keeps the marks, since the peers that already hold the slots still wait for the delta.
            if (!Delta)
            {
                for (Ref<Type> Slot : mSlots)
                {
                    Archive.Serialize(Slot);
                }
                return;
            }

            UInt16 Changed = 0;

            for (UInt32 Word = 0; Word < mMarks.GetSize(); ++Word)
            {
                Changed += static_cast<UInt16>(CountBits(GetMarks(Word)));
            }
            Archive.Serialize(Changed);

            for (UInt32 Word = 0; Word < mMarks.GetSize(); ++Word)
            {
                for (UInt64 Pending = GetMarks(Word); Pending; Pending &= Pending - 1)
                {
                    UInt16 Index = static_cast<UInt16>(Word * kBits + CountTrailingZeros(Pending));
                    Archive.Serialize(Index);
                    Archive.Serialize(mSlots[Index]);
                }
                mMarks[Word] = 0;
            }
            mShrunk = false;
        }

        /// \brief Reads what \ref Write wrote, every slot or a delta over the slots held, dropping what a lie names.
        ///
        /// \param Archive The archive to read from.
        /// \param Delta   `true` for a delta, `false` for every slot.
        template<typename Serializer>
        ZY_INLINE void Read(Serializer Archive, Bool Delta)
        {
            UInt16 Count = 0;
            Archive.Serialize(Count);

            if (!Delta)
            {
                // Every slot takes a byte at least, so a count past them, or past the most, is a lie.
                if (Count > kLimit || Count > Archive.GetAvailable())
                {
                    Count = 0;
                }
                mSlots.Clear();
                mSlots.Advance(Count);

                for (Ref<Type> Slot : mSlots)
                {
                    Archive.Serialize(Slot);
                }
                mMarks.Clear();
                Fit();
                return;
            }

            UInt16 Changed = 0;
            Archive.Serialize(Changed);

            // A delta carries every slot it adds behind its place, so growth past that is a lie.
            if (Count > kLimit || Changed * sizeof(UInt16) > Archive.GetAvailable() || Count > GetSize() + Changed)
            {
                return;
            }
            Resize(Count);

            for (UInt32 Step = 0; Step < Changed; ++Step)
            {
                UInt16 Index = 0;
                Archive.Serialize(Index);

                if (Index >= Count)
                {
                    return;
                }
                Archive.Serialize(mSlots[Index]);
            }
        }

        /// \brief Gets one word of marks, without those past the last slot.
        ///
        /// \param Word The place of the word.
        /// \return The marks of the slots the word covers.
        ZY_INLINE UInt64 GetMarks(UInt32 Word) const
        {
            const UInt32 First = Word * kBits;
            const UInt32 Count = GetSize();

            if (First >= Count)
            {
                return 0;
            }
            return Count - First >= kBits ? mMarks[Word] : mMarks[Word] & ((1ull << (Count - First)) - 1);
        }

        /// \brief Grows the marks to a word for every slot, each new word clear, so marking never grows them.
        ZY_INLINE void Fit()
        {
            mMarks.Expand((GetSize() + kBits - 1) / kBits);
        }

        /// \brief Marks a slot for the next delta.
        ///
        /// \param Index The place of the slot, below \ref GetSize.
        ZY_INLINE void Mark(UInt32 Index)
        {
            mMarks[Index / kBits] |= 1ull << (Index % kBits);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Type>   mSlots;
        Sequence<UInt64> mMarks;
        Bool             mShrunk;
    };
}