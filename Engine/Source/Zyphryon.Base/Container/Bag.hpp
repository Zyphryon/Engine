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

inline namespace ZyBase
{
    /// \brief Represents a cache-efficient unordered bag with dense storage and fast iteration.
    template<typename Key>
    class Bag final
    {
    public:

        /// \brief Constructs an empty bag with no allocated storage.
        ZY_INLINE Bag() = default;

        /// \brief Gets a pointer to the dense array of keys.
        ///
        /// \return A pointer to the internal storage array containing all keys.
        ZY_INLINE Ptr<Key> GetData()
        {
            const Ptr<typename Table<Key, Empty>::Pair> Entries = mTable.GetData();
            return Entries ? AddressOf(Entries->First) : nullptr;
        }

        /// \brief Gets a read-only pointer to the dense array of keys.
        ///
        /// \return A const pointer to the internal storage array containing all keys.
        ZY_INLINE ConstPtr<Key> GetData() const
        {
            const ConstPtr<typename Table<Key, Empty>::Pair> Entries = mTable.GetData();
            return Entries ? AddressOf(Entries->First) : nullptr;
        }

        /// \brief Gets the number of keys currently stored in the bag.
        ///
        /// \return The count of elements in the bag.
        ZY_INLINE UInt GetSize() const
        {
            return mTable.GetSize();
        }

        /// \brief Gets the number of slots the bag has allocated.
        ///
        /// \return The slot count, which the bag fills to seven eighths before it grows.
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
            return mTable.Assign(Needle, Empty());
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

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Key, Empty> mTable;
    };
}