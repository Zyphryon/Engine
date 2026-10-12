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

#include "Chunk.hpp"
#include "Zyphryon.Scene/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents every slot of a world, with where its entity sits, its generation and its hierarchy links.
    class ZY_API Directory final
    {
    public:

        /// The byte a tag's value points at, since a tag keeps nothing of its own.
        static constexpr Byte kPresent = 1;

        /// \brief Represents one slot.
        struct Slot final
        {
            /// The chunk the entity sits in, or `nullptr` while the slot is free.
            Ptr<Chunk> Holder;

            /// The row it sits at, or the next free slot while the slot is free.
            UInt32     Row;

            /// The generation of the slot, raised each time the entity in it is destroyed.
            UInt32     Generation;
        };

    private:

        /// \brief Represents where one slot sits in the hierarchy, kept apart since most entities sit in none.
        struct Family final
        {
            /// The slot of the parent, or zero for a root.
            UInt32 Parent;

            /// The slot of the first child, or zero.
            UInt32 First;

            /// The slot of the next sibling, or zero for the last one.
            UInt32 Next;

            /// The slot of the previous sibling, or of the last one for the first child.
            UInt32 Prev;
        };

        /// \brief Represents one range of slot numbers, counted from its first, since entities start far past zero.
        struct Range final
        {
            /// Where each entity sits.
            Sequence<Slot>   Slots;

            /// Where each sits in the hierarchy, grown only as far as the highest slot ever linked.
            Sequence<Family> Families;
        };

    public:

        /// \brief Constructs a directory holding the world's slot, which grows its other slots as they are handed out.
        Directory();

        /// \brief Gets how many slot numbers exist, handed out or not, the fixed ones below the first entity's included.
        ///
        /// \return One past the highest slot number an entity holds.
        ZY_INLINE UInt32 GetSize() const
        {
            return kMinRangeEntities + static_cast<UInt32>(mEntities.Slots.GetSize());
        }

        /// \brief Checks whether a handle still names a living entity.
        ///
        /// \param Actor The handle, which may be stale or name nothing.
        /// \return `true` if its slot holds an entity of the same generation, `false` otherwise.
        ZY_INLINE Bool IsAlive(Handle Actor) const
        {
            const ConstPtr<Slot> Entry = Probe(Actor.GetIndex());
            return Entry && Entry->Holder && Entry->Generation == Actor.GetGeneration();
        }

        /// \brief Checks whether an entity sits in a slot, which may lie past every slot handed out.
        ///
        /// \param Index The slot number.
        /// \return `true` if the slot exists and holds an entity, `false` otherwise.
        ZY_INLINE Bool IsHeld(UInt32 Index) const
        {
            const ConstPtr<Slot> Entry = Probe(Index);
            return Entry && Entry->Holder;
        }

        /// \brief Gets the slot of the living entity a handle this directory handed out names.
        ///
        /// \param Actor The handle, whose slot is in range since slots are never taken away.
        /// \return The slot, or `nullptr` once the entity is gone, or the slot was never given out.
        ZY_INLINE ConstPtr<Slot> FindSlot(Handle Actor) const
        {
            ConstRef<Slot> Entry = (* this)[Actor.GetIndex()];
            return Entry.Holder && Entry.Generation == Actor.GetGeneration() ? AddressOf(Entry) : nullptr;
        }

        /// \brief Gets the parent of a slot.
        ///
        /// \param Index The slot number.
        /// \return The slot of the parent, or zero for a root.
        ZY_INLINE UInt32 GetParent(UInt32 Index) const
        {
            const ConstPtr<Family> Entry = FindFamily(Index);
            return Entry ? Entry->Parent : 0;
        }

        /// \brief Gets the first child of a slot.
        ///
        /// \param Index The slot number.
        /// \return The slot of the first child, or zero when it has none.
        ZY_INLINE UInt32 GetFirst(UInt32 Index) const
        {
            const ConstPtr<Family> Entry = FindFamily(Index);
            return Entry ? Entry->First : 0;
        }

        /// \brief Gets the next sibling of a slot.
        ///
        /// \param Index The slot number.
        /// \return The slot of the next sibling, or zero for the last one.
        ZY_INLINE UInt32 GetNext(UInt32 Index) const
        {
            const ConstPtr<Family> Entry = FindFamily(Index);
            return Entry ? Entry->Next : 0;
        }

        /// \brief Hands the handle of every living archetype to a callback, in slot order.
        ///
        /// \param Callback The callable, taking each handle, which may destroy archetypes still ahead of it.
        template<typename Callable>
        ZY_INLINE void ForEachArchetype(AnyRef<Callable> Callback) const
        {
            mArchetypes.ForEach([this, &Callback](UInt32 Slot)
            {
                // Looked at on each turn, since destroying one takes the archetypes beneath it along.
                if (const UInt32 Index = kMinRangeArchetypes + Slot; mFixed.Slots[Index].Holder)
                {
                    Callback(GetHandle(Index));
                }
            });
        }

        /// \brief Gets the handle naming the entity alive in a slot.
        ///
        /// \param Index The slot number.
        /// \return The handle, with the slot's current generation.
        ZY_INLINE Handle GetHandle(UInt32 Index) const
        {
            return Handle(Index, (* this)[Index].Generation);
        }

        /// \brief Takes a slot for a new entity, reusing a freed one first.
        ///
        /// \param Archetype `true` for one of the slots archetypes are handed.
        /// \return The handle, whose slot sits in no chunk yet.
        Handle Allocate(Bool Archetype);

        /// \brief Takes the archetype slot a save names, with the generation it was saved with.
        ///
        /// \param Actor The handle the save names, whose slot is free.
        /// \return The handle, whose slot sits in no chunk yet.
        Handle Acquire(Handle Actor);

        /// \brief Gives a slot back, raising its generation so handles to it stop naming anything.
        ///
        /// \param Index The slot number, which sits in no chunk and has no parent or child.
        void Release(UInt32 Index);

        /// \brief Links an entity as the last child of another.
        ///
        /// \param Index  The slot number of the entity, which has no parent.
        /// \param Parent The slot number of the parent.
        void Link(UInt32 Index, UInt32 Parent);

        /// \brief Unlinks an entity from its parent.
        ///
        /// \param Index The slot number of the entity, which must have a parent.
        void Unlink(UInt32 Index);

        /// \brief Gets how many times an entity was linked or unlinked, which changes whenever the hierarchy does.
        ///
        /// \return The count, which wraps around.
        ZY_INLINE UInt32 GetLinks() const
        {
            return mLinks;
        }

        /// \brief Gets what an archetype lends of a component, the nearest one up the chain holding it deciding.
        ///
        /// \param Base       The slot of the archetype, or zero.
        /// \param Identifier The identifier of the component to look for.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when nothing lends it.
        ZY_INLINE ConstPtr<Byte> FindLent(UInt32 Base, UInt32 Identifier) const
        {
            // Only archetypes lend, and they all sit in the fixed slots.
            for (; Base; Base = mFixed.Slots[Base].Holder->GetBase())
            {
                ConstRef<Chunk> Holder = * mFixed.Slots[Base].Holder;

                if (const SInt16 Column = Holder.Find(Identifier); Column != Chunk::kAbsent)
                {
                    if (!(Holder.GetFlags(Identifier) & Chunk::kLends))
                    {
                        return nullptr;
                    }
                    return Column >= 0 ? Holder.At(Column, mFixed.Slots[Base].Row) : AddressOf(kPresent);
                }
            }
            return nullptr;
        }

        /// \brief Gets what an entity in a chunk sees of a component, held or lent.
        ///
        /// \param Target     The chunk the entity sits in.
        /// \param Row        The row the entity sits at.
        /// \param Identifier The identifier of the component to look for.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when it sees none.
        ZY_INLINE ConstPtr<Byte> FindValue(ConstRef<Chunk> Target, UInt32 Row, UInt32 Identifier) const
        {
            if (const SInt16 Column = Target.Find(Identifier); Column != Chunk::kAbsent)
            {
                return Column >= 0 ? Target.At(Column, Row) : AddressOf(kPresent);
            }
            return FindLent(Target.GetBase(), Identifier);
        }

        /// \brief Checks whether an entity in a chunk sees a component, held or lent.
        ///
        /// \param Target     The chunk the entity sits in.
        /// \param Identifier The identifier of the component to look for.
        /// \return `true` if the chunk holds the component or its archetype lends it, `false` otherwise.
        ZY_INLINE Bool Sees(ConstRef<Chunk> Target, UInt32 Identifier) const
        {
            return Target.Has(Identifier) || FindLent(Target.GetBase(), Identifier);
        }

        /// \brief Gets what the nearest entity up a chain sees of a component, held or lent.
        ///
        /// \param Cursor     The slot to look at first, or zero for none, which receives the slot found, or zero.
        /// \param Identifier The identifier of the component to look for.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when nothing up the chain sees it.
        ZY_INLINE ConstPtr<Byte> FindAncestor(Ref<UInt32> Cursor, UInt32 Identifier) const
        {
            for (; Cursor; Cursor = GetParent(Cursor))
            {
                ConstRef<Slot> Entry = (* this)[Cursor];

                if (const ConstPtr<Byte> Found = FindValue(* Entry.Holder, Entry.Row, Identifier))
                {
                    return Found;
                }
            }
            return nullptr;
        }

        /// \brief Gets a slot.
        ///
        /// \param Index The slot number, below \ref GetSize.
        /// \return The slot, which moves when a slot is added.
        ZY_INLINE Ref<Slot> operator[](UInt32 Index)
        {
            return Pick(Index).Slots[Locate(Index)];
        }

        /// \brief Gets a slot to read.
        ///
        /// \param Index The slot number, below \ref GetSize.
        /// \return The slot, which moves when a slot is added.
        ZY_INLINE ConstRef<Slot> operator[](UInt32 Index) const
        {
            return Pick(Index).Slots[Locate(Index)];
        }

    public:

        /// \brief Checks whether a slot is one archetypes are handed.
        ///
        /// \param Index The slot number, in use or not.
        /// \return `true` if it lies in the archetype range, `false` otherwise.
        ZY_INLINE static constexpr Bool IsArchetype(UInt32 Index)
        {
            return Index > kMinRangeArchetypes && Index <= kMaxRangeArchetypes;
        }

    private:

        /// \brief Gets the range a slot number falls in.
        ///
        /// \param Index The slot number.
        /// \return The entities' range from the first entity's slot on, the fixed one below it.
        ZY_INLINE Ref<Range> Pick(UInt32 Index)
        {
            return Index >= kMinRangeEntities ? mEntities : mFixed;
        }

        /// \brief Gets the range a slot number falls in, to read.
        ///
        /// \param Index The slot number.
        /// \return The entities' range from the first entity's slot on, the fixed one below it.
        ZY_INLINE ConstRef<Range> Pick(UInt32 Index) const
        {
            return Index >= kMinRangeEntities ? mEntities : mFixed;
        }

        /// \brief Gets where a slot number sits within its range.
        ///
        /// \param Index The slot number.
        /// \return The position, counted from the first slot of the range.
        ZY_INLINE static constexpr UInt32 Locate(UInt32 Index)
        {
            return Index >= kMinRangeEntities ? Index - kMinRangeEntities : Index;
        }

        /// \brief Gets a slot that may not exist yet.
        ///
        /// \param Index The slot number, which may lie past every slot handed out.
        /// \return The slot, or `nullptr` when it was never grown.
        ZY_INLINE ConstPtr<Slot> Probe(UInt32 Index) const
        {
            ConstRef<Sequence<Slot>> Slots = Pick(Index).Slots;
            const UInt32 Local = Locate(Index);
            return Local < Slots.GetSize() ? AddressOf(Slots[Local]) : nullptr;
        }

        /// \brief Gets where a slot sits in the hierarchy, when it ever sat anywhere.
        ///
        /// \param Index The slot number.
        /// \return The family, or `nullptr` when the slot was never linked.
        ZY_INLINE ConstPtr<Family> FindFamily(UInt32 Index) const
        {
            ConstRef<Sequence<Family>> Families = Pick(Index).Families;
            const UInt32 Local = Locate(Index);
            return Local < Families.GetSize() ? AddressOf(Families[Local]) : nullptr;
        }

        /// \brief Gets where a slot sits in the hierarchy, growing the families to reach it, which moves them.
        ///
        /// \param Index The slot number.
        /// \return The family, zero for a slot that was never linked.
        Ref<Family> Tie(UInt32 Index);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Range                            mFixed;
        Range                            mEntities;
        UInt32                           mFree;
        UInt32                           mLinks;
        Freelist<kMaxCountArchetypes, 0> mArchetypes;
    };
}