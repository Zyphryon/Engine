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

#include "Zyphryon.Math/Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    /// \brief Represents the named values one agent's leaves share between ticks.
    class ZY_API Blackboard final
    {
    public:

        /// \brief The short name an entry can hold, sized to the widest value so it costs no extra storage.
        using Name  = String<13>;

        /// \brief The value of an entry.
        using Value = Variant<Bool, Real64, UInt64, Vector2, Vector3, Name>;

    public:

        /// \brief Sets an entry, replacing any value it held.
        ///
        /// \tparam Type The type of the value, which must be one of the types \ref Value holds.
        /// \param  Key  The name of the entry.
        /// \param  Data The value to hold.
        template<typename Type>
        ZY_INLINE void Set(Text Key, Type Data)
        {
            mEntries.Assign(Key, Value(Data));
        }

        /// \brief Finds the value of an entry.
        ///
        /// \tparam Type The type the value is expected to hold.
        /// \param  Key  The name of the entry.
        /// \return The value, or `nullptr` when the entry is missing or holds another type.
        template<typename Type>
        ZY_INLINE ConstPtr<Type> Find(Text Key) const
        {
            const ConstPtr<Value> Entry = mEntries.Find(Key);
            return Entry ? Entry->TryGet<Type>() : nullptr;
        }

        /// \brief Gets the value of an entry.
        ///
        /// \tparam Type    The type the value is expected to hold.
        /// \param  Key     The name of the entry.
        /// \param  Default The value returned when the entry is missing or holds another type.
        /// \return The value, or `Default`.
        template<typename Type>
        ZY_INLINE Type Get(Text Key, Type Default = Type()) const
        {
            const ConstPtr<Type> Found = Find<Type>(Key);
            return Found ? (* Found) : Default;
        }

        /// \brief Finds an entry of any type.
        ///
        /// \param Key The name of the entry.
        /// \return The entry, or `nullptr` when it is missing.
        ZY_INLINE ConstPtr<Value> Lookup(Text Key) const
        {
            return mEntries.Find(Key);
        }

        /// \brief Checks whether an entry exists.
        ///
        /// \param Key The name of the entry.
        /// \return `true` when the entry exists, otherwise `false`.
        ZY_INLINE Bool Has(Text Key) const
        {
            return mEntries.Find(Key) != nullptr;
        }

        /// \brief Removes an entry.
        ///
        /// \param Key The name of the entry.
        /// \return `true` when the entry existed, otherwise `false`.
        ZY_INLINE Bool Erase(Text Key)
        {
            return mEntries.Erase(Key);
        }

        /// \brief Removes every entry.
        ZY_INLINE void Clear()
        {
            mEntries.Clear();
        }

        /// \brief Visits every entry, in storage order.
        ///
        /// \param Callback The callback invoked with the name and the value of each entry.
        template<typename Function>
        ZY_INLINE void ForEach(AnyRef<Function> Callback) const
        {
            mEntries.ForEach(Forward<Function>(Callback));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Str32, Value> mEntries;
    };
}