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

inline namespace ZyBase
{
    /// \brief A sequence taken from the front by a cursor rather than by removing what is taken.
    template<typename Type, UInt Count = 0>
    class Spool final
    {
    public:

        /// \brief The type of elements stored in the container.
        using Element = Type;

    public:

        /// \brief Constructs a spool holding nothing.
        ZY_INLINE constexpr Spool()
            : mTaken { 0 }
        {
        }

        /// \brief Gets a read-only pointer to the element the cursor stands on.
        ///
        /// \return A pointer to the first element left, which is past the end when none are.
        ZY_INLINE constexpr ConstPtr<Type> GetData() const
        {
            return mItems.GetData() + mTaken;
        }

        /// \brief Gets how many elements are still to be taken.
        ///
        /// \return The number left, which counts nothing the cursor has already passed.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mItems.GetSize() - mTaken;
        }

        /// \brief Gets how many elements the spool may hold before it is wound full.
        ///
        /// \return The capacity, which counts what has been taken as still held.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return mItems.GetCapacity();
        }

        /// \brief Checks whether every element has been taken.
        ///
        /// \return `true` when the cursor has caught the end, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mTaken >= mItems.GetSize();
        }

        /// \brief Checks whether the spool is wound as full as it may be.
        ///
        /// \return `true` when nothing more may be appended, otherwise `false`.
        ZY_INLINE constexpr Bool IsFull() const
            requires (Count > 0)
        {
            return mItems.IsFull();
        }

        /// \brief Gets the element the cursor stands on, which is the next to be taken.
        ///
        /// \return The element, which the spool must hold at least one untaken.
        ZY_INLINE constexpr ConstRef<Type> Peek() const
        {
            ZY_ASSERT(!IsEmpty(), "Spool has nothing left to take");

            return mItems[mTaken];
        }

        /// \brief Gets the last element appended, taken or not.
        ///
        /// \return The element, which the spool must hold at least one of.
        ZY_INLINE constexpr ConstRef<Type> GetBack() const
        {
            return mItems.GetBack();
        }

        /// \brief Gets everything still to be taken, in the order it will be.
        ///
        /// \return The elements left, which is empty once the cursor has caught the end.
        ZY_INLINE constexpr ConstSpan<Type> GetRemaining() const
        {
            return ConstSpan<Type>(GetData(), GetSize());
        }

        /// \brief Constructs a new element at the end of the spool.
        ///
        /// \param Parameters The arguments to forward to the element's constructor.
        /// \return A reference to the newly appended element.
        template<typename... Arguments>
        ZY_INLINE constexpr Ref<Type> Append(AnyRef<Arguments>... Parameters)
        {
            return mItems.Append(Forward<Arguments>(Parameters)...);
        }

        /// \brief Appends multiple elements from a contiguous container to the end of the spool.
        ///
        /// \param Elements The elements to append.
        template<typename Container>
        ZY_INLINE constexpr void Append(AnyRef<Container> Elements)
            requires IsContiguousOf<Container, Type>
        {
            mItems.Append(Elements);
        }

        /// \brief Steps the cursor past the element it stands on.
        ZY_INLINE constexpr void Pop()
        {
            ZY_ASSERT(!IsEmpty(), "Spool has nothing left to take");

            ++mTaken;
        }

        /// \brief Drops everything the spool holds, taken or not.
        ZY_INLINE constexpr void Clear()
        {
            mItems.Clear();
            mTaken = 0;
        }

        /// \brief Drops what has been taken, so the room it held may be wound again.
        ZY_INLINE constexpr void Compact()
        {
            if (mTaken > 0)
            {
                mItems.Remove(0, mTaken);
                mTaken = 0;
            }
        }

        /// \brief Gets an element counted from the cursor rather than from the front.
        ///
        /// \param Index The offset from the element the cursor stands on.
        /// \return The element at that offset.
        ZY_INLINE constexpr ConstRef<Type> operator[](UInt Index) const
        {
            ZY_ASSERT(Index < GetSize(), "Spool index is out of bounds");

            return mItems[mTaken + Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Type, Count> mItems;
        UInt32                mTaken;
    };
}