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

#include "Zyphryon.Base/Container/Table.hpp"
#include "Zyphryon.Base/Lexical/String.hpp"
#include "Zyphryon.Base/Memory/Unique.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A mixin that provides named factory registration for swappable implementations.
    ///
    /// \tparam Interface The abstract interface that all registered factories must produce.
    template<typename Interface>
    class Switchable
    {
    public:

        /// \brief A factory function that produces a new implementation instance.
        using Factory = Unique<Interface>(*)();

        /// \brief The type used for naming registered factories.
        using Name    = Str16;

    public:

        /// \brief Registers a named factory for the given implementation.
        ///
        /// \param Name    The unique name identifying the implementation.
        /// \param Factory The factory function that creates implementation instances.
        ZY_INLINE void Register(Text Name, Factory Factory)
        {
            mRegistry.Assign(Name, Factory);
        }

        /// \brief Unregisters the factory associated with the given name.
        ///
        /// \param Name The name of the implementation to unregister.
        ZY_INLINE void Unregister(Text Name)
        {
            mRegistry.Erase(Name);
        }

        /// \brief Checks whether a factory with the given name is registered.
        ///
        /// \param Name The name of the implementation to check.
        /// \return `true` if a factory is registered under the given name, `false` otherwise.
        ZY_INLINE Bool IsRegistered(Text Name) const
        {
            return mRegistry.Contains(Name);
        }

    protected:

        /// \brief Instantiates the implementation registered under the given name.
        ///
        /// \param Name The name of the factory to invoke.
        /// \return A unique pointer to the new instance, or empty if no factory was found.
        ZY_INLINE Unique<Interface> Switch(Text Name)
        {
            const Ptr<Factory> Entry = mRegistry.Find(Name);
            return Entry ? (* Entry)() : Unique<Interface>();
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Str16, Factory> mRegistry;
    };
}