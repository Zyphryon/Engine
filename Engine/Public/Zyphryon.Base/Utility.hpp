// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Trait.hpp"
#include <format>
#include <rapidhash.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Casts an object to an rvalue, enabling move semantics.
    /// 
    /// \param Object The object to move.
    /// \return An rvalue reference to the object.
    template<typename Type>
    constexpr UnRef<Type>&& Move(Type&& Object)
    {
        return static_cast<UnRef<Type>&&>(Object);
    }

    /// \brief Perfectly forwards an object to another function.
    /// 
    /// \param Object The object to forward.
    /// \return An lvalue or rvalue reference depending on the input type.
    template<typename Type>
    constexpr Type&& Forward(UnRef<Type>& Object)
    {
        return static_cast<Type&&>(Object);
    }

    /// \brief Perfectly forwards an object to another function.
    /// 
    /// \param Object The object to forward.
    /// \return An lvalue or rvalue reference depending on the input type.
    template<typename Type>
    constexpr Type&& Forward(UnRef<Type>&& Object)
    {
        return static_cast<Type&&>(Object);
    }

    /// \brief Exchanges the value of an object with a new one, returning the old value.
    /// 
    /// \param First  The object to update.
    /// \param Second The new value to assign.
    /// \return The previous value held by the object.
    template<typename Type, typename Other = Type>
    constexpr Type Exchange(Ref<Type> First, AnyRef<Other> Second)
    {
        Type Temp = Move(First);
        First     = Forward<Other>(Second);
        return Temp;
    }

    /// \brief Swaps the values of two objects using move semantics.
    /// 
    /// \param First  The first object.
    /// \param Second The second object.
    template<typename Type>
    void Swap(Ref<Type> First, Ref<Type> Second)
    {
        Second = Exchange(First, Move(Second));
    }

    /// \brief Combines multiple values into a single hash .
    ///
    /// \param Parameters References to the values to be combined into the hash.
    /// \return An integer value representing the combined hash of all input parameters.
    template<typename... Arguments>
    constexpr UInt64 HashCombine(AnyRef<Arguments>... Parameters)
    {
        UInt64 Seed = 0;

        (..., (Seed = rapid_mix(Seed ^ ([]<typename Type>(ConstRef<Type> Value)
        {
            if constexpr (requires(ConstRef<Type> Instance) { Instance.Hash(); })
            {
                return Value.Hash();
            }
            else
            {
                if constexpr (IsPointer<Type>)
                {
                    return rapidhash(Value, sizeof(std::remove_pointer_t<Type>));
                }
                else
                {
                    return rapidhash(&Value, sizeof(Type));
                }
            }
        }(Parameters)), 0x9DDFEA08EB382D69ULL)));

        return Seed;
    }

    /// \brief Computes a hash value for a block of text data.
    ///
    /// \param Block A string view representing the text data to be hashed.
    /// \return An integer value representing the hash of the input text block.
    ZYPHRYON_INLINE UInt64 HashText(ConstStr8 Block)
    {
        return rapidhash(Block.data(), Block.size());
    }

    /// \brief Computes a 64-bit FNV-1a hash of a string view at compile or run time.
    ///
    /// \param Block   The string data to hash.
    /// \param Counter Initial hash value (defaults to FNV offset basis).
    /// \return The 64-bit FNV-1a hash.
    constexpr UInt64 HashTextType(ConstStr8 Block, UInt64 Counter = 14695981039346656037ull)
    {
        for (const Char Character : Block)
        {
            Counter = (Counter ^ Character) * 1099511628211ull;
        }
        return Counter;
    }

    /// \brief Generates a compile-time hash identifier for a specific type.
    ///
    /// \tparam Type The type for which to generate a hash identifier.
    /// \return An integer value representing the unique hash of the type's name.
    template<typename Type>
    constexpr UInt64 HashType()
    {
#if defined(__clang__) || defined(__GNUC__) || defined(__GNUG__)
        return HashText(__PRETTY_FUNCTION__);
#else
        return HashTextType(__FUNCSIG__);
#endif
    }

    /// \brief Extracts an element of the specified type from a tuple-like container.
    /// 
    /// \tparam Type The type of the element to retrieve.
    /// \param Tuple The tuple-like container to fetch from.
    /// \return A reference (or value, if rvalue) to the extracted element.
    template<typename Type, typename Container>
    constexpr decltype(auto) Fetch(AnyRef<Container> Tuple)
    {
        return std::get<Type>(Forward<Container>(Tuple));
    }

    /// \brief Extracts an element of the specified type from a tuple-like container.
    ///
    /// \tparam Type The index of the element to retrieve.
    /// \param Tuple The tuple-like container to fetch from.
    /// \return A reference (or value, if rvalue) to the extracted element.
    template<UInt Type, typename Container>
    constexpr decltype(auto) Fetch(AnyRef<Container> Tuple)
    {
        return std::get<Type>(Forward<Container>(Tuple));
    }

    /// \brief Formats a UTF-8 string using the specified arguments.
    /// 
    /// \param Format     The format string, following std::format syntax.
    /// \param Parameters The parameters to substitute into the format string.
    /// \return A formatted UTF-8 string.
    template<typename... Arguments>
    ConstStr8 Format(ConstStr8 Format, AnyRef<Arguments>... Parameters)
    {
        thread_local Char Buffer[4096];

        const auto Result = std::vformat_to(Buffer, Format, std::make_format_args(Parameters...));
        (* Result) = '\0';

        return ConstStr8(Buffer, std::distance(Buffer, Result));
    }

    /// \brief Creates a lambda that captures the provided object and member function.
    ///
    /// \tparam Method The member function pointer to capture.
    /// \tparam Type The object type.
    /// \param Object The object instance to capture.
    /// \return A lambda bound to the member function and object.
    template<auto Method, typename Type>
    constexpr auto Capture(Ptr<Type> Object)
    {
        return [Object]<typename... T0>(AnyRef<T0>... Args)
        {
            return (Object->*Method)(Forward<T0>(Args)...);
        };
    }

    /// \brief Constructs an object of type `Type` in-place at the specified memory location.
    /// 
    /// performs a placement `new` using the provided memory address and constructor parameters.
    /// It is intended for use with manual memory management systems such as custom allocators or object pools.
    /// 
    /// \tparam Type The type of object to construct.
    /// \tparam Arguments The types of the constructor arguments.
    /// \param Memory     Pointer to the memory where the object will be constructed.
    /// \param Parameters The constructor arguments to forward.
    /// \return A reference to the newly constructed object.
    template<typename Type, typename... Arguments>
    constexpr decltype(auto) InPlaceCreate(Ptr<void> Memory, AnyRef<Arguments>... Parameters)
    {
        return * new (static_cast<Ptr<Type>>(Memory)) Type(Forward<Arguments>(Parameters)...);
    }

    /// \brief Constructs an object of type `Type` in-place at the specified memory location.
    ///
    /// performs a placement `new` using the provided memory address and constructor parameters.
    /// It is intended for use with manual memory management systems such as custom allocators or object pools.
    ///
    /// \tparam Type The type of object to construct.
    /// \tparam Arguments The types of the constructor arguments.
    /// \param Memory     Reference to the memory where the object will be constructed.
    /// \param Parameters The constructor arguments to forward.
    /// \return A reference to the newly constructed object.
    template<typename Type, typename... Arguments>
    constexpr decltype(auto) InPlaceConstruct(Ref<Type> Memory, AnyRef<Arguments>... Parameters)
    {
       return * ::new (static_cast<Ptr<Type>>(&Memory)) Type(Forward<Arguments>(Parameters)...);
    }

    /// \brief Destroys an object in-place if its type is non-trivially destructible.
    /// 
    /// explicitly calls the destructor of the given object, but only if the type
    /// is not trivially destructible. It is safe to use with objects allocated in custom memory
    /// pools or manually managed storage.
    /// 
    /// \tparam Type The type of the object.
    /// \param Object Reference to the object to destroy.
    template<typename Type>
    constexpr void InPlaceDelete(Ref<Type> Object) noexcept
    {
        if constexpr (!std::is_trivially_destructible_v<Type>)
        {
            Object.~Type();
        }
    }

    /// \brief Creates a \c Unique pointer to a newly constructed object.
    /// 
    /// \param Arguments The arguments to forward to the constructor.
    /// \return A \c Unique pointer managing the newly constructed object.
    template<typename Type, typename... Args>
    auto NewUniquePtr(AnyRef<Args> ... Arguments)
    {
        return std::make_unique<Type>(Forward<Args>(Arguments)...);
    }

    /// \brief Aligns an integer offset to the specified alignment.
    ///
    /// \tparam Alignment Alignment value in bytes.
    /// \param  Offset    Input value to align.
    /// \return Aligned value, greater than or equal to \p Offset.
    template<UInt Alignment>
    constexpr auto Align(UInt Offset)
    {
        return ((Offset + Alignment - 1) / Alignment) * Alignment;
    }

    /// \brief Aligns an integer offset to the specified alignment.
    ///
    /// \param  Offset    Input value to align.
    /// \param  Alignment Alignment value in bytes.
    /// \return Aligned value, greater than or equal to \p Offset.
    constexpr auto Align(UInt Offset, UInt Alignment)
    {
        return ((Offset + Alignment - 1) / Alignment) * Alignment;
    }

    /// \brief Converts 2D coordinates into a linear index.
    ///
    /// \param X     Horizontal coordinate.
    /// \param Y     Vertical coordinate.
    /// \param Width Number of elements per row.
    /// \return The 1D index corresponding to (X, Y).
    template<typename Type>
    constexpr auto Flatten2D(Type X, Type Y, Type Width)
    {
        return Y * Width + X;
    }

    /// \brief Converts 3D coordinates into a linear index.
    ///
    /// \param X      Horizontal coordinate.
    /// \param Y      Vertical coordinate.
    /// \param Z      Depth coordinate.
    /// \param Width  Number of elements per row.
    /// \param Height Number of rows per layer.
    /// \return The 1D index corresponding to (X, Y, Z).
    template<typename Type>
    constexpr auto Flatten3D(Type X, Type Y, Type Z, Type Width, Type Height)
    {
        return Z * (Width * Height) + Y * Width + X;
    }
}