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

#include "Bitset.hpp"
#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Tracks allocation state for a fixed-capacity set of handles using a free-list strategy.
    template<UInt Capacity>
    class Freelist final
    {
    public:

        /// \brief The type of the handle used to refer to allocated slots.
        using Handle = Integer<Capacity>;

    public:

        /// \brief Constructs a freelist with all slots initially free.
        ZY_INLINE constexpr Freelist()
            : mHead { 0 },
              mTop  { 0 },
              mSize { 0 }
        {
        }

        /// \brief Resets all slots to the free state.
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

        /// \brief Gets the highest currently-valid handle value, usable as an inclusive iteration bound.
        ///
        /// \return The highest allocated handle (1-based), or 0 if no slots are allocated.
        ZY_INLINE constexpr UInt GetTop() const
        {
            return mTop + 1;
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
        /// \return A handle to the newly allocated slot.
        ZY_INLINE constexpr Handle Allocate()
        {
            const Handle Found = mMask.FindFirstClear(mHead);
            ZY_ASSERT(Found < Capacity, "Attempted to allocate beyond maximum capacity");

            mMask.Set(Found);

            mHead = Found;
            mTop  = Max(mTop, Found);
            ++mSize;
            return Found + 1;
        }

        /// \brief Acquires a specific slot.
        ///
        /// \param ID The handle of the slot to acquire.
        ZY_INLINE constexpr void Acquire(Handle ID)
        {
            ZY_ASSERT(ID > 0 && ID <= Capacity, "Attempted to acquire invalid slot");
            ZY_ASSERT(!IsAllocated(ID), "Attempted to acquire already allocated slot");

            mMask.Set(ID - 1);
            mTop = Max(mTop, ID - 1);
            ++mSize;
        }

        /// \brief Frees the slot at the given handle.
        ///
        /// \param ID The handle of the slot to free.
        ZY_INLINE constexpr void Free(Handle ID)
        {
            ZY_ASSERT(IsAllocated(ID), "Attempted to free invalid slot");

            mMask.Reset(ID - 1);
            mHead = ID - 1;

            if (ID - 1 == mTop)
            {
                mTop = mMask.FindLastSet();
            }
            --mSize;
        }

        /// \brief Checks whether the given handle refers to an allocated slot.
        ///
        /// \param ID The handle to check.
        /// \return `true` if the slot is allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsAllocated(Handle ID) const
        {
            return ID > 0 && mMask.Test(ID - 1);
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Bitset<Capacity> mMask;
        Handle           mHead;
        Handle           mTop;
        Handle           mSize;
    };
}