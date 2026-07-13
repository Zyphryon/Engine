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

#include "Concept.hpp"
#include "Primitive.hpp"
#include <algorithm>
#include <new>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Casts \p Object to a rvalue reference, enabling move semantics.
    ///
    /// \param Object The object to move.
    /// \return An rvalue reference to \p Object.
    template<typename Type>
    constexpr StripRef<Type>&& Move(AnyRef<Type> Object)
    {
        return static_cast<StripRef<Type>&&>(Object);
    }

    /// \brief Forwards a lvalue, preserving its value category for perfect forwarding.
    ///
    /// \param Object The lvalue to forward.
    /// \return A forwarded reference to \p Object cast to \p Type.
    template<typename Type>
    constexpr AnyRef<Type> Forward(StripRef<Type>& Object)
    {
        return static_cast<AnyRef<Type>>(Object);
    }

    /// \brief Forwards a rvalue, preserving its value category for perfect forwarding.
    ///
    /// \param Object The rvalue to forward.
    /// \return A forwarded reference to \p Object cast to \p Type.
    template<typename Type>
    constexpr AnyRef<Type> Forward(StripRef<Type>&& Object)
    {
        return static_cast<AnyRef<Type>>(Object);
    }

    /// \brief Swaps the values of two objects.
    ///
    /// \param Left  The first object to swap.
    /// \param Right The second object to swap.
    template<typename Type>
    constexpr void Swap(Ref<Type> Left, Ref<Type> Right)
    {
        Type Temp = Move(Left);
        Left      = Move(Right);
        Right     = Move(Temp);
    }

    /// \brief Exchanges the value of the first object with the second, returning the original value of the first.
    ///
    /// \param Left  The object whose value will be replaced.
    /// \param Right The value to move into the first object.
    /// \return A copy of the original value of \p Left before the exchange.
    template<typename Type, typename Other = Type>
    constexpr Type Exchange(Ref<Type> Left, AnyRef<Other> Right)
    {
        Type Temp = Move(Left);
        Left      = static_cast<AnyRef<Other>>(Right);
        return Temp;
    }

    /// \brief Aligns an offset to the specified alignment boundary.
    ///
    /// \param Offset    The original offset to align.
    /// \param Alignment The alignment boundary to align to.
    /// \return A smallest multiple of \p Alignment that is greater than or equal to \p Offset.
    template<typename Type, typename Value>
    constexpr Type Align(Type Offset, Value Alignment)
    {
        return ((Offset + Alignment - 1) / Alignment) * Alignment;
    }

    /// \brief Aligns an offset to the specified power-of-two alignment boundary.
    ///
    /// \param Offset    The original offset to align.
    /// \param Alignment The power-of-two alignment boundary to align to.
    /// \return A smallest multiple of \p Alignment that is greater than or equal to \p Offset.
    template<typename Type, typename Value>
    constexpr Type AlignPowTwo(Type Offset, Value Alignment)
    {
        return (Offset + Alignment - 1) & ~(Alignment - 1);
    }

    /// \brief Gets the address of an object, even if it has an overloaded address-of operator.
    ///
    /// \param Object The object to get the address of.
    /// \return The address of \p Object.
    template<typename Type>
    constexpr Ptr<Type> AddressOf(Ref<Type> Object)
    {
        return std::addressof(Object);
    }

    /// \brief Constructs a \p Type object in-place at the given memory location.
    ///
    /// \param Memory     The raw memory address at which the object will be constructed.
    /// \param Parameters The constructor arguments to forward to \p Type.
    /// \return A reference to the newly constructed object.
    template<typename Type, typename... Arguments>
    constexpr Ref<Type> Construct(Ptr<Type> Memory, AnyRef<Arguments>... Parameters)
    {
        return * std::construct_at(Memory, Forward<Arguments>(Parameters)...);
    }

    /// \brief Destroys an object in place by invoking its destructor without releasing the underlying memory.
    ///
    /// \param Object The object to destroy.
    template<typename Type>
    constexpr void Destruct(Ref<Type> Object)
    {
        if constexpr (!IsTriviallyDestructible<Type>)
        {
            std::destroy_at(AddressOf(Object));
        }
    }

    /// \brief Relocates an object from one memory location to another by move-constructing and destructing.
    ///
    /// \param Destination The destination memory where the object will be move-constructed.
    /// \param Source      The source memory containing the object to relocate.
    template<typename Type>
    constexpr void Relocate(Ref<Type> Destination, AnyRef<Type> Source)
    {
        Construct<Type>(AddressOf(Destination), Move(Source));
        Destruct<Type>(Source);
    }

    /// \brief Obtains a pointer to the object at \p Memory.
    ///
    /// \param Memory The raw memory address holding a constructed object of \p Type.
    /// \return A pointer to the \p Type object at \p Memory with provenance guarantee.
    template<typename Type>
    constexpr Ptr<Type> Launder(Ptr<Type> Memory)
    {
        return std::launder(Memory);
    }

    /// \brief Fills a range of memory with the specified value.
    ///
    /// \param Destination The pointer to the start of the range to fill.
    /// \param Count       The number of elements to fill.
    /// \param Value       The value to assign to each element.
    template<typename Output, typename Input>
    constexpr void Fill(Ptr<Output> Destination, UInt Count, ConstRef<Input> Value)
    {
        std::fill_n(Destination, Count, static_cast<Output>(Value));
    }

    /// \brief Copies elements from source to destination.
    ///
    /// \param Destination The pointer to the destination range.
    /// \param Count       The number of elements to copy.
    /// \param Source      The pointer to the source range.
    template<typename Output, typename Input>
    constexpr void Copy(Ptr<Output> Destination, UInt Count, ConstPtr<Input> Source)
    {
        std::copy_n(Source, Count, Destination);
    }

    /// \brief Copies elements from source to destination.
    ///
    /// \param Destination The pointer to the destination range.
    /// \param Size        The number of bytes to copy.
    /// \param Source      The pointer to the source range.
    template<typename Output, typename Input>
    ZY_INLINE void Blit(Ptr<Output> Destination, UInt Size, ConstPtr<Input> Source)
    {
        std::memcpy(Destination, Source, Size);
    }

    /// \brief Zero-fills a contiguous range of elements, including any padding bytes.
    ///
    /// \param Destination The pointer to the destination range.
    /// \param Count       The number of elements to zero.
    template<typename Output>
    ZY_INLINE void Zero(Ptr<Output> Destination, UInt Count)
    {
        std::memset(Destination, 0, Count * sizeof(Output));
    }

    /// \brief Compares two ranges of elements for equality.
    ///
    /// \param Left  The pointer to the first range.
    /// \param Right The pointer to the second range.
    /// \param Count The number of elements to compare.
    /// \return `true` if all elements are equal, `false` otherwise.
    template<typename Input>
    constexpr Bool Compare(ConstPtr<Input> Left, ConstPtr<Input> Right, UInt Count)
    {
        return std::equal(Left, Left + Count, Right);
    }

    /// \brief Searches for the first occurrence of \p Value in the range [\p Data, \p Data + \p Size).
    ///
    /// \param Data  The pointer to the beginning of the range.
    /// \param Size  The number of elements to search.
    /// \param Value The value to search for.
    /// \return A pointer to the first occurrence of \p Value, or `nullptr` if not found.
    template<typename Input>
    constexpr ConstPtr<Input> Find(ConstPtr<Input> Data, UInt Size, ConstRef<Input> Value)
    {
        const ConstPtr<Input> End    = Data + Size;
        const ConstPtr<Input> Result = std::find(Data, End, Value);
        return Result == End ? nullptr : Result;
    }

    /// \brief Searches for the last occurrence of \p Value in the range [\p Data, \p Data + \p Size).
    ///
    /// \param Data  The pointer to the beginning of the range.
    /// \param Size  The number of elements to search.
    /// \param Value The value to search for.
    /// \return A pointer to the last occurrence of \p Value, or `nullptr` if not found.
    template<typename Input>
    constexpr ConstPtr<Input> FindLast(ConstPtr<Input> Data, UInt Size, ConstRef<Input> Value)
    {
        const auto Begin  = std::make_reverse_iterator(Data + Size);
        const auto End    = std::make_reverse_iterator(Data);
        const auto Result = std::find(Begin, End, Value);
        return Result == End ? nullptr : AddressOf(* Result);
    }

    /// \brief Counts the number of elements in [\p Data, \p Data + \p Size) equal to \p Value.
    ///
    /// \param Data  The pointer to the beginning of the range.
    /// \param Size  The number of elements to inspect.
    /// \param Value The value to count.
    /// \return A count of elements equal to \p Value.
    template<typename Input>
    constexpr UInt Count(ConstPtr<Input> Data, UInt Size, ConstRef<Input> Value)
    {
        return static_cast<UInt>(std::count(Data, Data + Size, Value));
    }
}
