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

#include "Span.hpp"
#include "Storage.hpp"
#include "Zyphryon.Base/Scalar.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A contiguous sequence of \p Type elements whose storage policy is controlled by \p Count.
    ///
    /// When \p Count is zero the sequence is heap-backed and grows dynamically.
    /// When \p Count is greater than zero all storage lives inside the object itself with a fixed capacity.
    template<typename Type, UInt Count = 0>
    class Sequence;

    /// \brief A heap-backed, dynamically growing contiguous sequence of \p Type elements.
    template<typename Type>
    class Sequence<Type, 0> final
    {
    public:

        /// \brief The type of elements stored in the container.
        using Element = Type;

    public:

        /// \brief Constructs an empty sequence with no allocated storage.
        ZY_INLINE Sequence()
            : mData     { nullptr },
              mSize     { 0 },
              mCapacity { 0 }
        {
        }

        /// \brief Constructs a sequence with the specified initial capacity.
        ///
        /// \param Capacity The initial capacity of the sequence.
        ZY_INLINE Sequence(UInt Capacity)
            : mData     { nullptr },
              mSize     { 0 },
              mCapacity { 0 }
        {
            Reserve(Capacity);
        }

        /// \brief Constructs a sequence as a copy of \p Other, duplicating all elements.
        ///
        /// \param Other The sequence to copy from.
        ZY_INLINE Sequence(ConstRef<Sequence> Other)
            : mData     { nullptr },
              mSize     { 0 },
              mCapacity { 0 }
        {
            Append(Other);
        }

        /// \brief Constructs a sequence by taking ownership of \p Other's storage.
        ///
        /// \param Other The sequence to move from. Left empty after the operation.
        ZY_INLINE Sequence(AnyRef<Sequence> Other)
            : mData     { Exchange(Other.mData,     nullptr) },
              mSize     { Exchange(Other.mSize,     0u) },
              mCapacity { Exchange(Other.mCapacity, 0u) }
        {
        }

        /// \brief Constructs a sequence by copying all elements from \p Collection.
        ///
        /// \param Collection The collection of elements to copy into the sequence.
        template<typename Container>
        ZY_INLINE constexpr Sequence(ConstRef<Container> Collection)
            requires IsContiguousOf<Container, Type>
            : mData     { nullptr },
              mSize     { 0 },
              mCapacity { 0 }
        {
            Append(Collection);
        }

        /// \brief Destroys all elements and releases the underlying heap storage.
        ZY_INLINE ~Sequence()
        {
            Clear();

            if (mData)
            {
                ::operator delete(mData);
            }
        }

        /// \brief Gets a pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element, or \c nullptr when empty.
        ZY_INLINE Ptr<Type> GetData()
        {
            return mData;
        }

        /// \brief Gets a read-only pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element, or \c nullptr when empty.
        ZY_INLINE ConstPtr<Type> GetData() const
        {
            return mData;
        }

        /// \brief Gets the number of elements currently stored in the sequence.
        ///
        /// \return The element count.
        ZY_INLINE UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the number of elements the sequence can hold without reallocating.
        ///
        /// \return The current capacity.
        ZY_INLINE UInt GetCapacity() const
        {
            return mCapacity;
        }

        /// \brief Checks whether the sequence contains no elements.
        ///
        /// \return `true` if the sequence is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mSize == 0;
        }

        /// \brief Gets the first element in the sequence.
        ///
        /// \return A reference to the first element.
        ZY_INLINE Ref<Type> GetFront()
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mData[0];
        }

        /// \brief Gets the first element in the sequence.
        ///
        /// \return A read-only reference to the first element.
        ZY_INLINE ConstRef<Type> GetFront() const
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mData[0];
        }

        /// \brief Gets the last element in the sequence.
        ///
        /// \return A reference to the last element.
        ZY_INLINE Ref<Type> GetBack()
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mData[mSize - 1];
        }

        /// \brief Gets the last element in the sequence.
        ///
        /// \return A read-only reference to the last element.
        ZY_INLINE ConstRef<Type> GetBack() const
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mData[mSize - 1];
        }

        /// \brief Constructs a new element at the end of the sequence.
        ///
        /// \param Parameters The arguments to forward to the element's constructor.
        /// \return A reference to the newly Appended element.
        template<typename... Arguments>
        ZY_INLINE Ref<Type> Append(AnyRef<Arguments>... Parameters)
        {
            if (mSize == mCapacity)
            {
                Grow(mCapacity > 0 ? mCapacity * 2 : 16);
            }
            return Construct<Type>(mData + mSize++, Forward<Arguments>(Parameters)...);
        }

        /// \brief Appends multiple elements from a contiguous container to the end of the sequence.
        ///
        /// \param Elements The container of elements to Append.
        template<typename Container>
        ZY_INLINE void Append(AnyRef<Container> Elements)
            requires IsContiguousOf<Container, Type>
        {
            const UInt Size = Elements.GetSize();

            if (const UInt Required = mSize + Size; Required > mCapacity)
            {
                Grow(mCapacity > 0 ? Max(mCapacity * 2, Required) : Max(16u, Required));
            }

            if constexpr (IsTriviallyCopyable<Type>)
            {
                Copy(mData + mSize, Size, Elements.GetData());
                mSize += Size;
            }
            else
            {
                for (UInt Index = 0; Index < Size; ++Index)
                {
                    Construct<Type>(mData + mSize++, Elements[Index]);
                }
            }
        }

        /// \brief Inserts a new element at the specified index, shifting subsequent elements right.
        ///
        /// \param Index The zero-based index where the element should be inserted.
        /// \param Element The arguments to forward to the element's constructor.
        template<typename... Arguments>
        ZY_INLINE Ref<Type> Insert(UInt Index, AnyRef<Arguments>... Element)
        {
            ZY_ASSERT(Index <= mSize, "Sequence insertion index is out of bounds");

            if (mSize == mCapacity)
            {
                Grow(mCapacity > 0 ? mCapacity * 2 : 16);
            }

            const Ptr<Type> Data = GetData();

            if constexpr (IsTriviallyCopyable<Type>)
            {
                Copy(Data + Index + 1, mSize - Index, Data + Index);
            }
            else
            {
                if (Index < mSize)
                {
                    Construct<Type>(Data + mSize, Move(Data[mSize - 1]));

                    for (UInt Offset = mSize - 1; Offset > Index; --Offset)
                    {
                        Data[Offset] = Move(Data[Offset - 1]);
                    }
                    Destruct<Type>(Data[Index]);
                }
            }

            ++mSize;
            return Construct<Type>(Data + Index, Forward<Arguments>(Element)...);
        }

        /// \brief Inserts multiple elements at the specified index, shifting subsequent elements right.
        ///
        /// \param Index    The zero-based index where elements should be inserted.
        /// \param Elements The span of elements to insert.
        template<typename Container>
        ZY_INLINE void Insert(UInt Index, AnyRef<Container> Elements)
            requires IsContiguousOf<Container, Type>
        {
            ZY_ASSERT(Index <= mSize, "Sequence insertion index is out of bounds");

            const UInt Size     = Elements.GetSize();
            const UInt Required = mSize + Size;

            if (Required > mCapacity)
            {
                Grow(mCapacity > 0 ? Max(mCapacity * 2, Required) : Max(16u, Required));
            }

            const Ptr<Type> Data = GetData();

            if constexpr (IsTriviallyCopyable<Type>)
            {
                Copy(Data + Index + Size, mSize - Index, Data + Index);
                Copy(Data + Index, Size, Elements.GetData());
            }
            else
            {
                for (UInt Offset = 0, Limit = mSize - Index; Offset < Limit; ++Offset)
                {
                    const UInt Source      = mSize - 1 - Offset;
                    const UInt Destination = mSize + Size - 1 - Offset;

                    if (Destination >= mSize)
                    {
                        Construct<Type>(Data + Destination, Move(Data[Source]));
                    }
                    else
                    {
                        Data[Destination] = Move(Data[Source]);
                    }
                }

                for (UInt Offset = 0; Offset < Size && Index + Offset < mSize; ++Offset)
                {
                    Destruct<Type>(Data[Index + Offset]);
                }

                for (UInt Offset = 0; Offset < Size; ++Offset)
                {
                    Construct<Type>(Data + Index + Offset, Elements[Offset]);
                }
            }

            mSize += Size;
        }

        /// \brief Removes the element at the given index, shifting subsequent elements left.
        ///
        /// \param Index The zero-based index of the element to remove.
        ZY_INLINE void Remove(UInt Index)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            if constexpr (IsTriviallyCopyable<Type>)
            {
                const Ptr<Type> Data = GetData() + Index;
                Copy(Data, mSize - Index - 1, Data + 1);
            }
            else
            {
                for (UInt Element = Index + 1; Element < mSize; ++Element)
                {
                    mData[Element - 1] = Move(mData[Element]);
                }
                Destruct<Type>(mData[mSize - 1]);
            }

            --mSize;
        }

        /// \brief Removes a range of elements starting at the given index, shifting subsequent elements left.
        ///
        /// \param Index  The zero-based index of the first element to remove.
        /// \param Length The number of elements to remove.
        ZY_INLINE void Remove(UInt Index, UInt Length)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            Length = Min(Length, mSize - Index);

            if constexpr (IsTriviallyCopyable<Type>)
            {
                const Ptr<Type> Data = mData + Index;
                Copy(Data, mSize - Index - Length, Data + Length);
            }
            else
            {
                for (UInt Offset = 0; Offset < Length; ++Offset)
                {
                    Destruct<Type>(mData[Index + Offset]);
                }

                for (UInt Element = Index + Length; Element < mSize; ++Element)
                {
                    Relocate<Type>(mData[Element - Length], Move(mData[Element]));
                }
            }

            mSize -= Length;
        }

        /// \brief Removes the first element for which \p Predicate returns `true`, shifting subsequent elements left.
        ///
        /// \param Predicate The unary predicate used to identify the element to remove.
        /// \return `true` if an element was found and removed, otherwise `false`.
        template<typename Callable>
        ZY_INLINE Bool RemoveIf(AnyRef<Callable> Predicate)
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mData[Index]))
                {
                    Remove(Index);
                    return true;
                }
            }
            return false;
        }

        /// \brief Removes the element at the given index by swapping it with the last element.
        ///
        /// \param Index The zero-based index of the element to remove. Does not preserve insertion order.
        ZY_INLINE void RemoveFast(UInt Index)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            if (Index + 1 < mSize)
            {
                mData[Index] = Move(mData[mSize - 1]);
            }

            Destruct<Type>(mData[mSize - 1]);
            --mSize;
        }

        /// \brief Removes the first element for which \p Predicate returns `true` by swapping it with the last element.
        ///
        /// \param Predicate The unary predicate used to identify the element to remove. Does not preserve insertion order.
        /// \return `true` if an element was found and removed, otherwise `false`.
        template<typename Callable>
        ZY_INLINE Bool RemoveFastIf(AnyRef<Callable> Predicate)
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mData[Index]))
                {
                    RemoveFast(Index);
                    return true;
                }
            }
            return false;
        }

        /// \brief Removes all elements for which \p Predicate returns `true` by swapping with the last element.
        ///
        /// \param Predicate The unary predicate used to identify elements to remove. Does not preserve insertion order.
        /// \return The number of elements removed.
        template<typename Callable>
        ZY_INLINE UInt RemoveFastSomeIf(AnyRef<Callable> Predicate)
        {
            UInt Result = 0;

            for (UInt Index = 0, Limit = mSize; Index < Limit; )
            {
                if (Predicate(mData[Index]))
                {
                    RemoveFast(Index);

                    --Limit;
                    ++Result;
                }
                else
                {
                    ++Index;
                }
            }
            return Result;
        }

        /// \brief Removes the last element from the sequence.
        ZY_INLINE void RemoveLast()
        {
            ZY_ASSERT(mSize > 0, "Cannot remove from an empty sequence");

            Destruct<Type>(mData[--mSize]);
        }

        /// \brief Finds the first occurrence of the specified element.
        ///
        /// \param Element The element to search for.
        /// \return The index of the first occurrence, or the size if not found.
        ZY_INLINE UInt Find(ConstRef<Type> Element) const
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (mData[Index] == Element)
                {
                    return Index;
                }
            }
            return mSize;
        }

        /// \brief Finds the first element matching the specified predicate.
        ///
        /// \param Predicate The unary predicate used to identify the element.
        /// \return The index of the first matching element, or the size if not found.
        template<typename Callable>
        ZY_INLINE UInt Find(AnyRef<Callable> Predicate) const
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mData[Index]))
                {
                    return Index;
                }
            }
            return mSize;
        }

        /// \brief Checks whether the sequence contains the specified element.
        ///
        /// \param Element The element to search for.
        /// \return `true` if the element is found, otherwise `false`.
        ZY_INLINE Bool Contains(ConstRef<Type> Element) const
        {
            return Find(Element) != mSize;
        }

        /// \brief Checks whether the sequence contains an element matching the specified predicate.
        ///
        /// \param Predicate The unary predicate used to identify the element.
        /// \return `true` if a matching element is found, otherwise `false`.
        template<typename Callable>
        ZY_INLINE Bool Contains(AnyRef<Callable> Predicate) const
        {
            return Find(Predicate) != mSize;
        }

        /// \brief Destroys all elements without releasing the underlying heap storage.
        ZY_INLINE void Clear()
        {
            if constexpr (!IsTriviallyCopyable<Type>)
            {
                for (UInt Index = 0; Index < mSize; ++Index)
                {
                    Destruct<Type>(mData[Index]);
                }
            }
            mSize = 0;
        }

        /// \brief Increases the size of the sequence by constructing \p Size new elements.
        ///
        /// \param Size The number of elements to construct.
        ZY_INLINE void Advance(UInt Size)
        {
            if (const UInt Required = mSize + Size; Required > mCapacity)
            {
                Grow(mCapacity > 0 ? Max(mCapacity * 2, Required) : Max(16u, Required));
            }
            mSize += Size;
        }

        /// \brief Fills \p Size elements with \p Element, advancing the size accordingly.
        ///
        /// For trivial element types this lowers to a single `memset`-equivalent call.
        ///
        /// \param Element The value to assign to each new slot.
        /// \param Size    The number of elements to fill.
        ZY_INLINE void Fill(ConstRef<Type> Element, UInt Size)
        {
            const UInt Offset = mSize;
            Advance(Size);

            ::Fill(mData + Offset, Size, Element);
        }

        /// \brief Ensures that the sequence can hold at least \p Capacity elements without reallocating.
        ///
        /// \param Capacity The minimum number of elements to reserve space for.
        ZY_INLINE void Reserve(UInt Capacity)
        {
            if (Capacity > mCapacity)
            {
                Grow(Capacity);
            }
        }

        /// \brief Reduces the capacity to match the current size, freeing unused memory.
        ZY_INLINE void Shrink()
        {
            if (mSize < mCapacity && mSize != 0)
            {
                Grow(mSize);
            }
        }

        /// \brief Replaces the contents with a copy of \p Other.
        ///
        /// \param Other The sequence to copy from.
        /// \return A reference to this sequence.
        ZY_INLINE Ref<Sequence> operator=(ConstRef<Sequence> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();

                Append(Other);
            }

            return (* this);
        }

        /// \brief Replaces the contents by taking ownership of \p Other's storage.
        ///
        /// \param Other The sequence to move from. Left empty after the operation.
        /// \return A reference to this sequence.
        ZY_INLINE Ref<Sequence> operator=(AnyRef<Sequence> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();

                if (mData)
                {
                    ::operator delete(mData);
                }

                mData     = Exchange(Other.mData,     nullptr);
                mSize     = Exchange(Other.mSize,     0u);
                mCapacity = Exchange(Other.mCapacity, 0u);
            }

            return (* this);
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A reference to the requested element.
        ZY_INLINE Ref<Type> operator[](UInt Index)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            return mData[Index];
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A read-only reference to the requested element.
        ZY_INLINE ConstRef<Type> operator[](UInt Index) const
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            return mData[Index];
        }

    private:

        /// \brief Reallocates the heap buffer to hold at least \p Capacity elements, moving all existing elements.
        ///
        /// \param Capacity The new minimum number of elements the buffer must accommodate.
        ZY_INLINE void Grow(UInt Capacity)
        {
            const Ptr<Type> Data = static_cast<Ptr<Type>>(::operator new(sizeof(Type) * Capacity));

            if constexpr (IsTriviallyCopyable<Type>)
            {
                Copy(Data, mSize, mData);
            }
            else
            {
                for (UInt Index = 0; Index < mSize; ++Index)
                {
                    Relocate(Data[Index], Move(mData[Index]));
                }
            }

            if (mData)
            {
                ::operator delete(mData);
            }

            mData     = Data;
            mCapacity = Capacity;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<Type> mData;
        UInt32    mSize;
        UInt32    mCapacity;
    };

    /// \brief A stack-backed contiguous sequence of \p Type elements with a fixed capacity of \p Count.
    template<typename Type, UInt Count>
        requires (Count > 0)
    class Sequence<Type, Count> final
    {
    public:

        /// \brief The type of elements stored in the container.
        using Element = Type;

    public:

        /// \brief Constructs an empty sequence.
        ZY_INLINE constexpr Sequence()
            : mSize { 0 }
        {
        }

        /// \brief Constructs a sequence as a copy of \p Other, duplicating all elements.
        ///
        /// \param Other The sequence to copy from.
        ZY_INLINE constexpr Sequence(ConstRef<Sequence> Other)
            : mSize { 0 }
        {
            Append(Other);
        }

        /// \brief Constructs a sequence by moving all elements from \p Other.
        ///
        /// \param Other The sequence to move from. Left empty after the operation.
        ZY_INLINE constexpr Sequence(AnyRef<Sequence> Other)
            : mSize { Exchange(Other.mSize, 0u) }
        {
            mStorage.Relocate(Other.mStorage, 0, 0, mSize);
        }

        /// \brief Constructs a sequence by copying all elements from \p Collection.
        ///
        /// \param Collection The collection of elements to copy into the sequence.
        template<typename Container>
        ZY_INLINE constexpr Sequence(ConstRef<Container> Collection)
            requires IsContiguousOf<Container, Type>
            : mSize { 0 }
        {
            Append(Collection);
        }

        /// \brief Destroys all elements in place.
        ZY_INLINE constexpr ~Sequence()
        {
            if !consteval
            {
                Clear();
            }
        }

        /// \brief Gets a pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element slot.
        ZY_INLINE constexpr Ptr<Type> GetData()
        {
            return mStorage.GetData();
        }

        /// \brief Gets a read-only pointer to the contiguous element storage.
        ///
        /// \return A pointer to the first element slot.
        ZY_INLINE constexpr ConstPtr<Type> GetData() const
        {
            return mStorage.GetData();
        }

        /// \brief Gets the number of elements currently stored in the sequence.
        ///
        /// \return The element count.
        ZY_INLINE constexpr UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the maximum number of elements the sequence can hold.
        ///
        /// \return The fixed capacity \p Count.
        ZY_INLINE constexpr UInt GetCapacity() const
        {
            return Count;
        }

        /// \brief Checks whether the sequence contains no elements.
        ///
        /// \return `true` if the sequence is empty, otherwise `false`.
        ZY_INLINE constexpr Bool IsEmpty() const
        {
            return mSize == 0;
        }

        /// \brief Checks whether the sequence has reached its fixed capacity.
        ///
        /// \return `true` if no more elements can be Appended, otherwise `false`.
        ZY_INLINE constexpr Bool IsFull() const
        {
            return mSize == Count;
        }

        /// \brief Gets the first element in the sequence.
        ///
        /// \return A reference to the first element.
        ZY_INLINE constexpr Ref<Type> GetFront()
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mStorage.GetValue(0);
        }

        /// \brief Gets the first element in the sequence.
        ///
        /// \return A read-only reference to the first element.
        ZY_INLINE constexpr ConstRef<Type> GetFront() const
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mStorage.GetValue(0);
        }

        /// \brief Gets the last element in the sequence.
        ///
        /// \return A reference to the last element.
        ZY_INLINE constexpr Ref<Type> GetBack()
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mStorage.GetValue(mSize - 1);
        }

        /// \brief Gets the last element in the sequence.
        ///
        /// \return A read-only reference to the last element.
        ZY_INLINE constexpr ConstRef<Type> GetBack() const
        {
            ZY_ASSERT(mSize > 0, "Sequence is empty");

            return mStorage.GetValue(mSize - 1);
        }

        /// \brief Constructs a new element at the end of the sequence.
        ///
        /// \param Parameters The arguments to forward to the element's constructor.
        /// \return A reference to the newly Appended element.
        template<typename... Arguments>
        ZY_INLINE constexpr Ref<Type> Append(AnyRef<Arguments>... Parameters)
        {
            ZY_ASSERT(mSize < Count, "Sequence capacity is full");

            return mStorage.Construct(mSize++, Forward<Arguments>(Parameters)...);
        }

        /// \brief Appends multiple elements from a span to the end of the sequence.
        ///
        /// \param Elements The span of elements to Append.
        template<typename Container>
        ZY_INLINE constexpr void Append(AnyRef<Container> Elements)
            requires IsContiguousOf<Container, Type>
        {
            const UInt Size = Elements.GetSize();

            ZY_ASSERT(mSize + Size <= Count, "Sequence capacity is full");

            mStorage.Copy(mSize, Elements.GetData(), Size);
            mSize += Size;
        }

        /// \brief Inserts a new element at the specified index, shifting subsequent elements right.
        ///
        /// \param Index The zero-based index where the element should be inserted.
        /// \param Element The element to insert.
        template<typename... Arguments>
        ZY_INLINE constexpr void Insert(UInt Index, AnyRef<Arguments>... Element)
        {
            ZY_ASSERT(Index <= mSize && Index + 1 <= Count, "Insertion index is out of bounds or capacity is full");

            if (Index < mSize)
            {
                mStorage.Relocate(mStorage, mSize - 1, mSize, 1);
                mStorage.Slide(Index, Index + 1, mSize - 1 - Index);
                mStorage.Destruct(Index);
            }

            mStorage.Construct(Index, Forward<Arguments>(Element)...);
            ++mSize;
        }

        /// \brief Inserts multiple elements at the specified index, shifting subsequent elements right.
        ///
        /// \param Index    The zero-based index where elements should be inserted.
        /// \param Elements The span of elements to insert.
        template<typename Container>
        ZY_INLINE constexpr void Insert(UInt Index, AnyRef<Container> Elements)
            requires IsContiguousOf<Container, Type>
        {
            const UInt Size = Elements.GetSize();

            ZY_ASSERT(Index <= mSize && Index + Size <= Count, "Insertion index is out of bounds or capacity is full");

            if (Size == 0)
            {
                return;
            }

            const UInt Length        = mSize - Index;
            const UInt Reallocations = Min(Size, Length);
            const UInt Slides        = Length - Reallocations;

            if (Reallocations > 0)
            {
                mStorage.Relocate(mStorage, mSize - Reallocations, mSize - Reallocations + Size, Reallocations);
            }

            if (Slides > 0)
            {
                mStorage.Slide(Index, Index + Size, Slides);
            }

            mStorage.Destruct(Index, Slides);

            for (UInt Offset = 0; Offset < Size; ++Offset)
            {
                mStorage.Construct(Index + Offset, Elements[Offset]);
            }

            mSize += Size;
        }

        /// \brief Removes the element at the given index, shifting subsequent elements left.
        ///
        /// \param Index The zero-based index of the element to remove.
        ZY_INLINE constexpr void Remove(UInt Index)
        {
            Remove(Index, 1);
        }

        /// \brief Removes a range of elements starting at the given index, shifting subsequent elements left.
        ///
        /// \param Index  The zero-based index of the first element to remove.
        /// \param Length The number of elements to remove.
        ZY_INLINE constexpr void Remove(UInt Index, UInt Length)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            Length = Min(Length, mSize - Index);

            mStorage.Swap(Index + Length, Index, mSize - Index - Length);
            mStorage.Destruct(mSize - Length, Length);

            mSize -= Length;
        }

        /// \brief Removes the first element for which \p Predicate returns `true`, shifting subsequent elements left.
        ///
        /// \param Predicate The unary predicate used to identify the element to remove.
        /// \return `true` if an element was found and removed, otherwise `false`.
        template<typename Callable>
        ZY_INLINE constexpr Bool RemoveIf(AnyRef<Callable> Predicate)
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mStorage[Index]))
                {
                    Remove(Index);
                    return true;
                }
            }
            return false;
        }

        /// \brief Removes the element at the given index by swapping it with the last element.
        ///
        /// \param Index The zero-based index of the element to remove. Does not preserve insertion order.
        ZY_INLINE constexpr void RemoveFast(UInt Index)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            if (Index + 1 < mSize)
            {
                mStorage.Swap(mSize - 1, Index);
            }
            mStorage.Destruct(--mSize);
        }

        /// \brief Removes the first element for which \p Predicate returns `true` by swapping it with the last element.
        ///
        /// \param Predicate The unary predicate used to identify the element to remove. Does not preserve insertion order.
        /// \return `true` if an element was found and removed, otherwise `false`.
        template<typename Callable>
        ZY_INLINE constexpr Bool RemoveFastIf(AnyRef<Callable> Predicate)
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mStorage[Index]))
                {
                    RemoveFast(Index);
                    return true;
                }
            }
            return false;
        }

        /// \brief Removes all elements for which \p Predicate returns `true` by swapping with the last element.
        ///
        /// \param Predicate The unary predicate used to identify elements to remove. Does not preserve insertion order.
        /// \return The number of elements removed.
        template<typename Callable>
        ZY_INLINE constexpr UInt RemoveFastSomeIf(AnyRef<Callable> Predicate)
        {
            UInt Result = 0;

            for (UInt Index = 0, Limit = mSize; Index < Limit; )
            {
                if (Predicate(mStorage[Index]))
                {
                    RemoveFast(Index);

                    --Limit;
                    ++Result;
                }
                else
                {
                    ++Index;
                }
            }
            return Result;
        }

        /// \brief Removes the last element from the sequence.
        ZY_INLINE constexpr void RemoveLast()
        {
            ZY_ASSERT(mSize > 0, "Cannot remove from an empty sequence");

            mStorage.Destruct(--mSize);
        }

        /// \brief Finds the first occurrence of the specified element.
        ///
        /// \param Element The element to search for.
        /// \return The index of the first occurrence, or the size if not found.
        ZY_INLINE constexpr UInt Find(ConstRef<Type> Element) const
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (mStorage[Index] == Element)
                {
                    return Index;
                }
            }
            return mSize;
        }

        /// \brief Finds the first element matching the specified predicate.
        ///
        /// \param Predicate The unary predicate used to identify the element.
        /// \return The index of the first matching element, or the size if not found.
        template<typename Callable>
        ZY_INLINE constexpr UInt Find(AnyRef<Callable> Predicate) const
        {
            for (UInt Index = 0; Index < mSize; ++Index)
            {
                if (Predicate(mStorage[Index]))
                {
                    return Index;
                }
            }
            return mSize;
        }

        /// \brief Checks whether the sequence contains the specified element.
        ///
        /// \param Element The element to search for.
        /// \return `true` if the element is found, otherwise `false`.
        ZY_INLINE constexpr Bool Contains(ConstRef<Type> Element) const
        {
            return Find(Element) != mSize;
        }

        /// \brief Checks whether the sequence contains an element matching the specified predicate.
        ///
        /// \param Predicate The unary predicate used to identify the element.
        /// \return `true` if a matching element is found, otherwise `false`.
        template<typename Callable>
        ZY_INLINE constexpr Bool Contains(AnyRef<Callable> Predicate) const
        {
            return Find(Predicate) != mSize;
        }

        /// \brief Destroys all elements without affecting the fixed-size storage.
        ZY_INLINE constexpr void Clear()
        {
            mStorage.Destruct(0, mSize);
            mSize = 0;
        }

        /// \brief Increases the size of the sequence by constructing \p Size new elements.
        ///
        /// \param Size The number of elements to construct.
        ZY_INLINE constexpr void Advance(UInt Size)
        {
            ZY_ASSERT(mSize + Size <= Count, "Sequence capacity is full");

            if consteval
            {
                for (UInt Index = 0; Index < Size; ++Index)
                {
                    mStorage.Construct(Index + mSize);
                }
            }
            mSize += Size;
        }

        /// \brief Fills \p Length elements with \p Element, advancing the size accordingly.
        ///
        /// For trivial element types this lowers to a single `memset`-equivalent call.
        ///
        /// \param Element The value to assign to each new slot.
        /// \param Size    The number of elements to fill.
        ZY_INLINE constexpr void Fill(ConstRef<Type> Element, UInt Size)
        {
            ZY_ASSERT(mSize + Size <= Count, "Sequence capacity is full");

            if consteval
            {
                for (UInt Index = 0; Index < Size; ++Index)
                {
                    mStorage.Construct(Index + mSize, Element);
                }
            }
            else
            {
                ::Fill(mStorage.GetAddress(mSize), Size, Element);
            }
            mSize += Size;
        }

        /// \brief Replaces the contents with a copy of \p Other.
        ///
        /// \param Other The sequence to copy from.
        /// \return A reference to this sequence.
        ZY_INLINE constexpr Ref<Sequence> operator=(ConstRef<Sequence> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();

                Append(Other);
            }
            return (* this);
        }

        /// \brief Replaces the contents by moving all elements from \p Other.
        ///
        /// \param Other The sequence to move from. Left empty after the operation.
        /// \return A reference to this sequence.
        ZY_INLINE constexpr Ref<Sequence> operator=(AnyRef<Sequence> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();

                mStorage.Relocate(Other.mStorage, 0, 0, Other.mSize);

                mSize = Exchange(Other.mSize, 0u);
            }
            return (* this);
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A reference to the requested element.
        ZY_INLINE constexpr Ref<Type> operator[](UInt Index)
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            return mStorage[Index];
        }

        /// \brief Gets the element at the specified index.
        ///
        /// \param Index The zero-based index of the element to access.
        /// \return A read-only reference to the requested element.
        ZY_INLINE constexpr ConstRef<Type> operator[](UInt Index) const
        {
            ZY_ASSERT(Index < mSize, "Sequence index is out of bounds");

            return mStorage[Index];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Integer<Count>       mSize;
        Storage<Type, Count> mStorage;
    };
}