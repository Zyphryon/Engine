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
#include "Zyphryon.Base/Bit.hpp"
#include "Zyphryon.Base/Hash.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A cache-efficient hash map using Robin Hood hashing with backward-shift deletion and dense value storage.
    template<typename Key, typename Value>
    class Table final
    {
    public:

        /// \brief A key-value pair stored in the dense value array.
        struct Pair
        {
            Key                 First;
            ZY_COMPRESSED Value Second;
        };

    public:

        /// \brief Constructs an empty hash table with no allocated storage.
        ZY_INLINE Table()
            : mControl   { nullptr },
              mIndices   { nullptr },
              mEntries   { nullptr },
              mSize      { 0u },
              mMask      { 0u },
              mThreshold { 0u }
        {
        }

        /// \brief Constructs a hash table with initial data.
        ///
        /// \param List An initializer list of key-value pairs to populate the table.
        ZY_INLINE Table(ConstSpan<Pair> List)
            : Table()
        {
            Reserve(List.GetSize());

            for (ConstRef<Pair> Entry : List)
            {
                DoAssign(Entry.First, Entry.Second);
            }
        }

        /// \brief Constructs a hash table by copying all key-value pairs from another table.
        ///
        /// \param Other The table to copy from.
        ZY_INLINE Table(ConstRef<Table> Other)
            : Table { }
        {
            if (Other.mSize > 0u)
            {
                Reserve(Other.mSize);

                for (UInt32 Index = 0u; Index < Other.mSize; ++Index)
                {
                    Construct<Pair>(AddressOf(mEntries[Index]), Other.mEntries[Index].First, Other.mEntries[Index].Second);

                    const UInt64 Digest = Hash(mEntries[Index].First);
                    DoInsert(Compute(Digest), Index, Digest & mMask);
                }

                mSize = Other.mSize;
            }
        }

        /// \brief Constructs a hash table by transferring ownership from another table.
        ///
        /// \param Other The table to move from, which will be left in an empty state.
        ZY_INLINE Table(AnyRef<Table> Other)
            : mControl   { Exchange(Other.mControl,   nullptr) },
              mIndices   { Exchange(Other.mIndices,   nullptr) },
              mEntries   { Exchange(Other.mEntries,   nullptr) },
              mSize      { Exchange(Other.mSize,      0u) },
              mMask      { Exchange(Other.mMask,      0u) },
              mThreshold { Exchange(Other.mThreshold, 0u) }
        {
        }

        /// \brief Destroys the hash table, releasing all stored key-value pairs and allocated memory.
        ZY_INLINE ~Table()
        {
            Clear();
            Free();
        }

        /// \brief Gets a pointer to the dense array of key-value pairs.
        ///
        /// \return A pointer to the internal storage array containing all pairs.
        ZY_INLINE Ptr<Pair> GetData()
        {
            return mEntries;
        }

        /// \brief Gets a read-only pointer to the dense array of key-value pairs.
        ///
        /// \return A const pointer to the internal storage array containing all pairs.
        ZY_INLINE ConstPtr<Pair> GetData() const
        {
            return mEntries;
        }

        /// \brief Gets the number of key-value pairs currently stored in the table.
        ///
        /// \return The count of elements in the table.
        ZY_INLINE UInt GetSize() const
        {
            return mSize;
        }

        /// \brief Gets the maximum number of elements the table can hold before rehashing.
        ///
        /// \return The current capacity before exceeding the load factor threshold.
        ZY_INLINE UInt GetCapacity() const
        {
            return mControl ? (mMask + 1u) : 0u;
        }

        /// \brief Checks whether the table contains any key-value pairs.
        ///
        /// \return `true` if the table is empty, `false` otherwise.
        ZY_INLINE Bool IsEmpty() const
        {
            return mSize == 0u;
        }

        /// \brief Reserves storage to accommodate at least the specified number of elements.
        ///
        /// \param Count The minimum number of elements to allocate space for.
        ZY_INLINE void Reserve(UInt Count)
        {
            if (const UInt Requires = (Count * 8u + 6u) / 7u; Requires > (mMask + 1u))
            {
                Rehash(CeilBit(Requires));
            }
        }

        /// \brief Rebuilds the hash index for pairs written directly into the dense array.
        ///
        /// \param Count The number of pairs present in the dense array.
        ZY_INLINE void Reindex(UInt Count)
        {
            ZY_ASSERT(mSize == 0u, "Reindex requires an empty index");
            ZY_ASSERT(Count <= GetCapacity(), "Bulk load exceeds reserved capacity");

            mSize = static_cast<UInt32>(Count);

            for (UInt32 Index = 0u; Index < mSize; ++Index)
            {
                const UInt64 Digest = Hash(mEntries[Index].First);
                DoInsert(Compute(Digest), Index, Digest & mMask);
            }
        }

        /// \brief Removes all key-value pairs from the table without releasing allocated memory.
        ZY_INLINE void Clear()
        {
            if (mControl == nullptr || mSize == 0u)
            {
                return;
            }

            if constexpr (!IsTriviallyCopyable<Pair>)
            {
                for (UInt Index = 0u; Index < mSize; ++Index)
                {
                    Destruct(mEntries[Index]);
                }
            }

            Fill(mControl, (mMask + 1u), 0u);
            mSize = 0u;
        }

        /// \brief Inserts or updates a key-value pair in the table.
        ///
        /// \param Needle The key to insert or update.
        /// \param Data   The value to associate with the key.
        /// \return `true` if a new key was inserted, `false` if an existing key was updated.
        template<typename KeyType = Key, typename ValueType>
        ZY_INLINE Bool Assign(AnyRef<KeyType> Needle, AnyRef<ValueType> Data)
        {
            return DoAssign(Needle, Forward<ValueType>(Data));
        }

        /// \brief Removes a key-value pair from the table.
        ///
        /// \param Needle The key to remove from the table.
        /// \return `true` if the key was found and removed, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool Erase(AnyRef<KeyType> Needle)
        {
            return DoErase(Needle);
        }

        /// \brief Removes a key-value pair from the table if it exists and satisfies the given predicate.
        ///
        /// \param Needle    The key to search for in the table.
        /// \param Predicate A callable invoked with the matching value; returning `true` removes the entry.
        /// \return `true` if the key was found and the predicate caused its removal, `false` otherwise.
        template<typename KeyType = Key, typename Callable>
        ZY_INLINE Bool EraseIf(AnyRef<KeyType> Needle, AnyRef<Callable> Predicate)
        {
            return DoEraseIf(Needle, Predicate);
        }

        /// \brief Removes all entries satisfying the given predicate.
        ///
        /// \param Predicate A callable invoked with each key-value pair; returning `true` removes the entry.
        template<typename Callable>
        ZY_INLINE void EraseIf(AnyRef<Callable> Predicate)
        {
            for (SInt Index = static_cast<SInt>(mSize) - 1; Index >= 0; --Index)
            {
                if (Predicate(mEntries[Index].First, mEntries[Index].Second))
                {
                    UInt32 Match = static_cast<UInt32>(Hash(mEntries[Index].First) & mMask);

                    while (mIndices[Match] != static_cast<UInt32>(Index))
                    {
                        Match = (Match + 1u) & mMask;
                    }
                    DoRemove(static_cast<UInt32>(Index), Match);
                }
            }
        }

        /// \brief Extracts a key-value pair from the table, removing it if found.
        ///
        /// \param Needle The key to search for and remove from the table.
        /// \param Output The output reference to receive the associated value if the key is found.
        /// \return `true` if the key was found and removed, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool Extract(AnyRef<KeyType> Needle, Ref<Value> Output)
        {
            if (mSize == 0u)
            {
                return false;
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                Output = Move(mEntries[mIndices[Match]].Second);

                DoRemove(mIndices[Match], Match);
                return true;
            }
            return false;
        }

        /// \brief Searches for a value associated with the given key.
        ///
        /// \param Needle The key to search for in the table.
        /// \return A pointer to the value if found, \c nullptr otherwise.
        template<typename KeyType>
        ZY_INLINE Ptr<Value> Find(AnyRef<KeyType> Needle)
        {
            return DoFind(Needle);
        }

        /// \brief Searches for a value associated with the given key, returning the value directly.
        ///
        /// \param Needle The key to search for in the table.
        /// \return A copy of the value if found, or a default-constructed value otherwise.
        template<typename KeyType>
        ZY_INLINE Value FindOrDefault(AnyRef<KeyType> Needle)
        {
            const Ptr<Value> Found = DoFind(Needle);
            return Found ? (* Found) : Value { };
        }

        /// \brief Searches for a value associated with the given key, returning the value directly.
        ///
        /// \param Needle The key to search for in the table.
        /// \return A copy of the value if found, or a default-constructed value otherwise.
        template<typename KeyType>
        ZY_INLINE Value FindOrDefault(AnyRef<KeyType> Needle) const
        {
            const ConstPtr<Value> Found = DoFind(Needle);
            return Found ? (* Found) : Value { };
        }

        /// \brief Searches for a value associated with the given key.
        ///
        /// \param Needle The key to search for in the table.
        /// \return A const pointer to the value if found, \c nullptr otherwise.
        template<typename KeyType>
        ZY_INLINE ConstPtr<Value> Find(AnyRef<KeyType> Needle) const
        {
            return DoFind(Needle);
        }

        /// \brief Finds the value for a key, or inserts a default-constructed value if the key is absent.
        ///
        /// \param Needle The key to search for or insert.
        /// \return A reference to the value associated with the key.
        template<typename KeyType = Key>
        ZY_INLINE Ref<Value> FindOrInsert(AnyRef<KeyType> Needle)
        {
            if (mSize >= mThreshold)
            {
                Rehash((mMask + 1u) < 16u ? 16u : (mMask + 1u) * 2u);
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                return mEntries[mIndices[Match]].Second;
            }

            Construct<Pair>(AddressOf(mEntries[mSize]), Needle, Value{});
            DoInsert(Metadata, mSize, Slot);
            ++mSize;

            return mEntries[mSize - 1u].Second;
        }

        /// \brief Checks whether the table contains the specified key.
        ///
        /// \param Needle The key to search for in the table.
        /// \return `true` if the key exists in the table, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool Contains(AnyRef<KeyType> Needle) const
        {
            return DoFind(Needle) != nullptr;
        }

        /// \brief Replaces the contents by copying all key-value pairs from \p Other.
        ///
        /// \param Other The table to copy from.
        /// \return A reference to this table.
        ZY_INLINE Ref<Table> operator=(ConstRef<Table> Other)
        {
            if (this != AddressOf(Other))
            {
                Table Temp(Other);

                Swap(mControl,   Temp.mControl);
                Swap(mIndices,   Temp.mIndices);
                Swap(mEntries,   Temp.mEntries);
                Swap(mSize,      Temp.mSize);
                Swap(mMask,      Temp.mMask);
                Swap(mThreshold, Temp.mThreshold);
            }
            return (* this);
        }

        /// \brief Replaces the contents by taking ownership of \p Other's storage.
        ///
        /// \param Other The table to move from. Left empty after the operation.
        /// \return A reference to this table.
        ZY_INLINE Ref<Table> operator=(AnyRef<Table> Other)
        {
            if (this != AddressOf(Other))
            {
                Clear();
                Free();

                mControl   = Exchange(Other.mControl,   nullptr);
                mIndices   = Exchange(Other.mIndices,   nullptr);
                mEntries   = Exchange(Other.mEntries,   nullptr);
                mSize      = Exchange(Other.mSize,      0u);
                mMask      = Exchange(Other.mMask,      0u);
                mThreshold = Exchange(Other.mThreshold, 0u);
            }
            return (* this);
        }

    private:

        /// \brief Sentinel value returned by Probe when no matching entry is found.
        static constexpr UInt32 kInvalid   = ~0u;

        /// \brief Distance increment applied to the upper bits of the dist-and-fingerprint word per probe step.
        static constexpr UInt32 kIncrement = 1u << 8u;

        /// \brief Allocates storage for the hash table with the given capacity.
        ///
        /// \param Capacity The number of slots to allocate (must be a power of two).
        ZY_INLINE void Allocate(UInt Capacity)
        {
            const UInt Offset      = Align<UInt>(2u * Capacity * sizeof(UInt32), alignof(Pair));
            const Ptr<Byte> Memory = static_cast<Ptr<Byte>>(::operator new(Offset + Capacity * sizeof(Pair)));

            mControl   = reinterpret_cast<Ptr<UInt32>>(Memory);
            mIndices   = reinterpret_cast<Ptr<UInt32>>(Memory + Capacity * sizeof(UInt32));
            mEntries   = reinterpret_cast<Ptr<Pair>>(Memory + Offset);
            mMask      = Capacity - 1u;
            mThreshold = Capacity - Capacity / 8u;

            Fill(mControl, Capacity, 0u);
        }

        /// \brief Releases all allocated storage and resets the table to an empty state.
        ZY_INLINE void Free()
        {
            if (mControl)
            {
                ::operator delete(mControl);
            }

            mControl   = nullptr;
            mIndices   = nullptr;
            mEntries   = nullptr;
            mSize      = 0u;
            mMask      = 0u;
            mThreshold = 0u;
        }

        /// \brief Computes the initial dist-and-fingerprint word for a given hash.
        ///
        /// \param Hash The full 64-bit hash value.
        /// \return A 32-bit value with distance=1 in the upper bits and hash fingerprint in the lower 8 bits.
        ZY_INLINE UInt32 Compute(UInt64 Hash) const
        {
            return kIncrement | (static_cast<UInt32>(Hash >> 32u) & 0xFFu);
        }

        /// \brief Probes the slot array for an entry matching the given key.
        ///
        /// \param Needle      The key to search for.
        /// \param OutMetadata Receives the metadata value at the probe stopping position.
        /// \param OutSlot     Receives the slot index at the probe stopping position.
        /// \return The slot index of the matching entry, or \c kInvalid if the key is absent.
        template<typename KeyType>
        ZY_INLINE UInt32 Probe(AnyRef<KeyType> Needle, Ref<UInt32> OutMetadata, Ref<UInt32> OutSlot) const
        {
            const UInt64 Digest = Hash(Needle);
            OutMetadata = Compute(Digest);
            OutSlot     = Digest & mMask;

            while (OutMetadata <= mControl[OutSlot])
            {
                if (OutMetadata == mControl[OutSlot] && mEntries[mIndices[OutSlot]].First == Needle)
                {
                    return OutSlot;
                }

                OutMetadata += kIncrement;
                OutSlot      = (OutSlot + 1u) & mMask;
            }
            return kInvalid;
        }

        /// \brief Inserts a slot entry using Robin Hood hashing, displacing lower-priority entries as needed.
        ///
        /// \param Metadata The initial dist-and-fingerprint word for the entry.
        /// \param Entry    The index in the dense entry array.
        /// \param Slot     The starting slot index derived from the hash.
        ZY_INLINE void DoInsert(UInt32 Metadata, UInt32 Entry, UInt32 Slot)
        {
            while (mControl[Slot] != 0u)
            {
                if (Metadata > mControl[Slot])
                {
                    Swap(Metadata, mControl[Slot]);
                    Swap(Entry,    mIndices[Slot]);
                }
                Slot     = (Slot + 1u) & mMask;
                Metadata += kIncrement;
            }
            mControl[Slot] = Metadata;
            mIndices[Slot] = Entry;
        }

        /// \brief Inserts a new key-value pair or updates the value of an existing key.
        ///
        /// \param Needle The key to insert or update.
        /// \param Data   The value to associate with the key.
        /// \return `true` if a new key was inserted, `false` if an existing key was updated.
        template<typename KeyType = Key, typename ValueType>
        ZY_INLINE Bool DoAssign(AnyRef<KeyType> Needle, AnyRef<ValueType> Data)
        {
            if (mSize >= mThreshold)
            {
                Rehash((mMask + 1u) < 16u ? 16u : (mMask + 1u) * 2u);
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                mEntries[mIndices[Match]].Second = Forward<ValueType>(Data);
                return false;
            }

            Construct<Pair>(AddressOf(mEntries[mSize]), Needle, Forward<ValueType>(Data));
            DoInsert(Metadata, mSize, Slot);
            ++mSize;

            return true;
        }

        /// \brief Searches for a value associated with the given key.
        ///
        /// \param Needle The key to search for in the table.
        /// \return A pointer to the value if found, \c nullptr otherwise.
        template<typename KeyType>
        ZY_INLINE Ptr<Value> DoFind(AnyRef<KeyType> Needle) const
        {
            if (mSize == 0u)
            {
                return nullptr;
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                return AddressOf(mEntries[mIndices[Match]].Second);
            }
            return nullptr;
        }

        /// \brief Removes a key-value pair from the table.
        ///
        /// \param Needle The key to remove from the table.
        /// \return `true` if the key was found and removed, `false` otherwise.
        template<typename KeyType = Key>
        ZY_INLINE Bool DoErase(AnyRef<KeyType> Needle)
        {
            if (mSize == 0u)
            {
                return false;
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                DoRemove(mIndices[Match], Match);
                return true;
            }
            return false;
        }

        /// \brief Removes a key-value pair from the table if it exists and satisfies the given predicate.
        ///
        /// \param Needle    The key to search for in the table.
        /// \param Predicate A callable invoked with the matching value; returning `true` removes the entry.
        /// \return `true` if the key was found and the predicate caused its removal, `false` otherwise.
        template<typename KeyType = Key, typename Callable>
        ZY_INLINE Bool DoEraseIf(AnyRef<KeyType> Needle, AnyRef<Callable> Predicate)
        {
            if (mSize == 0u)
            {
                return false;
            }

            UInt32 Metadata;
            UInt32 Slot;

            if (const UInt32 Match = Probe(Needle, Metadata, Slot); Match != kInvalid)
            {
                if (Predicate(mEntries[mIndices[Match]].Second))
                {
                    DoRemove(mIndices[Match], Match);
                    return true;
                }
            }
            return false;
        }

        /// \brief Removes a key-value pair from the table by its dense array index and known slot.
        ///
        /// \param Entry The dense array index of the entry to remove.
        /// \param Match The slot index in the control array that points to the entry.
        ZY_INLINE void DoRemove(UInt32 Entry, UInt32 Match)
        {
            const UInt32 EntryLast = mSize - 1u;

            if (Entry != EntryLast)
            {
                mEntries[Entry] = Move(mEntries[EntryLast]);
                Destruct(mEntries[EntryLast]);

                UInt32 Slot = static_cast<UInt32>(Hash(mEntries[Entry].First) & mMask);

                while (mIndices[Slot] != EntryLast)
                {
                    Slot = (Slot + 1u) & mMask;
                }
                mIndices[Slot] = Entry;
            }
            else
            {
                Destruct(mEntries[Entry]);
            }

            UInt32 ShiftSlot = Match;
            UInt32 NextSlot  = (ShiftSlot + 1u) & mMask;

            while (mControl[NextSlot] >= kIncrement * 2u)
            {
                mControl[ShiftSlot] = mControl[NextSlot] - kIncrement;
                mIndices[ShiftSlot] = mIndices[NextSlot];

                ShiftSlot = NextSlot;
                NextSlot  = (ShiftSlot + 1u) & mMask;
            }
            mControl[ShiftSlot] = 0u;

            --mSize;
        }

        /// \brief Reallocates the table with a new capacity and rehashes all existing elements.
        ///
        /// \param Capacity The new capacity for the table (must be a power of two).
        ZY_INLINE void Rehash(UInt Capacity)
        {
            const Ptr<UInt32> OldControl = mControl;
            const UInt        OldSize    = mSize;
            const Ptr<Pair>   OldEntries = mEntries;

            Allocate(Capacity);

            if (OldControl)
            {
                for (UInt32 Index = 0u; Index < OldSize; ++Index)
                {
                    Relocate(mEntries[Index], Move(OldEntries[Index]));

                    const UInt64 Digest = Hash(mEntries[Index].First);
                    DoInsert(Compute(Digest), Index, Digest & mMask);
                }

                mSize = OldSize;
                ::operator delete(OldControl);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<UInt32> mControl;
        Ptr<UInt32> mIndices;
        Ptr<Pair>   mEntries;
        UInt32      mSize;
        UInt32      mMask;
        UInt32      mThreshold;
    };
}