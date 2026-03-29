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

#include "Zyphryon.Base/Collection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A lightweight, bounded slot allocator with support for recycling.
    ///
    /// \tparam Capacity Maximum number of active slots.
    template<UInt32 Capacity>
    class Slot final
    {
        // TODO: Generation Counter

    public:

        /// \brief Constructs a new slot allocator with empty state.
        ZYPHRYON_INLINE explicit Slot()
            : mHead { 0 }
        {
        }

        /// \brief Allocates a new unique slot.
        ///
        /// \return A unique non-zero slot.
        ZYPHRYON_INLINE UInt32 Allocate()
        {
            UInt32 Handle;

            if (mPool.empty())
            {
                LOG_ASSERT(mHead < Capacity, "Attempted to allocate beyond maximum capacity");

                Handle = ++mHead;  // New sequential handle
            }
            else
            {
                Handle = mPool.back(); // Reuse from pool
                mPool.pop_back();
            }

            mMask.set(Handle - 1);
            return Handle;
        }

        /// \brief Releases a slot back to the allocator for reuse.
        ///
        /// \param Handle The slot to release (must be valid).
        ZYPHRYON_INLINE void Free(UInt32 Handle)
        {
            LOG_ASSERT(IsAllocated(Handle), "Attempted to free invalid slot");

            if (Handle == mHead)
            {
                --mHead;
            }
            else
            {
                mPool.emplace_back(Handle);
            }
            mMask.reset(Handle - 1);
        }

        /// \brief Clears all state and resets the allocator to empty.
        ZYPHRYON_INLINE void Clear()
        {
            mHead = 0;
            mPool.clear();
            mMask.reset();
        }

        /// \brief Marks a slot as acquired, preventing it from being reissued.
        ///
        /// \param Handle The slot to acquire (must be valid).
        ZYPHRYON_INLINE void Acquire(UInt32 Handle)
        {
            LOG_ASSERT(Handle != 0 && Handle <= Capacity, "Attempted to acquire invalid slot");

            if (Handle > mHead)
            {
                for (UInt32 Invalid = Handle - 1; Invalid > mHead; --Invalid)
                {
                    mPool.push_back(Invalid);
                }
                mHead = Handle;
            }
            else
            {
                if (const auto Iterator = std::ranges::find(mPool, Handle); Iterator != mPool.end())
                {
                    mPool.erase(Iterator);
                }
            }
            mMask.set(Handle - 1);
        }

        /// \brief Checks whether the allocator has reached its capacity.
        /// 
        /// \return `true` if the allocator is full, `false` otherwise.
        ZYPHRYON_INLINE Bool IsFull() const
        {
            return mPool.empty() && mHead == Capacity;
        }

        /// \brief Checks whether the allocator has no active slots.
        /// 
        /// \return `true` if the allocator is empty, `false` otherwise.
        ZYPHRYON_INLINE Bool IsEmpty() const
        {
            return mPool.empty() && mHead == 0;
        }

        /// \brief Checks whether a specific slot is currently allocated.
        ///
        /// \param Handle The slot to check for allocation status.
        /// \return `true` if the slot is allocated, `false` otherwise.
        ZYPHRYON_INLINE Bool IsAllocated(UInt32 Handle) const
        {
            return Handle > 0 && Handle <= mHead && mMask.test(Handle - 1);
        }

        /// \brief Returns the highest sequential slot ever issued.
        /// 
        /// \return The maximum issued slot value.
        ZYPHRYON_INLINE UInt32 GetHead() const
        {
            return mHead;
        }

        /// \brief Returns the number of currently active slots.
        /// 
        /// \return The total count of slots in use.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return mHead - mPool.size();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector<UInt32>   mPool;
        UInt32           mHead;
        Bitset<Capacity> mMask;
    };
}