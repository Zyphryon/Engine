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

#include "Walk.hpp"
#include "Zyphryon.Platform/Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real64 Walk::GetTime()
    {
        static const ZyPlatform::Timer kClock;
        return kClock.GetSeconds();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Walk::FindMatch(ConstRef<Selection> State, UInt32 Row)
    {
        UInt32 Low  = 0;
        UInt32 High = static_cast<UInt32>(State.Offsets.GetSize()) - 1;

        while (Low + 1 < High)
        {
            if (const UInt32 Middle = (Low + High) / 2; State.Offsets[Middle] <= Row)
            {
                Low = Middle;
            }
            else
            {
                High = Middle;
            }
        }
        return Low;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Walk::GetShares(ConstRef<Selection> State, UInt32 Rows, UInt32 Threads)
    {
        // Shares of a few microseconds each, but never so many that taking one costs more than walking it.
        const Real64 Portions = Max(State.Cost, State.Pace) * Rows / kShareSeconds;
        const Real64 Limit    = Threads * kSharesPerThread;
        return static_cast<UInt32>(Clamp(Portions, 1.0, Limit));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Walk::Sort(Ref<Storage> Owner, Ref<Selection> State)
    {
        Ref<Sequence<UInt64>> Rows   = State.Gathered;
        Ref<Sequence<UInt32>> Starts = State.Offsets;

        // The last order holds while every row holds the same entity and nothing was linked or unlinked anywhere.
        if (IsSorted(Owner, State))
        {
            return static_cast<UInt32>(Rows.GetSize() / 2);
        }

        Rows.Clear();
        State.Depths.Clear();
        State.Snapshot.Clear();
        State.Sizes.Clear();
        Starts.Clear();

        // Siblings sit side by side, so a run of them climbs once for its depth.
        UInt32 Above = ~0u;
        UInt32 Depth = 0;

        for (UInt32 Match = 0; Match < State.Chunks.GetSize(); ++Match)
        {
            ConstRef<Chunk> Target = * State.Chunks[Match];

            State.Sizes.Append(Target.GetSize());

            for (UInt32 Row = 0; Row < Target.GetSize(); ++Row)
            {
                const Handle Actor  = Target.GetHandle(Row);
                const UInt32 Parent = Owner.mDirectory[Actor.GetIndex()].Parent;

                if (Parent != Above)
                {
                    Above = Parent;
                    Depth = 0;

                    for (UInt32 Next = Parent; Next; Next = Owner.mDirectory[Next].Parent)
                    {
                        ++Depth;
                    }

                    if (Depth >= Starts.GetSize())
                    {
                        Starts.Advance(Depth + 1 - Starts.GetSize());
                    }
                }
                ++Starts[Depth];
                State.Depths.Append(Depth);
                State.Snapshot.Append(Actor);
                Rows.Append(Pack(Match, Row));
            }
        }

        // A counting sort, which keeps storage order within a depth, laid out behind the rows it sorts.
        for (UInt32 Level = 0, Total = 0; Level < Starts.GetSize(); ++Level)
        {
            Total += Exchange(Starts[Level], Total);
        }

        const UInt32 Count = static_cast<UInt32>(Rows.GetSize());
        Rows.Advance(Count);

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            Rows[Count + Starts[State.Depths[Index]]++] = Rows[Index];
        }
        State.Links = Owner.mDirectory.GetLinks();
        return Count;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Walk::IsSorted(Ref<Storage> Owner, ConstRef<Selection> State)
    {
        if (State.Links != Owner.mDirectory.GetLinks())
        {
            return false;
        }
        if (State.Snapshot.GetSize() != State.Count() || State.Sizes.GetSize() != State.Chunks.GetSize())
        {
            return false;
        }
        
        // Every row of every match against the entity it held then, a page at a time.
        ConstPtr<Handle> Expected = State.Snapshot.GetData();

        for (UInt32 Match = 0; Match < State.Chunks.GetSize(); ++Match)
        {
            const Ptr<Chunk> Target = State.Chunks[Match];

            // An entity moved from the head of one match onto the tail of the one before it leaves the snapshot as it
            // was, but not where its gathered row points, so each match must still hold as many rows as it did.
            if (Target->GetSize() != State.Sizes[Match])
            {
                return false;
            }

            for (UInt32 Row = 0; Row < Target->GetSize();)
            {
                const UInt32 Run = Min(Target->GetRows() - Target->GetPlace(Row), Target->GetSize() - Row);

                if (!Compare(Target->GetHandles(Row) + Target->GetPlace(Row), Expected, Run))
                {
                    return false;
                }
                Expected += Run;
                Row      += Run;
            }
        }
        return true;
    }
}