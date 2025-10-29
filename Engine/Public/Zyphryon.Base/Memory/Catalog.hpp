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

#include "Handles.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A dynamically growable memory pool for efficient allocation and indexed access.
    template<typename Type, UInt32 Size>
    class Catalog final // TODO: Merge with Pool
    {
    public:

        /// \brief Allocates storage for a new object and constructs it in-place.
        ///
        /// \param Parameters Arguments forwarded to the constructor.
        /// \return A unique non-zero handle.
        template<typename... Arguments>
        ZYPHRYON_INLINE UInt32 Allocate(AnyRef<Arguments>... Parameters)
        {
            const UInt32 Handle = mAllocator.Allocate();

            if (Handle >= mPool.size())
            {
                /// Expand the pool to accommodate the new handle.
                mPool.emplace_back(Forward<Arguments>(Parameters)...);
            }
            else
            {
                /// Construct a new object directly in the storage slot for this handle.
                InPlaceConstruct<Type>(mPool[Handle - 1], Forward<Arguments>(Parameters)...);
            }
            return Handle;
        }

        /// \brief Acquires an existing handle without allocating a new one, constructing the object in-place.
        ///
        /// \param Handle     A valid non-zero handle previously allocated.
        /// \param Parameters Arguments forwarded to the constructor.
        template<typename... Arguments>
        ZYPHRYON_INLINE void Acquire(UInt32 Handle, AnyRef<Arguments>... Parameters)
        {
            mAllocator.Acquire(Handle);

            if (Handle >= mPool.size())
            {
                mPool.resize(Handle);
            }

            /// Construct a new object directly in the storage slot for this handle.
            InPlaceConstruct<Type>(mPool[Handle - 1], Forward<Arguments>(Parameters)...);
        }

        /// \brief Releases a previously allocated handle back to the pool.
        ///
        /// \param Handle A valid non-zero handle to free.
        ZYPHRYON_INLINE void Free(UInt32 Handle)
        {
            mAllocator.Free(Handle);

            /// Destroy the object stored at this handle in place before recycling.
            InPlaceDelete<Type>(mPool[Handle - 1]);
        }

        /// \brief Resets the pool, clearing all allocations.
        ZYPHRYON_INLINE void Clear()
        {
            for (Ref<Type> Element : GetSpan())
            {
                InPlaceDelete<Type>(Element);   // TODO: Check if element is valid?
            }
            mAllocator.Clear();
        }

        /// \brief Checks if all slots are currently in use.
        ///
        /// \return `true` if no further allocations are possible, otherwise `false`.
        ZYPHRYON_INLINE Bool IsFull() const
        {
            return mAllocator.IsFull();
        }

        /// \brief Checks if no objects are currently allocated.
        ///
        /// \return `true` if the pool is empty, otherwise `false`.
        ZYPHRYON_INLINE Bool IsEmpty() const
        {
            return mAllocator.IsEmpty();
        }

        /// \brief Returns the highest sequential handle ever issued.
        ///
        /// \return The maximum issued handle value.
        ZYPHRYON_INLINE UInt32 GetHead() const
        {
            return mAllocator.GetHead();
        }

        /// \brief Gets the current number of active (allocated) handles.
        ///
        /// \return The number of objects in use.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return mAllocator.GetSize();
        }

        /// \brief Returns a contiguous read-only view of the pool storage.
        ///
        /// \return A span providing read-only access to the pool storage.
        ZYPHRYON_INLINE Span<Type> GetSpan()
        {
            return Span<Type>(mPool.data(), GetHead());
        }

        /// \brief Returns a contiguous read-only view of the pool storage.
        ///
        /// \return A span providing read-only access to the pool storage.
        ZYPHRYON_INLINE ConstSpan<Type> GetSpan() const
        {
            return ConstSpan<Type>(mPool.data(), GetHead());
        }

        /// \brief Accesses the element associated with the given handle.
        ///
        /// \param Handle The handle of the element to access. Must be a valid, allocated handle.
        /// \return A reference to the element associated with the handle.
        ZYPHRYON_INLINE Ref<Type> operator[](UInt32 Handle)
        {
            LOG_ASSERT(Handle && Handle <= GetHead(), "Attempted to access invalid or out-of-range handle");

            return mPool[Handle - 1];
        }

        /// \brief Accesses the element associated with the given handle.
        ///
        /// \param Handle The handle of the element to access. Must be a valid, allocated handle.
        /// \return A reference to the element associated with the handle.
        ZYPHRYON_INLINE ConstRef<Type> operator[](UInt32 Handle) const
        {
            LOG_ASSERT(Handle && Handle <= GetHead(), "Attempted to access invalid or out-of-range handle");

            return mPool[Handle - 1];
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mAllocator);

            for (UInt32 Element = 0, Limit = mAllocator.GetHead(); Element < Limit; ++Element)
            {
                Archive.SerializeObject(mPool[Element]);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handles<Size> mAllocator;
        Vector<Type>  mPool;
    };
}