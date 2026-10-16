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

#include "Manifest.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::Protocol
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Manifest::Manifest()
        : mCount { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Manifest::GetHash() const
    {
        UInt64 Result = 0;

        // The name rather than the identifier, since only the other side's declarations have to agree.
        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            ConstRef<Entry> Item = mEntries[Index];

            Result = HashCombine(
                Result,
                Item.Info->Name,
                static_cast<UInt64>(Item.Policy),
                static_cast<UInt64>(Item.Rate));
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Mask Manifest::Write(Ref<Writer> Output, Entity Actor, Mask Wanted, Mask Dropped) const
    {
        // Only the bits set are visited, so a set costs what it names, not what the table holds.
        Mask Held;

        Wanted.ForEach([this, &Output, Actor, &Held](UInt Index)
        {
            WriteEntry(Output, Actor, static_cast<UInt8>(Index), Held);
        });
        Output.Write<UInt8>(kEnd);

        // Asked for and not held is as gone as dropped, while dropped and held again stays, even when its change waits.
        const Mask Absent = (Wanted | Dropped) & ~Held;

        Absent.ForEach([this, &Output, Actor](UInt Index)
        {
            WriteAbsent(Output, Actor, static_cast<UInt8>(Index));
        });
        Output.Write<UInt8>(kEnd);
        return Held;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Read(Ref<Reader> Input, Entity Actor, Mask Ignored) const
    {
        while (true)
        {
            if (Input.GetAvailable() == 0)
            {
                return false;
            }

            const UInt8 Index = Input.Read<UInt8>();

            if (Index == kEnd)
            {
                break;
            }

            if (!IsWhole(Input, Index))
            {
                return false;
            }

            // Read past without an entity, which is how a set is checked whole before it is applied.
            if (Ignored.Test(Index) || !Actor.IsAlive())
            {
                Input.Skip(Input.Read<UInt32>());
            }
            else
            {
                Actor.Read(Component(mEntries[Index].Identifier), Input);
            }
        }

        while (true)
        {
            if (Input.GetAvailable() == 0)
            {
                return false;
            }

            const UInt8 Index = Input.Read<UInt8>();

            if (Index == kEnd)
            {
                return true;
            }

            if (Index >= mCount)
            {
                return false;
            }

            if (!Ignored.Test(Index) && Actor.IsAlive())
            {
                Actor.Remove(Component(mEntries[Index].Identifier));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Skip(Ref<Reader> Input) const
    {
        return Read(Input, Entity(), Mask());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Manifest> Manifest::Get()
    {
        static Manifest Instance;
        return Instance;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::WriteEntry(Ref<Writer> Output, Entity Actor, UInt8 Index, Ref<Mask> Held) const
    {
        ConstRef<Entry> Item = mEntries[Index];
        const Component Type(Item.Identifier);

        // What the entity only inherits comes with its archetype on the other side.
        if (!Actor.Owns(Type))
        {
            return;
        }
        Output.Write<UInt8>(Index);
        Held.Set(Index);

        // The same block an entity writes of itself, so the other side reads it through the entity.
        if (Item.Info->IsTag())
        {
            Output.Write<UInt32>(0);
        }
        else
        {
            const ConstPtr<void> Value = Actor.Get(Type);

            Output.WriteBlock<UInt32>([&Item, Value](Ref<Writer> Block)
            {
                Item.Info->Save(Block, Value);
            });
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::WriteAbsent(Ref<Writer> Output, Entity Actor, UInt8 Index) const
    {
        if (!Actor.Owns(Component(mEntries[Index].Identifier)))
        {
            Output.Write<UInt8>(Index);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::IsWhole(Ref<Reader> Input, UInt8 Index) const
    {
        if (Index >= mCount || Input.GetAvailable() < sizeof(UInt32))
        {
            return false;
        }
        return Input.Peek<UInt32>() <= Input.GetAvailable() - sizeof(UInt32);
    }
}