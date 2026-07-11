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

#include "Table.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A cache-efficient unordered bag with dense storage and fast iteration.
    template<typename Key>
    class Bag final
    {
    public:

        /// \brief Constructs an empty bag with no allocated storage.
        ZY_INLINE Bag() = default;

        /// \brief Constructs a bag by transferring ownership from another bag.
        ///
        /// \param Other The bag to move from, which will be left in an empty state.
        ZY_INLINE Bag(AnyRef<Bag> Other)
            : mTable { Move (Other.mTable) }
        {
        }

        /// \brief Gets a pointer to the dense array of keys.
        ///
        /// \return A pointer to the internal storage array containing all keys.
        ZY_INLINE Ptr<Key> GetData()
        {
            const Ptr<typename Table<Key, Unit>::Pair> Entries = mTable.GetData();
            return AddressOf(Entries[0].First);
        }

        /// \brief Gets a read-only pointer to the dense array of keys.
        ///
        /// \return A const pointer to the internal storage array containing all keys.
        ZY_INLINE ConstPtr<Key> GetData() const
        {
            const ConstPtr<typename Table<Key, Unit>::Pair> Entries = mTable.GetData();
            return AddressOf(Entries[0].First);
        }

        /// \brief Gets the number of keys currently stored in the bag.
        ///
        /// \return The count of elements in the bag.
        ZY_INLINE UInt GetSize() const
        {
            return mTable.GetSize();
        }

        /// \brief Gets the maximum number of elements the bag can hold before rehashing.
        ///
        /// \return The current capacity of the bag's internal storage.
        ZY_INLINE UInt GetCapacity() const
        {
            return mTable.GetCapacity();
        }

        /// \brief Checks whether the bag contains any keys.
        ///
        /// \return `true` if the bag is empty, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mTable.IsEmpty();
        }

        /// \brief Reserves storage to accommodate at least the specified number of elements.
        ///
        /// \param Count The minimum number of elements to allocate space for.
        ZY_INLINE void Reserve(UInt Count)
        {
            mTable.Reserve(Count);
        }

        /// \brief Removes all keys from the bag without releasing allocated memory.
        ZY_INLINE void Clear()
        {
            mTable.Clear();
        }

        /// \brief Inserts a key into the bag.
        ///
        /// \param Needle The key to insert.
        /// \return `true` if the key was inserted, `false` if it already existed.
        template<typename KeyType = Key>
        ZY_INLINE Bool Insert(AnyRef<KeyType> Needle)
        {
            return mTable.Assign(Needle, Unit {});
        }

        /// \brief Removes a key from the bag.
        ///
        /// \param Needle The key to remove from the bag.
        /// \return `true` if the key was found and removed, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool Erase(AnyRef<KeyType> Needle)
        {
            return mTable.Erase(Needle);
        }

        /// \brief Extracts a key from the bag, removing it and returning it through the output parameter.
        ///
        /// \param Needle The key to extract from the bag.
        /// \param Output The output parameter to receive the extracted key if found.
        /// \return `true` if the key was found and extracted, `false` otherwise
        template<typename KeyType = Key>
        ZY_INLINE Bool Extract(AnyRef<KeyType> Needle, Ref<Key> Output)
        {
            return mTable.Extract(Needle, Output);
        }

        /// \brief Checks whether the bag contains the specified key.
        ///
        /// \param Needle The key to search for in the bag.
        /// \return `true` if the key exists in the bag, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool Contains(AnyRef<KeyType> Needle) const
        {
            return mTable.Contains(Needle);
        }

    private:

        /// \brief Empty type used as dummy value in the underlying table.
        struct Unit
        {

        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Key, Unit> mTable;
    };
}