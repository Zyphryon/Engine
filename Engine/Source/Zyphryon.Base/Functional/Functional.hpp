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

#include "Zyphryon.Base/Primitive.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Creates a lambda that captures an object pointer and invokes a member function on it.
    ///
    /// \tparam Method The member function pointer to invoke.
    /// \param  Object The pointer to the object on which to invoke the method.
    /// \return A callable that forwards arguments to the captured member function.
    template<auto Method, typename Type>
    constexpr auto Capture(Ptr<Type> Object)
    {
        return [Object]<typename... T0>(AnyRef<T0>... Args)
        {
            return (Object->*Method)(Forward<T0>(Args)...);
        };
    }

    /// \brief A callable that dispatches a member function call through a base pointer.
    ///
    /// \tparam Method The member function pointer to invoke.
    /// \tparam Type   The derived type to cast the source pointer to before invocation.
    template<auto Method, typename Type>
    constexpr auto Dispatch = []<typename T0, typename... T1>(Ptr<T0> Source, T1... Arguments)
    {
        return (static_cast<Ptr<Type>>(Source)->*Method)(Forward<T1>(Arguments)...);
    };
}