// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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
    /// \brief A lightweight, bounded handle allocator with support for recycling.
    ///
    /// \tparam Capacity Maximum number of active handles.
    template<UInt32 Capacity>
    class Handle final
    {
    public:

        /// \brief Constructs a new handle allocator with empty state.
        ZYPHRYON_INLINE explicit Handle()
            : mHead { 0 }
        {
        }

        /// \brief Allocates a new unique handle.
        ///
        /// \return A unique non-zero handle.
        ZYPHRYON_INLINE UInt32 Allocate()
        {
            if (mPool.empty())
            {
                LOG_ASSERT(mHead < Capacity, "Attempted to allocate beyond maximum capacity");

                return ++mHead;
            }

            const UInt32 Handle = mPool.back();
            mPool.pop_back();
            return Handle;
        }

        /// \brief Releases a handle back to the allocator for reuse.
        ///
        /// \param Handle The handle to release (must be valid).
        ZYPHRYON_INLINE void Free(UInt32 Handle)
        {
            LOG_ASSERT(Handle && Handle <= mHead, "Attempted to free invalid handle");

            if (Handle == mHead)
            {
                --mHead;
            }
            else
            {
                mPool.emplace_back(Handle); // TODO: Check double free in debug version?
            }
        }

        /// \brief Clears all state and resets the allocator to empty.
        ZYPHRYON_INLINE void Clear()
        {
            mHead = 0;
            mPool.clear();
        }

        /// \brief Marks a handle as acquired, preventing it from being reissued.
        ///
        /// \param Handle The handle to acquire (must be valid or zero).
        ZYPHRYON_INLINE void Acquire(UInt32 Handle)
        {
            LOG_ASSERT(Handle != 0 && Handle <= Capacity, "Attempted to acquire invalid handle");

            if (Handle > mHead)
            {
                for (UInt32 Invalid = mHead + 1; Invalid < Handle; ++Invalid)
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
        }

        /// \brief Checks whether the allocator has reached its capacity.
        /// 
        /// \return `true` if the allocator is full, `false` otherwise.
        ZYPHRYON_INLINE Bool IsFull() const
        {
            return mPool.empty() && mHead == Capacity;
        }

        /// \brief Checks whether the allocator has no active handles.
        /// 
        /// \return `true` if the allocator is empty, `false` otherwise.
        ZYPHRYON_INLINE Bool IsEmpty() const
        {
            return mPool.empty() && mHead == 0;
        }

        /// \brief Returns the highest sequential handle ever issued.
        /// 
        /// \return The maximum issued handle value.
        ZYPHRYON_INLINE UInt32 GetHead() const
        {
            return mHead;
        }

        /// \brief Returns the number of currently active handles.
        /// 
        /// \return The total count of handles in use.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return mHead - mPool.size();
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeVector(mPool);
            Archive.SerializeInt(mHead);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector<UInt32> mPool;
        UInt32         mHead;
    };
}