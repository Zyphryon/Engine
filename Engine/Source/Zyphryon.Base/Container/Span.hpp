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
    /// \brief Concept satisfied when \p Type exposes a contiguous element sequence via \c GetData() and \c GetSize().
    template<typename Type>
    concept IsContiguous   = requires (ConstRef<Type> Value)
    {
        { Value.GetData() };
        { Value.GetSize() };
    };

    /// \brief Concept satisfied when \p Type exposes a contiguous element sequence via \c GetData() and \c GetSize().
    template<typename Type, typename Element>
    concept IsContiguousOf = IsContiguous<Type> && requires (ConstRef<Type> Value)
    {
        { Value.GetData() } -> IsCastable<ConstPtr<Element>>;
    };
    
    /// \brief A non-owning view over a contiguous sequence of elements of \p Type.
    template<typename Type>
    class Span final
    {
    public:

        /// \brief Constructs an empty span.
        ZY_INLINE constexpr Span()
            : mData { nullptr },
              mSize { 0u }
        {
        }

        /// \brief Constructs a span from a pointer and an element count.
        ///
        /// \param Data The pointer to the first element.
        /// \param Size The number of elements in the view.
        ZY_INLINE constexpr Span(Ptr<Type> Data, UInt Size)
            : mData { Data },
              mSize { Size }
        {
        }

        /// \brief Constructs a span from a pointer range.
        ///
        /// \param First The pointer to the first element.
        /// \param Last  The pointer one past the last element.
        ZY_INLINE constexpr Span(Ptr<Type> First, Ptr<Type> Last)
            : mData { First },
              mSize { static_cast<UInt>(Last - First) }
        {
        }

        /// \brief Constructs a span from any contiguous container that satisfies \c IsContiguous.
        ///
        /// \param Collection The container whose elements to view.
        template<typename Container>
        ZY_INLINE constexpr Span(Ref<Container> Collection)
            requires IsContiguousOf<Container, Type>
            : mData { Collection.GetData() },
              mSize { Collection.GetSize() }
        {
        }

        /// \brief Constructs a span from any contiguous container that satisfies \c IsContiguous.
        ///
        /// \param Collection The container whose elements to view.
        template<typename Container>
         ZY_INLINE constexpr Span(ConstRef<Container> Collection)
             requires IsContiguousOf<Container, Type>
             : mData { Collection.GetData() },
               mSize { Collection.GetSize() }
        {
        }

        /// \brief Constructs a span from a single object, treating it as a sequence of one element.
        ///
        /// \param Object The object to view as a single-element span.
        ZY_INLINE constexpr Span(Ref<Type> Object)
            : mData { AddressOf(Object) },
              mSize { 1 }
        {
        }

        /// \brief Constructs a span from an array.
        ///
        /// \param Data The array whose elements to view.
        template<UInt Size>
        ZY_INLINE constexpr Span(Type (& Data)[Size])
            : mData { Data },
              mSize { Size }
        {
        }

        /// \brief Gets a pointer to the first element in the viewed sequence.
        ///
        /// \return A pointer to the first element, or \c nullptr when the span is empty.
        ZY_INLINE constexpr Ptr<Type> GetData() const
        {
            return mData;
        }

        /// \brief Gets the first element in the span.
        ///
        /// \return A reference to the first element.
        ZY_INLINE constexpr Ref<Type> GetFront() const
        {
            ZY_ASSERT(!IsEmpty(), "Span is empty");

            return mData[0];
        }

        /// \brief Gets the last element in the span.
        ///
        /// \return A reference to the last element.
        ZY_INLINE constexpr Ref<Type> GetBack() const
        {
            ZY_ASSERT(!IsEmpty(), "Span is empty");

            return mData[mSize - 1];
        }

        /// \brief Checks whether the span contains no elements.
        ///
        /// \return `true` if the span is empty, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mSize == 0u;
        }

        /// \brief Gets the number of elements in the viewed sequence.
        ///
        /// \return The element count.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the size of the viewed sequence in bytes.
        ///
        /// \return The total byte count of all elements.
        ZY_INLINE constexpr UInt GetSizeInBytes() const
        {
            return mSize * sizeof(Type);
        }

        /// \brief Returns a sub-span starting at the given offset.
        ///
        /// \param Offset The zero-based index of the first element to include.
        /// \param Count  The number of elements to include. When zero, the sub-span extends to the end.
        /// \return A span over the requested sub-range.
        ZY_INLINE constexpr Span Slice(UInt Offset, UInt Count = 0u) const
        {
            ZY_ASSERT(Offset <= mSize, "Slice offset is out of bounds");

            const UInt Length = (Count == 0u ? mSize - Offset : Count);
            ZY_ASSERT(Offset + Length <= mSize, "Slice range exceeds span bounds");

            return Span(mData + Offset, Length);
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The index of the element to access.
        /// \return A reference to the requested element.
        ZY_INLINE constexpr Ref<Type> operator[](UInt Index) const
        {
            ZY_ASSERT(Index < mSize, "Span index is out of bounds");

            return mData[Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mData;
        UInt      mSize;
    };

    /// \brief A non-owning read-only view over a contiguous sequence of elements of \p Type.
    template<typename Type>
    using ConstSpan = Span<const Type>;
}