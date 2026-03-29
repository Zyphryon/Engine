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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Moves an object, allowing its resources to be transferred rather than copied.
    ///
    /// \param Object The object to move.
    /// \return An rvalue reference to the moved object.
    template<typename Type>
    static constexpr StripRef<Type>&& Move(AnyRef<Type> Object)
    {
        return static_cast<StripRef<Type>&&>(Object);
    }

    /// \brief Forwards an object, preserving its value category (lvalue or rvalue).
    ///
    /// \param Object The object to forward.
    /// \return The forwarded object with its original value category.
    template<typename Type>
    static constexpr AnyRef<Type> Forward(StripRef<Type>& Object)
    {
        return static_cast<AnyRef<Type>>(Object);
    }

    /// \brief Forwards an rvalue object, preserving its value category.
    ///
    /// \param Object The rvalue object to forward.
    /// \return The forwarded rvalue object.
    template<typename Type>
    static constexpr AnyRef<Type> Forward(StripRef<Type>&& Object)
    {
        return static_cast<AnyRef<Type>>(Object);
    }

    /// \brief Exchanges the value of the first object with the second, returning the original value of the first.
    ///
    /// \param Left  A reference to the first object whose value will be replaced.
    /// \param Right A reference to the second object whose value will be moved into the first.
    /// \return The original value of the first object before the exchange.
    template<typename Type, typename Other = Type>
    static constexpr Type Exchange(Ref<Type> Left, AnyRef<Other> Right)
    {
        Type Temp = static_cast<AnyRef<Type>>(Left);
        Left      = static_cast<AnyRef<Other>>(Right);
        return Temp;
    }

    /// \brief Swaps the values of two objects.
    ///
    /// \param Left  A reference to the first object to swap.
    /// \param Right A reference to the second object to swap.
    template<typename Type>
    static constexpr void Swap(Ref<Type> Left, Ref<Type> Right)
    {
        Type Temp = Move(Left);
        Left      = Move(Right);
        Right     = Move(Temp);
    }

    /// \brief Aligns an offset to the specified alignment boundary.
    ///
    /// \param Offset    The original offset to align.
    /// \param Alignment The alignment boundary to align to.
    /// \return The aligned offset.
    static constexpr UInt32 Align(UInt32 Offset, UInt32 Alignment)
    {
        return ((Offset + Alignment - 1) / Alignment) * Alignment;
    }

    /// \brief Captures a member function and its associated object into a callable lambda.
    ///
    /// \tparam Method The member function pointer to capture.
    /// \tparam Type   The type of the object instance.
    /// \param Object  A pointer to the object instance.
    /// \return A lambda that invokes the member function on the captured object.
    template<auto Method, typename Type>
    static constexpr auto Capture(Ptr<Type> Object)
    {
        return [Object]<typename... T0>(AnyRef<T0>... Args)
        {
            return (Object->*Method)(Forward<T0>(Args)...);
        };
    }

    /// \brief Creates a thunk lambda that invokes a member function on a given object pointer.
    ///
    /// \tparam Method The member function pointer to invoke.
    /// \tparam Type   The type of the object instance.
    /// @return A lambda that takes an object pointer and arguments, invoking the member function.
    template<auto Method, typename Type>
    static constexpr auto Thunk = []<typename T0, typename... T1>(Ptr<T0> Source, T1... Arguments)
    {
        return (static_cast<Ptr<Type>>(Source)->*Method)(Forward<T1>(Arguments)...);
    };

    /// \brief Constructs an object of type `Type` in the provided memory using placement new.
    ///
    /// \param Memory     A pointer to the memory where the object will be constructed.
    /// \param Parameters The constructor arguments to forward to the object's constructor.
    /// \return A reference to the newly constructed object.
    template<typename Type, typename... Arguments>
    static constexpr Ref<Type> InPlaceConstruct(Ptr<void> Memory, AnyRef<Arguments>... Parameters)
    {
        return * std::construct_at(static_cast<Ptr<Type>>(Memory), Forward<Arguments>(Parameters)...);
    }

    /// \brief Destroys an object of type `Type` in place, calling its destructor without deallocating memory.
    ///
    /// \param Object A reference to the object to be destroyed.
    template<typename Type>
    static constexpr void InPlaceDelete(Ref<Type> Object)
    {
        std::destroy_at(& Object);
    }
}