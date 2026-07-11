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

#include "Zyphryon.Base/Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A fixed-size contiguous sequence container holding exactly \p Count elements of \p Type.
    template<typename Type, UInt Count>
    class Array final
    {
        static_assert(Count > 0, "Array size must be greater than zero");

    public:

        /// \brief The type of elements stored in the container.
        using Element = Type;

    public:

        /// \brief Constructs an array with all elements value-initialized.
        ZY_INLINE constexpr Array()
            : mData { }
        {
        }

        /// \brief Constructs an array from a list of values, forwarding each into the underlying storage.
        ///
        /// \param Parameters The values to initialize the array with, in order.
        template<typename... Arguments>
        ZY_INLINE constexpr Array(AnyRef<Arguments>... Parameters)
            requires (sizeof...(Arguments) > 0 && sizeof...(Arguments) <= Count)
            : mData { static_cast<Type>(Parameters)... }
        {
        }

        /// \brief Gets the number of elements in the array, which is always \p Count.
        ///
        /// \return The fixed element count.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return Count;
        }

        /// \brief Gets a pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element.
        ZY_INLINE constexpr Ptr<Type> GetData()
        {
            return mData;
        }

        /// \brief Gets a read-only pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element.
        ZY_INLINE constexpr ConstPtr<Type> GetData() const
        {
            return mData;
        }

        /// \brief Gets the first element in the array.
        ///
        /// \return A reference to the first element.
        ZY_INLINE constexpr Ref<Type> GetFront()
        {
            return mData[0];
        }

        /// \brief Gets the first element in the array.
        ///
        /// \return A reference to the first element.
        ZY_INLINE constexpr ConstRef<Type> GetFront() const
        {
            return mData[0];
        }

        /// \brief Gets the last element in the array.
        ///
        /// \return A reference to the last element.
        ZY_INLINE constexpr Ref<Type> GetBack()
        {
            return mData[Count - 1];
        }

        /// \brief Gets the last element in the array.
        ///
        /// \return A reference to the last element.
        ZY_INLINE constexpr ConstRef<Type> GetBack() const
        {
            return mData[Count - 1];
        }

        /// \brief Assigns the specified value to every element in the array.
        ///
        /// \param Value The value to assign to each element.
        ZY_INLINE constexpr void Fill(ConstRef<Type> Value)
        {
            ::Fill(mData, Count, Value);
        }

        /// \brief Finds the first occurrence of the specified element.
        ///
        /// \param Element The element to search for.
        /// \return The index of the first occurrence, or Count if not found.
        ZY_INLINE constexpr UInt Find(ConstRef<Type> Element) const
        {
            for (UInt Index = 0; Index < Count; ++Index)
            {
                if (mData[Index] == Element)
                {
                    return Index;
                }
            }
            return Count;
        }

        /// \brief Checks whether the array contains the specified element.
        ///
        /// \param Element The element to search for.
        /// \return `true` if the element is found, otherwise `false`.
        ZY_INLINE constexpr Bool Contains(ConstRef<Type> Element) const
        {
            return Find(Element) != Count;
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A reference to the requested element.
        ZY_INLINE constexpr Ref<Type> operator[](UInt Index)
        {
            ZY_ASSERT(Index < Count, "Array index is out of bounds");

            return mData[Index];
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A reference to the requested element.
        ZY_INLINE constexpr ConstRef<Type> operator[](UInt Index) const
        {
            ZY_ASSERT(Index < Count, "Array index is out of bounds");

            return mData[Index];
        }

        /// \brief Checks whether two arrays compare equal element-wise.
        ///
        /// \param Other The array to compare against.
        /// \return `true` if all elements are equal, otherwise `false`.
        ZY_INLINE constexpr Bool operator==(ConstRef<Array> Other) const
        {
            return Compare(mData, Other.mData, Count);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mData[Count];
    };

    /// \brief Deduction guide for constructing an Array from a parameter pack.
    ///
    /// \tparam Arguments The types of the arguments used to deduce the element type and count.
    template<typename... Arguments>
    Array(AnyRef<Arguments>...) -> Array<Common<Arguments...>, sizeof...(Arguments)>;
}