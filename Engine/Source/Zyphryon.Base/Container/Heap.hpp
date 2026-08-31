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

#include "Sequence.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a binary heap keeping its smallest element at the front.
    ///
    /// \tparam Type  The element type, ordered by `operator<`.
    /// \tparam Count The fixed capacity, or zero to grow on demand.
    template<typename Type, UInt Count = 0>
    class Heap final
    {
    public:

        /// \brief Constructs a heap holding nothing.
        ZY_INLINE Heap() = default;

        /// \brief Checks whether the heap holds no element at all.
        ///
        /// \return `true` when the heap is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mData.IsEmpty();
        }

        /// \brief Checks whether the heap holds as many elements as it can.
        ///
        /// \return `true` when no further element fits, otherwise `false`.
        ZY_INLINE Bool IsFull() const
        {
            if constexpr (Count > 0)
            {
                return mData.IsFull();
            }
            else
            {
                return false;
            }
        }

        /// \brief Gets how many elements the heap holds.
        ///
        /// \return The element count.
        ZY_INLINE UInt GetSize() const
        {
            return mData.GetSize();
        }

        /// \brief Reserves room for the given number of elements, so growing holds no allocation.
        ///
        /// \param Capacity The number of elements to make room for.
        ZY_INLINE void Reserve(UInt Capacity)
            requires (Count == 0)
        {
            mData.Reserve(Capacity);
        }

        /// \brief Drops every element, leaving the heap empty.
        ZY_INLINE void Clear()
        {
            mData.Clear();
        }

        /// \brief Adds an element, sifting it up until the heap holds again.
        ///
        /// \param Parameters The arguments the element is constructed from.
        template<typename... Arguments>
        ZY_INLINE void Push(AnyRef<Arguments>... Parameters)
        {
            mData.Append(Forward<Arguments>(Parameters)...);

            for (UInt Child = mData.GetSize() - 1; Child > 0; )
            {
                const UInt Parent = (Child - 1) / 2;

                if (!(mData[Child] < mData[Parent]))
                {
                    break;
                }

                Swap(mData[Child], mData[Parent]);
                Child = Parent;
            }
        }

        /// \brief Gets the smallest element the heap holds.
        ///
        /// \return The smallest element, which the heap must hold at least one of.
        ZY_INLINE ConstRef<Type> Peek() const
        {
            return mData[0];
        }

        /// \brief Takes the smallest element out, sifting the last one down in its place.
        ///
        /// \return The element that was smallest, which the heap must hold at least one of.
        ZY_INLINE Type Pop()
        {
            Type Smallest = Move(mData[0]);

            mData[0] = Move(mData[mData.GetSize() - 1]);
            mData.RemoveLast();

            for (UInt Parent = 0, Limit = mData.GetSize(); ; )
            {
                const UInt Left  = Parent * 2 + 1;
                const UInt Right = Left + 1;

                UInt Target = Parent;

                if (Left < Limit && mData[Left] < mData[Target])
                {
                    Target = Left;
                }

                if (Right < Limit && mData[Right] < mData[Target])
                {
                    Target = Right;
                }

                if (Target == Parent)
                {
                    break;
                }

                Swap(mData[Parent], mData[Target]);
                Parent = Target;
            }
            return Smallest;
        }

    private:

        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-
        // -=-=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=--=-

        Sequence<Type, Count> mData;
    };
}