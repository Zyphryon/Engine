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
    ///
    /// \tparam Type      The object the pool holds.
    /// \tparam Capacity  The number of objects the pool can hold.
    /// \tparam EpochBits The number of bits given to each slot's epoch, or zero to track none.
    template<typename Type, UInt Capacity, UInt EpochBits = 16>
    class Pool final
    {
    public:

        /// \brief The key naming one object at the epoch its slot was handed out at.
        using Key  = typename Freelist<Capacity, EpochBits>::Key;

        /// \brief The integer a slot is counted in.
        using Slot = typename Freelist<Capacity, EpochBits>::Slot;

    public:

        /// \brief Constructs an empty pool with no allocated objects.
        ZY_INLINE constexpr Pool()
            : mAllocator { }
        {
        }

        /// \brief Destroys all objects and resets the pool to an empty state.
        ZY_INLINE constexpr void Clear()
        {
            if constexpr (!IsTriviallyDestructible<Type>)
            {
                for (UInt Index = 1; Index <= mAllocator.GetTop(); ++Index)
                {
                    if (mAllocator.IsOccupied(static_cast<Slot>(Index)))
                    {
                        mStorage.Destruct(Index - 1);
                    }
                }
            }
            mAllocator.Clear();
        }

        /// \brief Gets the highest slot that has been allocated.
        ///
        /// \return The highest slot that has been allocated.
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
        /// \return A key naming the newly allocated object.
        template<typename... Arguments>
        ZY_INLINE constexpr Key Allocate(AnyRef<Arguments>... Parameters)
        {
            const Key Handle = mAllocator.Allocate();

            mStorage.Construct(Handle.GetSlot() - 1, Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates a new object in the pool, passing its key as the first constructor argument.
        ///
        /// \param Parameters The constructor arguments to forward to \p Type after the key.
        /// \return A key naming the newly allocated object.
        template<typename... Arguments>
        ZY_INLINE constexpr Key AllocateWithHandle(AnyRef<Arguments>... Parameters)
        {
            const Key Handle = mAllocator.Allocate();

            mStorage.Construct(Handle.GetSlot() - 1, Handle, Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates an object at a specific slot and constructs it with the given arguments.
        ///
        /// \param Index      The slot at which to allocate the object, counted from one.
        /// \param Parameters The constructor arguments to forward to \p Type.
        /// \return A key naming the newly allocated object.
        template<typename... Arguments>
        ZY_INLINE constexpr Key Acquire(Slot Index, AnyRef<Arguments>... Parameters)
        {
            const Key Handle = mAllocator.Acquire(Index);

            mStorage.Construct(Index - 1, Forward<Arguments>(Parameters)...);

            return Handle;
        }

        /// \brief Allocates an object at the slot a key names, taking the key's epoch rather than minting one.
        ///
        /// \param Handle     The key naming where to allocate the object.
        /// \param Parameters The constructor arguments to forward to \p Type.
        template<typename... Arguments>
        ZY_INLINE constexpr void Acquire(Key Handle, AnyRef<Arguments>... Parameters)
        {
            mAllocator.Adopt(Handle);

            mStorage.Construct(Handle.GetSlot() - 1, Forward<Arguments>(Parameters)...);
        }

        /// \brief Frees the object a key names, destroying it and releasing its slot.
        ///
        /// \param Handle The key naming the object to free.
        ZY_INLINE constexpr void Free(Key Handle)
        {
            mAllocator.Free(Handle);

            mStorage.Destruct(Handle.GetSlot() - 1);
        }

        /// \brief Frees whatever occupies a slot, whichever key named it.
        ///
        /// \param Index The slot to free, counted from one.
        ZY_INLINE constexpr void Release(Slot Index)
        {
            mAllocator.Release(Index);

            mStorage.Destruct(Index - 1);
        }

        /// \brief Checks whether a key still names the object in its slot.
        ///
        /// \param Handle The key to check.
        /// \return `true` if the key names an allocated object, otherwise `false`.
        ZY_INLINE constexpr Bool IsAllocated(Key Handle) const
        {
            return mAllocator.IsAllocated(Handle);
        }

        /// \brief Checks whether a slot holds an object, whichever key names it.
        ///
        /// \param Index The slot to check, counted from one.
        /// \return `true` if the slot is allocated, otherwise `false`.
        ZY_INLINE constexpr Bool IsOccupied(Slot Index) const
        {
            return mAllocator.IsOccupied(Index);
        }

        /// \brief Gets the key naming whatever occupies a slot.
        ///
        /// \param Index The slot to name, counted from one.
        /// \return The key naming that slot at its current epoch.
        ZY_INLINE constexpr Key GetKey(Slot Index) const
        {
            return mAllocator.GetKey(Index);
        }

        /// \brief Attempts to get a pointer to the object a key names.
        ///
        /// \param Handle The key naming the object to get.
        /// \return A pointer to the object, or `nullptr` when the key no longer names it.
        ZY_INLINE constexpr Ptr<Type> TryGet(Key Handle)
        {
            if (mAllocator.IsAllocated(Handle))
            {
                return mStorage.GetAddress(Handle.GetSlot() - 1);
            }
            return nullptr;
        }

        /// \brief Attempts to get a const pointer to the object a key names.
        ///
        /// \param Handle The key naming the object to get.
        /// \return A const pointer to the object, or `nullptr` when the key no longer names it.
        ZY_INLINE constexpr ConstPtr<Type> TryGet(Key Handle) const
        {
            if (mAllocator.IsAllocated(Handle))
            {
                return mStorage.GetAddress(Handle.GetSlot() - 1);
            }
            return nullptr;
        }

        /// \brief Invokes a callback for each allocated object in the pool.
        ///
        /// \param Callback The function to invoke for each allocated object.
        template<typename Callable>
        ZY_INLINE constexpr void ForEach(AnyRef<Callable> Callback)
        {
            for (UInt Index = 1; Index <= mAllocator.GetTop(); ++Index)
            {
                if (mAllocator.IsOccupied(static_cast<Slot>(Index)))
                {
                    Callback(mStorage[Index - 1]);
                }
            }
        }

        /// \brief Invokes a callback for each allocated object in the pool.
        ///
        /// \param Callback The function to invoke for each allocated object.
        template<typename Callable>
        ZY_INLINE constexpr void ForEach(AnyRef<Callable> Callback) const
        {
            for (UInt Index = 1; Index <= mAllocator.GetTop(); ++Index)
            {
                if (mAllocator.IsOccupied(static_cast<Slot>(Index)))
                {
                    Callback(mStorage[Index - 1]);
                }
            }
        }

        /// \brief Frees every allocated object for which \p Predicate returns `true`.
        ///
        /// \param Predicate The unary predicate used to identify the objects to free.
        /// \return The number of objects freed.
        template<typename Callable>
        ZY_INLINE constexpr UInt ForEachOrRemove(AnyRef<Callable> Predicate)
        {
            UInt Result = 0;

            for (UInt Index = 1; Index <= mAllocator.GetTop(); ++Index)
            {
                if (mAllocator.IsOccupied(static_cast<Slot>(Index)) && Predicate(mStorage[Index - 1]))
                {
                    Release(static_cast<Slot>(Index));

                    ++Result;
                }
            }
            return Result;
        }

        /// \brief Gets a reference to the object a key names.
        ///
        /// \param Handle The key naming the object to access.
        /// \return A reference to the object \p Handle names.
        ZY_INLINE constexpr Ref<Type> operator[](Key Handle)
        {
            ZY_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range key");

            return mStorage[Handle.GetSlot() - 1];
        }

        /// \brief Gets a const reference to the object a key names.
        ///
        /// \param Handle The key naming the object to access.
        /// \return A const reference to the object \p Handle names.
        ZY_INLINE constexpr ConstRef<Type> operator[](Key Handle) const
        {
            ZY_ASSERT(mAllocator.IsAllocated(Handle), "Attempted to access invalid or out-of-range key");

            return mStorage[Handle.GetSlot() - 1];
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mAllocator);

            // Serialize each element in the storage array.
            for (UInt Index = 1; Index <= mAllocator.GetTop(); ++Index)
            {
                if (mAllocator.IsOccupied(static_cast<Slot>(Index)))
                {
                    if constexpr (Serializer::IsReader)
                    {
                        mStorage.Construct(Index - 1);
                    }
                    Archive.Serialize(mStorage[Index - 1]);
                }
            }
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Freelist<Capacity, EpochBits> mAllocator;
        Storage<Type, Capacity>       mStorage;
    };
}
