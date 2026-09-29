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

            /// The slot of the parent, or zero for a root.
            UInt32     Parent;

            /// The slot of the first child, or zero.
            UInt32     First;

            /// The slot of the next sibling, or zero for the last one.
            UInt32     Next;

            /// The slot of the previous sibling, or of the last one for the first child.
            UInt32     Prev;
        };

    public:

        /// \brief Constructs a directory holding every slot below the first entity's, all free.
        Directory();

        /// \brief Gets how many slots exist, handed out or not.
        ///
        /// \return The number of slots.
        ZY_INLINE UInt32 GetSize() const
        {
            return static_cast<UInt32>(mSlots.GetSize());
        }

        /// \brief Checks whether a handle still names a living entity.
        ///
        /// \param Actor The handle.
        /// \return `true` if it does, `false` otherwise.
        ZY_INLINE Bool IsAlive(Handle Actor) const
        {
            const UInt32 Index = Actor.GetIndex();

            return Index < mSlots.GetSize()
                && mSlots[Index].Holder
                && mSlots[Index].Generation == Actor.GetGeneration();
        }

        /// \brief Gets the slot of the living entity a handle this directory handed out names.
        ///
        /// \param Actor The handle, whose slot is in range since slots are never taken away.
        /// \return The slot, or `nullptr` once the entity is gone, or the slot was never given out.
        ZY_INLINE ConstPtr<Slot> FindSlot(Handle Actor) const
        {
            ConstRef<Slot> Entry = mSlots[Actor.GetIndex()];
            return Entry.Holder && Entry.Generation == Actor.GetGeneration() ? AddressOf(Entry) : nullptr;
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
                if (const UInt32 Index = kMinRangeArchetypes + Slot; mSlots[Index].Holder)
                {
                    Callback(GetHandle(Index));
                }
            });
        }

        /// \brief Gets the handle naming the entity alive in a slot.
        ///
        /// \param Index The slot number.
        /// \return The handle.
        ZY_INLINE Handle GetHandle(UInt32 Index) const
        {
            return Handle(Index, mSlots[Index].Generation);
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
        /// \param Identifier The component.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when nothing lends it.
        ZY_INLINE ConstPtr<Byte> FindLent(UInt32 Base, UInt32 Identifier) const
        {
            for (; Base; Base = mSlots[Base].Holder->GetBase())
            {
                ConstRef<Chunk> Holder = * mSlots[Base].Holder;

                if (const SInt16 Column = Holder.Find(Identifier); Column != Chunk::kAbsent)
                {
                    if (!(Holder.GetFlags(Identifier) & Chunk::kLends))
                    {
                        return nullptr;
                    }
                    return Column >= 0 ? Holder.At(Column, mSlots[Base].Row) : AddressOf(kPresent);
                }
            }
            return nullptr;
        }

        /// \brief Gets what an entity in a chunk sees of a component, held or lent.
        ///
        /// \param Target     The chunk.
        /// \param Row        The row.
        /// \param Identifier The component.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when it sees none.
        ZY_INLINE ConstPtr<Byte> FindValue(ConstRef<Chunk> Target, UInt32 Row, UInt32 Identifier) const
        {
            if (const SInt16 Column = Target.Find(Identifier); Column != Chunk::kAbsent)
            {
                return Column >= 0 ? Target.At(Column, Row) : AddressOf(kPresent);
            }
            return FindLent(Target.GetBase(), Identifier);
        }

        /// \brief Gets what the nearest entity up a chain sees of a component, held or lent.
        ///
        /// \param Cursor     The slot to look at first, or zero for none, which receives the slot found, or zero.
        /// \param Identifier The component.
        /// \return The value, \ref kPresent for a tag, or `nullptr` when nothing up the chain sees it.
        ZY_INLINE ConstPtr<Byte> FindAncestor(Ref<UInt32> Cursor, UInt32 Identifier) const
        {
            for (; Cursor; Cursor = mSlots[Cursor].Parent)
            {
                ConstRef<Slot> Entry = mSlots[Cursor];

                if (const ConstPtr<Byte> Found = FindValue(* Entry.Holder, Entry.Row, Identifier))
                {
                    return Found;
                }
            }
            return nullptr;
        }

        /// \brief Gets a slot.
        ///
        /// \param Index The slot number.
        /// \return The slot.
        ZY_INLINE Ref<Slot> operator[](UInt32 Index)
        {
            return mSlots[Index];
        }

        /// \brief Gets a slot to read.
        ///
        /// \param Index The slot number.
        /// \return The slot.
        ZY_INLINE ConstRef<Slot> operator[](UInt32 Index) const
        {
            return mSlots[Index];
        }

    public:

        /// \brief Checks whether a slot is one archetypes are handed.
        ///
        /// \param Index The slot number.
        /// \return `true` if it is, `false` otherwise.
        ZY_INLINE static constexpr Bool IsArchetype(UInt32 Index)
        {
            return Index > kMinRangeArchetypes && Index <= kMaxRangeArchetypes;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Slot>                   mSlots;
        UInt32                           mFree;
        UInt32                           mLinks;
        Freelist<kMaxCountArchetypes, 0> mArchetypes;
    };
}