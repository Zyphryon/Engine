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
        mSlots.Advance(kMinRangeEntities);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Directory::Allocate(Bool Archetype)
    {
        UInt32 Index;

        if (Archetype)
        {
            Index = static_cast<UInt32>(kMinRangeArchetypes + mArchetypes.Allocate().GetSlot());
        }
        else if (mFree)
        {
            Index = mFree;
            mFree = mSlots[Index].Row;
        }
        else
        {
            // Half again, not double, so a few entities past a power of two do not double every slot.
            if (mSlots.GetSize() == mSlots.GetCapacity())
            {
                mSlots.Reserve(mSlots.GetSize() + mSlots.GetSize() / 2);
            }
            Index = static_cast<UInt32>(mSlots.GetSize());
            mSlots.Append();
        }
        return Handle(Index, mSlots[Index].Generation);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Handle Directory::Acquire(Handle Actor)
    {
        const UInt32 Index = Actor.GetIndex();

        ZY_ASSERT(IsArchetype(Index) && !mSlots[Index].Holder, "Only a free archetype slot is taken back");

        mArchetypes.Acquire(static_cast<UInt16>(Index - kMinRangeArchetypes));
        mSlots[Index].Generation = Actor.GetGeneration();
        return Actor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Directory::Release(UInt32 Index)
    {
        Ref<Slot> Entry = mSlots[Index];
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
        Ref<Slot> Entry = mSlots[Index];
        Ref<Slot> Above = mSlots[Parent];
        ++mLinks;

        Entry.Parent = Parent;
        Entry.Next   = 0;

        // The first child's previous is the last one, so appending needs no walk.
        if (Above.First == 0)
        {
            Above.First = Index;
            Entry.Prev   = Index;
        }
        else
        {
            const UInt32 Last = mSlots[Above.First].Prev;
            mSlots[Last].Next        = Index;
            Entry.Prev               = Last;
            mSlots[Above.First].Prev = Index;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Directory::Unlink(UInt32 Index)
    {
        Ref<Slot> Entry = mSlots[Index];
        Ref<Slot> Above = mSlots[Entry.Parent];
        ++mLinks;

        if (Above.First == Index)
        {
            Above.First = Entry.Next;

            if (Entry.Next)
            {
                mSlots[Entry.Next].Prev = Entry.Prev;
            }
        }
        else
        {
            mSlots[Entry.Prev].Next = Entry.Next;
            mSlots[Entry.Next ? Entry.Next : Above.First].Prev = Entry.Prev;
        }

        Entry.Parent = 0;
        Entry.Next   = 0;
        Entry.Prev   = 0;
    }
}