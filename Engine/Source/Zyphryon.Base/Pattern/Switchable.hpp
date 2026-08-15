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

        /// \brief Specifies how strongly an implementation is preferred when the caller names none.
        enum class Priority : UInt8
        {
            Fallback,   ///< Stands in only while nothing else is registered.
            Low,        ///< Works everywhere, so it yields to anything written for the host.
            Normal,     ///< The rank an implementation takes when it states none.
            High,       ///< Written for the host, so it is taken over a portable one.
            Exclusive,  ///< Taken over every other implementation registered.
        };

    public:

        /// \brief Registers a named factory for the given implementation.
        ///
        /// \note The priority only decides the fallback, so naming an implementation always overrules it.
        ///
        /// \param Name     The unique name identifying the implementation.
        /// \param Factory  The factory function that creates implementation instances.
        /// \param Priority The rank the implementation is preferred by when no name is requested.
        ZY_INLINE void Register(Text Name, Factory Factory, Priority Priority = Priority::Normal)
        {
            mRegistry.Assign(Name, Entry(Factory, Priority));
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

        /// \brief Gets the name of the implementation that stands in when none is requested.
        ///
        /// \note Equal priorities are settled by name, so one build always resolves to the same implementation.
        ///
        /// \return The name of the highest-priority implementation, or an empty name if none is registered.
        ZY_INLINE Name GetDefault() const
        {
            Name     Result;
            Priority Rank = Priority::Fallback;

            for (UInt Index = 0, Limit = mRegistry.GetSize(); Index < Limit; ++Index)
            {
                ConstRef<Record> Candidate = mRegistry.GetData()[Index];

                if (Result.IsEmpty()
                    || Candidate.Second.Priority > Rank
                    || (Candidate.Second.Priority == Rank && Candidate.First < Result))
                {
                    Result = Candidate.First;
                    Rank   = Candidate.Second.Priority;
                }
            }
            return Result;
        }

    protected:

        /// \brief Instantiates the implementation registered under the given name.
        ///
        /// \param Name The name of the factory to invoke, or empty to take \ref GetDefault().
        /// \return A unique pointer to the new instance, or empty if no factory was found.
        ZY_INLINE Unique<Interface> Switch(Text Name)
        {
            if (Name.IsEmpty())
            {
                // Nothing was asked for, so whichever implementation ranks highest stands in for it.
                const Str16 Fallback = GetDefault();
                return Fallback.IsEmpty() ? Unique<Interface>() : Switch(Fallback);
            }

            const Ptr<Entry> Candidate = mRegistry.Find(Name);
            return Candidate ? Candidate->Build() : Unique<Interface>();
        }

    private:

        /// \brief Represents one registered implementation and the rank it is preferred by.
        struct Entry final
        {
            /// The factory function that creates implementation instances.
            Factory  Build    = nullptr;

            /// The rank the implementation is preferred by when no name is requested.
            Priority Priority = Priority::Normal;
        };

        /// \brief Defines the table entry each registration is stored as.
        using Record = typename Table<Str16, Entry>::Pair;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Str16, Entry> mRegistry;
    };
}