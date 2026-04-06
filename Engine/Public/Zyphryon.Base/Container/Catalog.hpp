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

#include "Slot.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A dynamically growable memory pool for efficient allocation and indexed access.
    template<typename Type, UInt32 Size>
    class Catalog final  // TODO: Merge with Pool
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

            // Ensure the storage vector is large enough to hold the new handle. If not, resize it.
            if (Handle >= mStorage.size())
            {
                mStorage.resize(Handle);
            }

            // Construct a new object directly in the storage slot for this handle.
            InPlaceConstruct<Type>(& mStorage[Handle - 1], Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates storage for a new object and constructs it in-place, passing the handle to the constructor.
        ///
        /// \param Parameters Arguments forwarded to the constructor.
        /// \return A unique non-zero handle.
        template<typename... Arguments>
        ZYPHRYON_INLINE UInt32 AllocateWithHandle(AnyRef<Arguments>... Parameters)
        {
            const UInt32 Handle = mAllocator.Allocate();

            // Ensure the storage vector is large enough to hold the new handle. If not, resize it.
            if (Handle >= mStorage.size())
            {
                mStorage.resize(Handle);
            }

            // Construct a new object directly in the storage slot for this handle.
            InPlaceConstruct<Type>(& mStorage[Handle - 1], Handle, Forward<Arguments>(Parameters)...);

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

            // Ensure the storage vector is large enough to hold the new handle. If not, resize it.
            if (Handle >= mStorage.size())
            {
                mStorage.resize(Handle);
            }

            // Construct a new object directly in the storage slot for this handle.
            InPlaceConstruct<Type>(& mStorage[Handle - 1], Forward<Arguments>(Parameters)...);
        }

        /// \brief Releases a previously allocated handle back to the pool.
        ///
        /// \param Handle A valid non-zero handle to free.
        ZYPHRYON_INLINE void Free(UInt32 Handle)
        {
            mAllocator.Free(Handle);

            /// Destroy the object stored at this handle in place before recycling.
            InPlaceDelete<Type>(mStorage[Handle - 1]);
        }

        /// \brief Resets the pool, clearing all allocations.
        ZYPHRYON_INLINE void Clear()
        {
            for (UInt32 Handle = 1; Handle <= mAllocator.GetHead(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    InPlaceDelete<Type>(mStorage[Handle - 1]);
                }
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

        /// \brief Iterates over each allocated object in the pool, invoking the provided callback.
        ///
        /// \param Function The callback function to invoke for each allocated object.
        template<typename Callback>
        ZYPHRYON_INLINE void ForEach(AnyRef<Callback> Function)
        {
            for (UInt32 Handle = 1; Handle <= mAllocator.GetHead(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    Function(mStorage[Handle - 1]);
                }
            }
        }

        /// \brief Iterates over each allocated object in the pool, invoking the provided callback.
        ///
        /// \param Function The callback function to invoke for each allocated object.
        template<typename Callback>
        ZYPHRYON_INLINE void ForEach(AnyRef<Callback> Function) const
        {
            for (UInt32 Handle = 1; Handle <= mAllocator.GetHead(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    Function(mStorage[Handle - 1]);
                }
            }
        }

        /// \brief Accesses the element associated with the given handle.
        ///
        /// \param Handle The handle of the element to access. Must be a valid, allocated handle.
        /// \return A reference to the element associated with the handle.
        ZYPHRYON_INLINE Ref<Type> operator[](UInt32 Handle)
        {
            LOG_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range handle");

            return mStorage[Handle - 1];
        }

        /// \brief Accesses the element associated with the given handle.
        ///
        /// \param Handle The handle of the element to access. Must be a valid, allocated handle.
        /// \return A reference to the element associated with the handle.
        ZYPHRYON_INLINE ConstRef<Type> operator[](UInt32 Handle) const
        {
            LOG_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range handle");

            return mStorage[Handle - 1];
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mAllocator);

            // Ensure the storage vector is appropriately sized when deserializing.
            if constexpr (Serializer::IsReader)
            {
                mStorage.resize(mAllocator.GetHead());
            }

            // Serialize each allocated element in the storage vector.
            ForEach([&](Ref<Type> Element)
            {
                Archive.SerializeObject(Element);
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Slot<Size>   mAllocator;
        Vector<Type> mStorage;
    };
}