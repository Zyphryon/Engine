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

#include "Atomic.hpp"
#include "Zyphryon.Base/Container/Array.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Lock-free single-producer / single-consumer ring buffer of fixed capacity.
    ///
    /// \tparam Capacity The maximum number of elements the ring can hold. Must be a power of two.
    template<typename Type, UInt32 Capacity>
    class Ring final
    {
    public:

        static_assert(Capacity > 0 && (Capacity & (Capacity - 1)) == 0, "Ring capacity must be a power of two");

    public:

        /// \brief Constructs an empty ring.
        ZY_INLINE Ring()
            : mWrite { 0 },
              mRead  { 0 }
        {
        }

        /// \brief Attempts to push a value onto the ring (producer side).
        ///
        /// \param Value The value to enqueue.
        /// \return `true` if the value was enqueued, `false` if the ring is full.
        ZY_INLINE Bool Push(ConstRef<Type> Value)
        {
            const UInt32 Write = mWrite.load(std::memory_order_relaxed);

            if (Write - mRead.load(std::memory_order_acquire) >= Capacity)
            {
                return false;
            }

            mBuffer[Write & kMask] = Value;
            mWrite.store(Write + 1, std::memory_order_release);
            return true;
        }

        /// \brief Attempts to pop a value from the ring (consumer side).
        ///
        /// \param Value The destination that receives the dequeued value.
        /// \return `true` if a value was dequeued, `false` if the ring is empty.
        ZY_INLINE Bool Pop(Ref<Type> Value)
        {
            const UInt32 Read = mRead.load(std::memory_order_relaxed);

            if (Read == mWrite.load(std::memory_order_acquire))
            {
                return false;
            }

            Value = mBuffer[Read & kMask];
            mRead.store(Read + 1, std::memory_order_release);
            return true;
        }

        /// \brief Checks whether the ring currently holds no elements.
        ///
        /// \return `true` if the ring is empty, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mRead.load(std::memory_order_acquire) == mWrite.load(std::memory_order_acquire);
        }

    private:

        /// \brief
        static constexpr UInt32 kMask = Capacity - 1;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZY_ALIGN(64) Atomic<UInt32> mWrite;
        ZY_ALIGN(64) Atomic<UInt32> mRead;
        Array<Type, Capacity>       mBuffer;
    };
}
