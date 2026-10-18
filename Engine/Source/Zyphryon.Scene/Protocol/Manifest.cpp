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
                static_cast<UInt64>(Item.Info->GetSize()),
                static_cast<UInt64>(Item.Policy));
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::Write(Ref<Writer> Output, Entity Actor, Mask Wanted, Mask Dropped) const
    {
        // Only the bits set are visited, so a set costs what it names, not what the table holds.
        Mask Held;

        Wanted.ForEach([this, Actor, &Held](UInt Index)
        {
            if (Actor.Owns(Component(mEntries[Index].Identifier)))
            {
                Held.Set(Index);
            }
        });

        // Asked for and not held is as gone as dropped, while dropped and held again stays, even when its change waits.
        const Mask Absent = (Wanted | Dropped) & ~Held;

        // The length lets a set be stepped over whole, and the entries the owner ignores come last, so it stops there.
        Output.WriteBlock<UInt32>([&](Ref<Writer> Set)
        {
            Absent.ForEach([this, &Set, Actor](UInt Index)
            {
                WriteAbsent(Set, Actor, static_cast<UInt8>(Index));
            });
            Set.Write<UInt8>(kEnd);

            (Held & ~mOthers).ForEach([this, &Set, Actor](UInt Index)
            {
                WriteEntry(Set, Actor, static_cast<UInt8>(Index));
            });
            (Held & mOthers).ForEach([this, &Set, Actor](UInt Index)
            {
                WriteEntry(Set, Actor, static_cast<UInt8>(Index));
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Read(Ref<Reader> Input, Entity Actor, Mask Ignored) const
    {
        ZY_ASSERT((Ignored & ~mOthers).None(), "Only the components the owner ignores may be read past");

        // Every entry is read within the set, so a malformed one cannot reach the messages after it.
        Reader Set(Input.ReadBlock<UInt32, Byte>());

        while (true)
        {
            if (Set.GetAvailable() == 0)
            {
                return false;
            }

            const UInt8 Index = Set.Read<UInt8>();

            if (Index == kEnd)
            {
                break;
            }

            if (Index >= mCount)
            {
                return false;
            }

            if (!Ignored.Test(Index))
            {
                Actor.Remove(Component(mEntries[Index].Identifier));
            }
        }

        // What is left is the entries, the ignored ones last, which the rest of the set is stepped over with.
        while (Set.GetAvailable() > 0)
        {
            const UInt8 Index = Set.Read<UInt8>();

            if (Index >= mCount)
            {
                return false;
            }

            if (Ignored.Test(Index))
            {
                break;
            }
            Actor.Receive(Component(mEntries[Index].Identifier), Set);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Skip(Ref<Reader> Input) const
    {
        return !Input.ReadBlock<UInt32, Byte>().IsEmpty();
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

    void Manifest::WriteEntry(Ref<Writer> Output, Entity Actor, UInt8 Index) const
    {
        ConstRef<Entry> Item = mEntries[Index];

        Output.Write<UInt8>(Index);

        // The same bytes the type saves itself as, with no size, since the other side reads them through the same type.
        if (!Item.Info->IsTag())
        {
            Item.Info->Save(Output, Actor.Get(Component(Item.Identifier)));
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
}