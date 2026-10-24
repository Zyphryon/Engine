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

namespace ZyReplication
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Manifest::Manifest()
        : mCount  { 0 },
          mSealed { false }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::Register(UInt32 Identifier, Audience Audience, UInt8 Rate, ConstRef<Adapter> Adapter)
    {
        ZY_ASSERT(!mSealed, "A component cannot be replicated once a session opened");

        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            if (mEntries[Index].Identifier == Identifier)
            {
                return;
            }
        }

        ConstRef<ZyScene::Metatype> Info = ZyScene::Registry::Get().GetMetatype(Identifier);

        ZY_ASSERT(mCount < kMaxComponents, "Too many replicated components for one mask");
        ZY_ASSERT(Rate > 0, "A replicated component needs a rate of at least one");
        ZY_ASSERT(Info.IsTag() || (Info.Save && Info.Load), "A replicated component must write and read itself");

        mEntries[mCount++] = Entry(Identifier, AddressOf(Info), Audience, Rate, Adapter);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::Forget(UInt32 Identifier)
    {
        if (mSealed)
        {
            return;
        }

        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            if (mEntries[Index].Identifier == Identifier)
            {
                mEntries[Index] = mEntries[--mCount];
                return;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Manifest::Seal()
    {
        if (Exchange(mSealed, true))
        {
            return;
        }

        // By name, which both sides share, so the order they declared in never reaches the wire.
        for (UInt8 Index = 1; Index < mCount; ++Index)
        {
            for (UInt8 Slot = Index; Slot > 0; --Slot)
            {
                if (StrCompare(mEntries[Slot].Info->Name, mEntries[Slot - 1].Info->Name) >= 0)
                {
                    break;
                }
                mEntries[Slot - 1] = Exchange(mEntries[Slot], mEntries[Slot - 1]);
            }
        }

        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            const Audience Policy = mEntries[Index].Policy;

            if (Policy == Audience::Owner)
            {
                mOwner.Set(Index);
            }
            else
            {
                mShared.Set(Index);
            }

            if (Policy == Audience::Others)
            {
                mOthers.Set(Index);
            }
        }
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

    void Manifest::Write(Ref<Writer> Output, ZyScene::Entity Actor, Mask Wanted, Mask Dropped, Mask Delta) const
    {
        // Only the bits set are visited, so a set costs what it names, not what the table holds.
        Mask Held;

        Wanted.ForEach([this, Actor, &Held](UInt Index)
        {
            if (Actor.Owns(ZyScene::Component(mEntries[Index].Identifier)))
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
                if (!Actor.Owns(ZyScene::Component(mEntries[Index].Identifier)))
                {
                    Set.Write<UInt8>(static_cast<UInt8>(Index));
                }
            });
            Set.Write<UInt8>(kDelimiter);

            // The bytes the type saves itself as, with no size, since the other side reads them through the same type.
            const auto Present = [this, &Set, Actor, Delta](UInt Index)
            {
                ConstRef<Entry> Item = mEntries[Index];

                // A delta is laid over the value the peer holds, which carries only what changed since the last one.
                if (Delta.Test(Index) && Item.Adapter.Emit)
                {
                    Set.Write<UInt8>(static_cast<UInt8>(Index) | kDelta);
                    Item.Adapter.Emit(Set, Actor.Get(ZyScene::Component(Item.Identifier)));
                    return;
                }
                Set.Write<UInt8>(static_cast<UInt8>(Index));

                if (!Item.Info->IsTag())
                {
                    Item.Info->Save(Set, Actor.Get(ZyScene::Component(Item.Identifier)));
                }
            };
            (Held & ~mOthers).ForEach(Present);
            (Held & mOthers).ForEach(Present);
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Read(Ref<Reader> Input, ZyScene::Entity Actor, Mask Ignored) const
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

            if (Index == kDelimiter)
            {
                break;
            }

            if (Index >= mCount)
            {
                return false;
            }

            if (!Ignored.Test(Index))
            {
                Actor.Remove(ZyScene::Component(mEntries[Index].Identifier));
            }
        }

        // What is left is the entries, the ignored ones last, which the rest of the set is stepped over with.
        while (Set.GetAvailable() > 0)
        {
            const UInt8 Place = Set.Read<UInt8>();
            const UInt8 Index = Place & ~kDelta;

            if (Index >= mCount)
            {
                return false;
            }

            if (Ignored.Test(Index))
            {
                break;
            }

            ConstRef<Entry>          Item = mEntries[Index];
            const ZyScene::Component Type(Item.Identifier);

            if ((Place & kDelta) == 0)
            {
                Actor.Read(Type, Set);
                continue;
            }

            // A delta lays over the value held, so one for a tag or for a component the entity lacks is a lie.
            if (Item.Adapter.Apply == nullptr || !Actor.Has(Type))
            {
                return false;
            }
            Item.Adapter.Apply(Set, Actor.Ensure(Type));
            Actor.Notify(Type);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Manifest::Skip(Ref<Reader> Input) const
    {
        return !Input.ReadBlock<UInt32, Byte>().IsEmpty();
    }
}