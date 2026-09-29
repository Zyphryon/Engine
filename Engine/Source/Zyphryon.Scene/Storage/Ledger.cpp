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

#include "Ledger.hpp"
#include "Chunk.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ledger::Ledger()
        : mCount { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ledger::~Ledger()
    {
        for (UInt32 Index = 0; Index < mLists.GetSize(); ++Index)
        {
            if (mLists[Index])
            {
                RemoveFront(Index, mLists[Index]->Actors.GetSize());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Ledger::Subscribe(UInt32 Identifier, Pull Kind)
    {
        const UInt32 Index = GetListIndex(Identifier, Kind);

        if (Index >= mLists.GetSize())
        {
            mLists.Advance(Index + 1 - mLists.GetSize());
        }

        if (!mLists[Index])
        {
            mLists[Index] = Unique<List>::Create(Identifier);
        }

        if (Identifier >= mWatched.GetSize())
        {
            mWatched.Advance(Identifier + 1 - mWatched.GetSize());
        }
        mWatched[Identifier] |= static_cast<UInt8>(Kind);

        ++mLists[Index]->Subscribers;
        ++mCount;

        // A destroyed reader's slot is taken first, so handles stay few.
        const Subscription Made(Index, mLists[Index]->First + mLists[Index]->Actors.GetSize());
        const SInt Vacant = mSubscriptions.Find([](ConstRef<Subscription> Other)
        {
            return Other.List == kVacant;
        });

        if (Vacant < 0)
        {
            mSubscriptions.Append(Made);
            return static_cast<UInt32>(mSubscriptions.GetSize());
        }
        mSubscriptions[Vacant] = Made;
        return static_cast<UInt32>(Vacant) + 1;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::Unsubscribe(UInt32 Subscription)
    {
        if (Subscription == 0
            || Subscription > mSubscriptions.GetSize()
            || mSubscriptions[Subscription - 1].List == kVacant)
        {
            return;
        }

        const UInt32 Index  = mSubscriptions[Subscription - 1].List;
        Ref<List>    Target = * mLists[Index];

        mSubscriptions[Subscription - 1].List = kVacant;
        --mCount;

        if (--Target.Subscribers == 0)
        {
            mWatched[Target.Identifier] &= static_cast<UInt8>(~(1u << (Index % 3)));
        }
        Trim(Index);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::Record(UInt32 Identifier, Pull Kind, Handle Actor, ConstPtr<Byte> Value)
    {
        Ref<List> Target = * mLists[GetListIndex(Identifier, Kind)];
        Target.Actors.Append(Actor);

        if (!Value || Kind != Pull::Removed)
        {
            return;
        }

        ConstRef<Metatype> Info = Registry::Get().GetMetatype(Identifier);

        if (Info.Has(Trait::Keeps) && !Info.IsTag())
        {
            // Entries recorded since the last value kept none, so they are padded before this one is added.
            Target.Values.Advance(Target.Actors.GetSize() - 1 - Target.Values.GetSize());
            Target.Values.Append(Allocate<Byte>(Info.GetSize(), Info.GetAlignment()));
            Chunk::Copy(Info, Target.Values.GetBack(), Value, 1);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::SetPosition(UInt32 Subscription, UInt64 Position)
    {
        mSubscriptions[Subscription - 1].Position = Position;
        Trim(mSubscriptions[Subscription - 1].List);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::Retire(UInt32 Identifier)
    {
        const UInt32 Index = GetListIndex(Identifier, Pull::Removed);

        if (Index >= mLists.GetSize() || !mLists[Index])
        {
            return;
        }

        ConstRef<Metatype> Info = Registry::Get().GetMetatype(Identifier);

        for (Ref<Ptr<Byte>> Value : mLists[Index]->Values)
        {
            if (Value)
            {
                Chunk::Destruct(Info, Value, 1);
                Free(Exchange(Value, nullptr), Info.GetAlignment());
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::Trim(UInt32 Index)
    {
        ConstRef<List> Target = * mLists[Index];
        UInt64         Oldest = Target.First + Target.Actors.GetSize();

        for (ConstRef<Subscription> Other : mSubscriptions)
        {
            if (Other.List == Index)
            {
                Oldest = Min(Oldest, Other.Position);
            }
        }

        const UInt Passed = Oldest - Target.First;

        // Erasing from the front moves what is left, so it waits until that is at most as much as what goes.
        if (Passed > 0 && Passed * 2 >= Target.Actors.GetSize())
        {
            RemoveFront(Index, Passed);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Ledger::RemoveFront(UInt32 Index, UInt Count)
    {
        Ref<List> Target = * mLists[Index];

        DropFront(Target.Actors, Count);
        Target.First += Count;

        // Only a list keeping values holds any, and never for more entries than it has.
        if (const UInt Kept = Min(Count, Target.Values.GetSize()))
        {
            ConstRef<Metatype> Info = Registry::Get().GetMetatype(Target.Identifier);

            for (UInt Item = 0; Item < Kept; ++Item)
            {
                if (const Ptr<Byte> Value = Target.Values[Item])
                {
                    Chunk::Destruct(Info, Value, 1);
                    Free(Value, Info.GetAlignment());
                }
            }
            DropFront(Target.Values, Kept);
        }
    }
}