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
    /// \brief A union providing uninitialized, contiguous storage for \p Count elements of \p Type.
    template<typename Type, UInt Count>
    class Storage final
    {
        static_assert(Count > 0, "Storage size must be greater than zero");

    public:

        /// \brief Constructs the union, leaving the internal storage uninitialized.
        ZY_INLINE constexpr Storage()
        {
            if consteval
            {
                for (UInt Index = 0; Index < Count; ++Index)
                {
                    Construct(Index);
                }
            }
        }

        /// \brief Destructs the union.
        ZY_INLINE constexpr ~Storage()
        {

        }

        /// \brief Gets a pointer to the contiguous element storage.
        ///
        /// \return A non-owning pointer to the first slot.
        ZY_INLINE constexpr Ptr<Type> GetData()
        {
            return mData;
        }

        /// \brief Gets a const pointer to the contiguous element storage.
        ///
        /// \return A non-owning const pointer to the first slot.
        ZY_INLINE constexpr ConstPtr<Type> GetData() const
        {
            return mData;
        }

        /// \brief Gets a pointer to the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning pointer to the slot.
        ZY_INLINE constexpr Ptr<Type> GetAddress(UInt Index)
        {
            return AddressOf(mData[Index]);
        }

        /// \brief Gets a const pointer to the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning const pointer to the slot.
        ZY_INLINE constexpr ConstPtr<Type> GetAddress(UInt Index) const
        {
            return AddressOf(mData[Index]);
        }

        /// \brief Gets a reference to the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning reference to the slot.
        ZY_INLINE constexpr Ref<Type> GetValue(UInt Index)
        {
            return mData[Index];
        }

        /// \brief Gets a const reference to the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning const reference to the slot.
        ZY_INLINE constexpr ConstRef<Type> GetValue(UInt Index) const
        {
            return mData[Index];
        }

        /// \brief Constructs the element at the specified index in place.
        ///
        /// \param Index      The zero-based index of the slot to construct.
        /// \param Parameters Forwarded arguments used to construct the value.
        /// \return A non-owning reference to the constructed value.
        template<typename... Arguments>
        ZY_INLINE constexpr Ref<Type> Construct(UInt Index, AnyRef<Arguments>... Parameters)
        {
            return ::Construct<Type>(GetAddress(Index), Forward<Arguments>(Parameters)...);
        }

        /// \brief Destructs the element at the specified index in place.
        ///
        /// \param Index The zero-based index of the slot to destruct.
        ZY_INLINE constexpr void Destruct(UInt Index)
        {
            ::Destruct(mData[Index]);
        }

        /// \brief Destructs a range of elements starting at the specified index.
        ///
        /// \param Offset The zero-based index of the first element to destruct.
        /// \param Length The number of elements to destruct.
        ZY_INLINE constexpr void Destruct(UInt Offset, UInt Length)
        {
            if constexpr (!IsTriviallyCopyable<Type>)
            {
                for (UInt Index = 0; Index < Length; ++Index)
                {
                    ::Destruct(mData[Offset + Index]);
                }
            }
        }

        /// \brief Swaps the elements at the specified indices.
        ///
        /// \param Source      The zero-based index of the source element.
        /// \param Destination The zero-based index of the destination element.
        ZY_INLINE constexpr void Swap(UInt Source, UInt Destination)
        {
            mData[Destination] = ::Move(mData[Source]);
        }

        /// \brief Swaps a range of elements starting at the specified indices.
        ///
        /// \param Offset The zero-based index of the first element to swap.
        /// \param Target The zero-based index of the first element to swap with.
        /// \param Size   The number of elements to swap.
        ZY_INLINE constexpr void Swap(UInt Offset, UInt Target, UInt Size)
        {
            if constexpr (IsTriviallyCopyable<Type>)
            {
                if consteval
                {
                    for (UInt Index = 0; Index < Size; ++Index)
                    {
                        mData[Target + Index] = mData[Offset + Index];
                    }
                }
                else
                {
                    ::Copy(GetAddress(Target), Size, GetAddress(Offset));
                }
            }
            else
            {
                for (UInt Index = 0; Index < Size; ++Index)
                {
                    mData[Target + Index] = ::Move(mData[Offset + Index]);
                }
            }
        }

        /// \brief Move-assigns a range of elements backward; safe when \p Target > \p Offset (right shift, overlapping).
        ///
        /// \param Offset The zero-based index of the first source element.
        /// \param Target The zero-based index of the first destination element.
        /// \param Size   The number of elements to move.
        ZY_INLINE constexpr void Slide(UInt Offset, UInt Target, UInt Size)
        {
            if constexpr (IsTriviallyCopyable<Type>)
            {
                if consteval
                {
                    for (UInt Index = Size; Index > 0; --Index)
                    {
                        mData[Target + Index - 1] = mData[Offset + Index - 1];
                    }
                }
                else
                {
                    ::Copy(GetAddress(Target), Size, GetAddress(Offset));
                }
            }
            else
            {
                for (UInt Index = Size; Index > 0; --Index)
                {
                    mData[Target + Index - 1] = ::Move(mData[Offset + Index - 1]);
                }
            }
        }

        /// \brief Copies a range of elements from a source pointer into this storage at the specified offset.
        ///
        /// \param Offset The zero-based index of the first destination slot.
        /// \param Source The non-owning pointer to the source elements.
        /// \param Length The number of elements to copy.
        ZY_INLINE constexpr void Copy(UInt Offset, ConstPtr<Type> Source, UInt Length)
        {
            if constexpr (IsTriviallyCopyable<Type>)
            {
                if consteval
                {
                    for (UInt Index = 0; Index < Length; ++Index)
                    {
                        mData[Index + Offset] = Source[Index];
                    }
                }
                else
                {
                    ::Copy(GetAddress(Offset), Length, Source);
                }
            }
            else
            {
                for (UInt Index = 0; Index < Length; ++Index)
                {
                    ::Construct<Type>(GetAddress(Index + Offset), Source[Index]);
                }
            }
        }

        /// \brief Relocates a range of elements from another storage to this storage.
        ///
        /// \param Other  The source storage.
        /// \param Offset The zero-based index of the first element to relocate from the source storage.
        /// \param Target The zero-based index of the first element to relocate to in this storage.
        /// \param Size   The number of elements to relocate.
        ZY_INLINE constexpr void Relocate(Ref<Storage> Other, UInt Offset, UInt Target, UInt Size)
        {
            if constexpr (IsTriviallyCopyable<Type>)
            {
                if consteval
                {
                    for (UInt Index = 0; Index < Size; ++Index)
                    {
                        mData[Target + Index] = Other.mData[Offset + Index];
                    }
                }
                else
                {
                    ::Copy(GetAddress(Target), Size, Other.GetAddress(Offset));
                }
            }
            else
            {
                for (UInt Index = 0; Index < Size; ++Index)
                {
                    ::Relocate<Type>(mData[Target + Index], ::Move(Other.GetValue(Offset + Index)));
                }
            }
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning reference to the slot.
        ZY_INLINE constexpr Ref<Type> operator[](UInt Index)
        {
            return mData[Index];
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the slot.
        /// \return A non-owning const reference to the slot.
        ZY_INLINE constexpr ConstRef<Type> operator[](UInt Index) const
        {
            return mData[Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Type mData[Count];
    };
}