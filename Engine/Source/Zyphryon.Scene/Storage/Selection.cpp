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

#include "Selection.hpp"
#include "Directory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Selection::Matches(ConstRef<Chunk> Target, ConstRef<Directory> Slots) const
    {
        const UInt32 Base = Target.GetBase();

        for (const UInt32 Identifier : Owned)
        {
            if (!Target.Has(Identifier))
            {
                return false;
            }
        }

        for (const UInt32 Identifier : Required)
        {
            if (!Target.Has(Identifier) && !Slots.FindLent(Base, Identifier))
            {
                return false;
            }
        }

        for (const UInt32 Identifier : Excluded)
        {
            if (Target.Has(Identifier) || Slots.FindLent(Base, Identifier))
            {
                return false;
            }
        }

        for (ConstRef<Sequence<UInt32>> Group : Alternatives)
        {
            Bool Carried = false;

            for (UInt32 Index = 0; !Carried && Index < Group.GetSize(); ++Index)
            {
                Carried = Target.Has(Group[Index]) || Slots.FindLent(Base, Group[Index]);
            }

            if (!Carried)
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Selection::Update(Ptr<Chunk> Target, ConstRef<Directory> Slots)
    {
        // Pending and destroyed chunks are never updated, and the pending one sits past every position.
        if (!Matches(* Target, Slots))
        {
            Remove(Target);
            return;
        }

        const UInt32 Index = Target->GetIndex();

        if (Index >= Positions.GetSize())
        {
            Positions.Advance(Index + 1 - Positions.GetSize());
        }

        if (Positions[Index] == 0)
        {
            Chunks.Append(Target);
            Positions[Index] = static_cast<UInt32>(Chunks.GetSize());
            Sources.Advance(Fields.GetSize());

            // A new match holds rows the last sort never saw.
            Snapshot.Clear();
        }

        // What the chunk does not hold comes from its archetype, the world, an ancestor or nowhere.
        const UInt32 First = (Positions[Index] - 1) * static_cast<UInt32>(Fields.GetSize());

        for (UInt32 Field = 0; Field < Fields.GetSize(); ++Field)
        {
            const SInt16 Column = Target->Find(Fields[Field]);
            UInt16       Source = kMissing;

            switch (Modes[Field])
            {
            case Mode::Global:
                Source = kGlobal;
                break;
            case Mode::Ancestor:
                Source = kAncestor;
                break;
            case Mode::Read:
                if (Column >= 0)
                {
                    Source = static_cast<UInt16>(Column);
                }
                else if (Slots.FindLent(Target->GetBase(), Fields[Field]))
                {
                    Source = kLent;
                }
                break;
            case Mode::Write:
                if (Column >= 0)
                {
                    Source = static_cast<UInt16>(Column);
                }
                break;
            }
            Sources[First + Field] = Source;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Selection::Remove(ConstPtr<Chunk> Target)
    {
        const UInt32 Place = GetPosition(* Target);

        if (Place == 0)
        {
            return;
        }

        // The matches after it move one place down, keeping the order walks visit them in.
        Chunks.Remove(Place - 1);
        Snapshot.Clear();

        if (!Fields.IsEmpty())
        {
            Sources.Remove((Place - 1) * Fields.GetSize(), Fields.GetSize());
        }

        for (UInt32 Match = Place - 1; Match < Chunks.GetSize(); ++Match)
        {
            Positions[Chunks[Match]->GetIndex()] = Match + 1;
        }
        Positions[Target->GetIndex()] = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt Selection::Count() const
    {
        static constexpr auto Accumulator = [](Ptr<Chunk> Target)
        {
            return Target->GetSize();
        };
        return Accumulate(Chunks.GetData(), Chunks.GetSize(), 0, Accumulator);
    }
}