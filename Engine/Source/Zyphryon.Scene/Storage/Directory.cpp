// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Directory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Directory::Directory()
        : mFree  { 0 },
          mLinks { 0 }
    {
        mFixed.Slots.Advance(2);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Directory::Allocate(Bool Archetype)
    {
        UInt32 Index;

        if (Archetype)
        {
            Index = static_cast<UInt32>(kMinRangeArchetypes + mArchetypes.Allocate().GetSlot());
            mFixed.Slots.Expand(Index + 1);
        }
        else if (mFree)
        {
            Index = mFree;
            mFree = (* this)[Index].Row;
        }
        else
        {
            // Half again, not double, so a few entities past a power of two do not double every slot.
            Ref<Sequence<Slot>> Slots = mEntities.Slots;

            if (Slots.GetSize() == Slots.GetCapacity())
            {
                Slots.Reserve(Max<UInt>(16, Slots.GetSize() + Slots.GetSize() / 2));
            }
            Index = kMinRangeEntities + static_cast<UInt32>(Slots.GetSize());
            Slots.Append();
        }
        return Handle(Index, (* this)[Index].Generation);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Directory::Acquire(Handle Actor)
    {
        const UInt32 Index = Actor.GetIndex();

        ZY_ASSERT(IsArchetype(Index), "Only an archetype slot is taken back");

        mFixed.Slots.Expand(Index + 1);

        ZY_ASSERT(!mFixed.Slots[Index].Holder, "Only a free archetype slot is taken back");

        mArchetypes.Acquire(static_cast<UInt16>(Index - kMinRangeArchetypes));
        mFixed.Slots[Index].Generation = Actor.GetGeneration();
        return Actor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Directory::Release(UInt32 Index)
    {
        Ref<Slot> Entry = (* this)[Index];
        Entry.Holder = nullptr;
        ++Entry.Generation;

        if (IsArchetype(Index))
        {
            mArchetypes.Release(static_cast<UInt16>(Index - kMinRangeArchetypes));
        }
        else
        {
            Entry.Row = mFree;
            mFree     = Index;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Directory::Link(UInt32 Index, UInt32 Parent)
    {
        ++mLinks;

        Tie(Index);
        Tie(Parent);

        Ref<Family> Entry = Tie(Index);
        Ref<Family> Above = Tie(Parent);

        Entry.Parent = Parent;
        Entry.Next   = 0;

        // The first child's previous is the last one, so appending needs no walk.
        if (Above.First == 0)
        {
            Above.First = Index;
            Entry.Prev  = Index;
        }
        else
        {
            Ref<Family> Head = Tie(Above.First);
            const UInt32 Last = Head.Prev;

            Tie(Last).Next = Index;
            Entry.Prev     = Last;
            Head.Prev      = Index;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Directory::Unlink(UInt32 Index)
    {
        ++mLinks;

        Ref<Family> Entry = Tie(Index);
        Ref<Family> Above = Tie(Entry.Parent);

        if (Above.First == Index)
        {
            Above.First = Entry.Next;

            if (Entry.Next)
            {
                Tie(Entry.Next).Prev = Entry.Prev;
            }
        }
        else
        {
            Tie(Entry.Prev).Next = Entry.Next;
            Tie(Entry.Next ? Entry.Next : Above.First).Prev = Entry.Prev;
        }

        Entry.Parent = 0;
        Entry.Next   = 0;
        Entry.Prev   = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Directory::Family> Directory::Tie(UInt32 Index)
    {
        Ref<Sequence<Family>> Families = Pick(Index).Families;
        const UInt32 Local = Locate(Index);

        Families.Expand(Local + 1);
        return Families[Local];
    }
}