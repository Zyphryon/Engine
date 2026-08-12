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
#include "Bitset.hpp"
#include "Key.hpp"
#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Tracks allocation state for a fixed-capacity set of slots using a free-list strategy.
    ///
    /// \tparam Capacity  The number of slots to track.
    /// \tparam EpochBits The number of bits given to each slot's epoch, or zero to track none.
    template<UInt Capacity, UInt EpochBits = 16>
    class Freelist final
    {
    public:

        /// \brief The key naming one slot at the epoch it was handed out at.
        using Key   = Base::Key<Capacity, EpochBits>;

        /// \brief The integer a slot is counted in.
        using Slot  = typename Key::Slot;

        /// \brief The integer an epoch is counted in.
        using Epoch = typename Key::Epoch;

    public:

        /// \brief Constructs a freelist with all slots initially free.
        ZY_INLINE constexpr Freelist()
            : mHead { 0 },
              mTop  { 0 },
              mSize { 0 }
        {
        }

        /// \brief Resets all slots to the free state.
        ///
        /// \note Epochs carry over, since a key minted before the reset must not match a slot handed out after.
        ZY_INLINE constexpr void Clear()
        {
            mMask.Reset();
            mHead = 0;
            mTop  = 0;
            mSize = 0;
        }

        /// \brief Gets the number of currently allocated slots.
        ///
        /// \return The number of currently allocated slots.
        ZY_INLINE constexpr UInt GetCount() const
        {
            return mSize;
        }

        /// \brief Gets the highest currently-valid slot, usable as an inclusive iteration bound.
        ///
        /// \return The highest allocated slot (1-based), or 0 if no slots are allocated.
        ZY_INLINE constexpr UInt GetTop() const
        {
            return (mSize == 0 ? 0 : mTop + 1);
        }

        /// \brief Gets the maximum number of slots that can be allocated.
        ///
        /// \return The maximum number of slots that can be allocated.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return Capacity;
        }

        /// \brief Checks whether all slots are currently allocated.
        ///
        /// \return `true` if all slots are allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsFull() const
        {
            return (mSize == Capacity);
        }

        /// \brief Checks whether no slots are currently allocated.
        ///
        /// \return `true` if no slots are allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return (mSize == 0);
        }

        /// \brief Allocates the next available slot.
        ///
        /// \return A key naming the newly allocated slot at its new epoch.
        ZY_INLINE constexpr Key Allocate()
        {
            UInt Found = mMask.FindFirstClear(mHead);

            if (Found >= Capacity)
            {
                Found = mMask.FindFirstClear(0);
            }

            ZY_ASSERT(Found < Capacity, "Attempted to allocate beyond maximum capacity");

            mMask.Set(Found);

            mHead = static_cast<Slot>(Found);
            mTop  = Max(mTop, static_cast<Slot>(Found));
            ++mSize;
            return Stamp(static_cast<Slot>(Found + 1));
        }

        /// \brief Acquires a specific slot.
        ///
        /// \param Index The slot to acquire, counted from one.
        /// \return A key naming that slot at its new epoch.
        ZY_INLINE constexpr Key Acquire(Slot Index)
        {
            ZY_ASSERT(Index > 0 && Index <= Capacity, "Attempted to acquire invalid slot");
            ZY_ASSERT(!IsOccupied(Index), "Attempted to acquire already allocated slot");

            mMask.Set(Index - 1);
            mHead = Min(mHead, static_cast<Slot>(Index - 1));
            mTop  = Max(mTop,  static_cast<Slot>(Index - 1));
            ++mSize;
            return Stamp(Index);
        }

        /// \brief Acquires the slot a key names, taking the key's epoch rather than minting a new one.
        ///
        /// \param Handle The key naming the slot to acquire.
        ZY_INLINE constexpr void Adopt(Key Handle)
        {
            const Slot Index = Handle.GetSlot();

            ZY_ASSERT(Index > 0 && Index <= Capacity, "Attempted to adopt invalid slot");
            ZY_ASSERT(!IsOccupied(Index), "Attempted to adopt already allocated slot");

            mMask.Set(Index - 1);
            mHead = Min(mHead, static_cast<Slot>(Index - 1));
            mTop  = Max(mTop,  static_cast<Slot>(Index - 1));
            ++mSize;

            if constexpr (EpochBits > 0)
            {
                mEpochs[Index - 1] = Handle.GetEpoch();
            }
        }

        /// \brief Frees the slot a key names, when the key still names its occupant.
        ///
        /// \param Handle The key naming the slot to free.
        ZY_INLINE constexpr void Free(Key Handle)
        {
            ZY_ASSERT(IsAllocated(Handle), "Attempted to free a slot the key no longer names");

            Release(Handle.GetSlot());
        }

        /// \brief Frees whatever occupies a slot, whichever key named it.
        ///
        /// \param Index The slot to free, counted from one.
        ZY_INLINE constexpr void Release(Slot Index)
        {
            ZY_ASSERT(IsOccupied(Index), "Attempted to free invalid slot");

            mMask.Reset(Index - 1);
            mHead = static_cast<Slot>(Index - 1);

            if (Index - 1 == mTop)
            {
                const UInt Last = mMask.FindLastSet();
                mTop = (Last == Capacity) ? Slot(0) : static_cast<Slot>(Last);
            }
            --mSize;
        }

        /// \brief Checks whether a key still names the occupant of its slot.
        ///
        /// \param Handle The key to check.
        /// \return `true` if the slot is allocated at the epoch the key holds, otherwise `false`.
        ZY_INLINE constexpr Bool IsAllocated(Key Handle) const
        {
            const Slot Index = Handle.GetSlot();

            if (!IsOccupied(Index))
            {
                return false;
            }

            if constexpr (EpochBits > 0)
            {
                return mEpochs[Index - 1] == Handle.GetEpoch();
            }
            else
            {
                return true;
            }
        }

        /// \brief Checks whether a slot is allocated, whichever key names it.
        ///
        /// \param Index The slot to check, counted from one.
        /// \return `true` if the slot is allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsOccupied(Slot Index) const
        {
            return Index > 0 && Index <= Capacity && mMask.Test(Index - 1);
        }

        /// \brief Gets the key naming whatever occupies a slot.
        ///
        /// \param Index The slot to name, counted from one.
        /// \return The key naming that slot at its current epoch.
        ZY_INLINE constexpr Key GetKey(Slot Index) const
        {
            if constexpr (EpochBits > 0)
            {
                return Key(Index, Index > 0 ? mEpochs[Index - 1] : Epoch(0));
            }
            else
            {
                return Key(Index);
            }
        }

    private:

        /// \brief Moves a slot on to its next epoch and names it.
        ///
        /// \param Index The slot being handed out, counted from one.
        /// \return The key naming that slot at its new epoch.
        ZY_INLINE constexpr Key Stamp(Slot Index)
        {
            if constexpr (EpochBits > 0)
            {
                return Key(Index, ++mEpochs[Index - 1]);
            }
            else
            {
                return Key(Index);
            }
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Bitset<Capacity>                             mMask;
        Slot                                         mHead;
        Slot                                         mTop;
        Slot                                         mSize;
        Array<Epoch, (EpochBits > 0 ? Capacity : 1)> mEpochs;
    };
}
