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

#include "Freelist.hpp"
#include "Storage.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A fixed-capacity object pool that manages allocation and deallocation of objects.
    template<typename Type, UInt Capacity>
    class Pool final
    {
    public:

        /// \brief The type of handle used to reference allocated objects in the pool.
        using Handle = Freelist<Capacity>::Handle;

    public:

        /// \brief Constructs an empty pool with no allocated objects.
        ZY_INLINE constexpr Pool()
            : mAllocator { }
        {
        }

        /// \brief Destroys all objects and resets the pool to an empty state.
        ZY_INLINE constexpr void Clear()
        {
            if constexpr (!IsTriviallyCopyable<Type>)
            {
                for (Handle Handle = 1; Handle <= mAllocator.GetTop(); ++Handle)
                {
                    if (mAllocator.IsAllocated(Handle))
                    {
                        mStorage.Destruct(Handle - 1);
                    }
                }
            }
            mAllocator.Clear();
        }

        /// \brief Gets the highest handle value that has been allocated.
        ///
        /// \return The highest handle value that has been allocated.
        ZY_INLINE constexpr UInt GetTop() const
        {
            return mAllocator.GetTop();
        }

        /// \brief Gets the number of currently allocated objects.
        ///
        /// \return The number of currently allocated objects.
        ZY_INLINE constexpr UInt GetCount() const
        {
            return mAllocator.GetCount();
        }

        /// \brief Gets the maximum number of objects that can be allocated.
        ///
        /// \return The maximum number of objects that can be allocated.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return Capacity;
        }

        /// \brief Checks whether the pool has reached its maximum capacity.
        ///
        /// \return `true` if the pool is full, otherwise `false`.
        ZY_INLINE constexpr Bool IsFull() const
        {
            return mAllocator.IsFull();
        }

        /// \brief Checks whether the pool contains no allocated objects.
        ///
        /// \return `true` if the pool is empty, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mAllocator.IsEmpty();
        }

        /// \brief Allocates a new object in the pool and constructs it with the given arguments.
        ///
        /// \param Parameters The constructor arguments to forward to \p Type.
        /// \return A handle to the newly allocated object.
        template<typename... Arguments>
        ZY_INLINE constexpr Handle Allocate(AnyRef<Arguments>... Parameters)
        {
            const Handle Handle = mAllocator.Allocate();

            mStorage.Construct(Handle - 1, Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates a new object in the pool, passing the handle as the first constructor argument.
        ///
        /// \param Parameters The constructor arguments to forward to \p Type after the handle.
        /// \return A handle to the newly allocated object.
        template<typename... Arguments>
        ZY_INLINE constexpr Handle AllocateWithHandle(AnyRef<Arguments>... Parameters)
        {
            const Handle Handle = mAllocator.Allocate();

            mStorage.Construct(Handle - 1, Handle, Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates an object at a specific handle and constructs it with the given arguments.
        ///
        /// \param Handle     The handle at which to allocate the object.
        /// \param Parameters The constructor arguments to forward to \p Type.
        template<typename... Arguments>
        ZY_INLINE constexpr void Acquire(Handle Handle, AnyRef<Arguments>... Parameters)
        {
            mAllocator.Acquire(Handle);

            mStorage.Construct(Handle - 1, Forward<Arguments>(Parameters)...);
        }

        /// \brief Frees the object at the given handle, destroying it and releasing the slot.
        ///
        /// \param Handle The handle of the object to free.
        ZY_INLINE constexpr void Free(Handle Handle)
        {
            mAllocator.Free(Handle);

            mStorage.Destruct(Handle - 1);
        }

        /// \brief Checks whether the given handle refers to an allocated object.
        ///
        /// \param Handle The handle to check.
        /// \return `true` if the handle is allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsAllocated(Handle Handle) const
        {
            return mAllocator.IsAllocated(Handle);
        }

        /// \brief Attempts to get a pointer to the object at the given handle.
        ///
        /// \param Handle The handle of the object to get.
        /// \return A pointer to the object if allocated, otherwise `nullptr`.
        ZY_INLINE constexpr Ptr<Type> TryGet(Handle Handle)
        {
            if (mAllocator.IsAllocated(Handle))
            {
                return mStorage.GetAddress(Handle - 1);
            }
            return nullptr;
        }

        /// \brief Attempts to get a const pointer to the object at the given handle.
        ///
        /// \param Handle The handle of the object to get.
        /// \return A const pointer to the object if allocated, otherwise `nullptr`.
        ZY_INLINE constexpr ConstPtr<Type> TryGet(Handle Handle) const
        {
            if (mAllocator.IsAllocated(Handle))
            {
                return mStorage.GetAddress(Handle - 1);
            }
            return nullptr;
        }

        /// \brief Invokes a callback for each allocated object in the pool.
        ///
        /// \param Callback The function to invoke for each allocated object.
        template<typename Callable>
        ZY_INLINE constexpr void ForEach(AnyRef<Callable> Callback)
        {
            for (Handle Handle = 1; Handle <= mAllocator.GetTop(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    Callback(mStorage[Handle - 1]);
                }
            }
        }

        /// \brief Invokes a callback for each allocated object in the pool.
        ///
        /// \param Callback The function to invoke for each allocated object.
        template<typename Callable>
        ZY_INLINE constexpr void ForEach(AnyRef<Callable> Callback) const
        {
            for (Handle Handle = 1; Handle <= mAllocator.GetTop(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    Callback(mStorage[Handle - 1]);
                }
            }
        }

        /// \brief Gets a reference to the object at the given handle.
        ///
        /// \param Handle The handle of the object to access.
        /// \return A reference to the object at \p Handle.
        ZY_INLINE constexpr Ref<Type> operator[](Handle Handle)
        {
            ZY_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range handle");

            return mStorage[Handle - 1];
        }

        /// \brief Gets a const reference to the object at the given handle.
        ///
        /// \param Handle The handle of the object to access.
        /// \return A const reference to the object at \p Handle.
        ZY_INLINE constexpr ConstRef<Type> operator[](Handle Handle) const
        {
            ZY_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range handle");

            return mStorage[Handle - 1];
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mAllocator);

            // Serialize each element in the storage array.
            for (Handle Handle = 1; Handle <= mAllocator.GetTop(); ++Handle)
            {
                if (mAllocator.IsAllocated(Handle))
                {
                    Archive.Serialize(mStorage[Handle - 1]);
                }
            }
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Freelist<Capacity>      mAllocator;
        Storage<Type, Capacity> mStorage;
    };
}