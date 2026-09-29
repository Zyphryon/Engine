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

#include "Layout.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Layout::Layout()
        : mPending { Unique<Chunk>::Create(~0u, 0, false, ConstSpan<UInt32>()) }
    {
        mChunks.Append(Unique<Chunk>::Create(0, 0, false, ConstSpan<UInt32>()));

        mSignature.Assign(GetKey(ConstSpan<UInt32>(), 0), GetRoot());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Layout::Find(ConstSpan<UInt32> Signature, UInt32 Base) const
    {
        for (UInt64 Key = GetKey(Signature, Base);; ++Key)
        {
            const ConstPtr<Ptr<Chunk>> Found = mSignature.Find(Key);

            if (!Found)
            {
                return nullptr;
            }

            ConstSpan<UInt32> Known = (* Found)->GetSignature();

            if ((* Found)->GetBase() == Base
                && Known.GetSize() == Signature.GetSize()
                && Compare(Known.GetData(), Signature.GetData(), Signature.GetSize()))
            {
                return * Found;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Chunk> Layout::Create(ConstSpan<UInt32> Signature, UInt32 Base, UInt32 Marker, ConstRef<Directory> Slots)
    {
        const UInt32     Index     = static_cast<UInt32>(mChunks.GetSize());
        const Bool       Archetype = Signature.Contains(Marker);
        const Ptr<Chunk> Made      = mChunks.Append(Unique<Chunk>::Create(Index, Base, Archetype, Signature)).Grab();

        UInt64 Key = GetKey(Signature, Base);

        while (mSignature.Contains(Key))
        {
            ++Key;
        }
        mSignature.Assign(Key, Made);

        if (Base)
        {
            GetLineage(Base).Heirs.Append(Made);
        }
        Update(Made, Slots);
        return Made;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::Destroy(Ptr<Chunk> Target)
    {
        // A different set filed alike sits one key further.
        UInt64 Key = GetKey(Target->GetSignature(), Target->GetBase());

        while (!mSignature.Erase(Key, Target))
        {
            ++Key;
        }

        for (ConstRef<Unique<Selection>> State : mSelections)
        {
            State->Remove(Target);
        }
        mChunks[Target->GetIndex()].Reset();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::SetCopy(UInt32 Archetype, Ptr<Chunk> Holder, Bool AsArchetype, Ptr<Chunk> Target)
    {
        Ref<Lineage> Line = GetLineage(Archetype);

        if (Line.Shape != Holder)
        {
            Line.Shape  = Holder;
            Line.Copies = Array<Ptr<Chunk>, 2>(nullptr, nullptr);
        }
        Line.Copies[AsArchetype] = Target;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::UpdateHeirs(UInt32 Archetype, ConstRef<Directory> Slots)
    {
        if (GetHeirChunks(Archetype).IsEmpty())
        {
            return;
        }

        // The archetypes made from it lend through it, so what they lend changed too.
        Sequence<UInt32> Pending;
        Pending.Append(Archetype);

        for (UInt32 Next = 0; Next < Pending.GetSize(); ++Next)
        {
            for (const Ptr<Chunk> Target : GetHeirChunks(Pending[Next]))
            {
                Update(Target, Slots);

                for (UInt32 Row = 0; Target->IsArchetype() && Row < Target->GetSize(); ++Row)
                {
                    Pending.Append(Target->GetHandle(Row).GetIndex());
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Ptr<Chunk>> Layout::ExtractHeirs(UInt32 Archetype)
    {
        const UInt32 Kin = Archetype - kMinRangeArchetypes - 1;

        if (Kin >= mLineages.GetSize())
        {
            return Sequence<Ptr<Chunk>>();
        }

        Ref<Lineage> Line = mLineages[Kin];
        Line.Shape  = nullptr;
        Line.Copies = Array<Ptr<Chunk>, 2>(nullptr, nullptr);
        return ZyBase::Move(Line.Heirs);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Selection> Layout::AddSelection(AnyRef<Unique<Selection>> State, ConstRef<Directory> Slots)
    {
        Populate(* State, Slots);
        return mSelections.Append(ZyBase::Move(State)).Grab();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::RemoveSelection(ConstPtr<Selection> State)
    {
        mSelections.RemoveIf([State](ConstRef<Unique<Selection>> Known)
        {
            return AddressOf(* Known) == State;
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::Populate(Ref<Selection> State, ConstRef<Directory> Slots) const
    {
        for (ConstRef<Unique<Chunk>> Target : mChunks)
        {
            if (Target)
            {
                State.Update(AddressOf(* Target), Slots);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Layout::Update(Ptr<Chunk> Target, ConstRef<Directory> Slots)
    {
        for (ConstRef<Unique<Selection>> State : mSelections)
        {
            State->Update(Target, Slots);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Layout::Lineage> Layout::GetLineage(UInt32 Archetype)
    {
        const UInt32 Kin = Archetype - kMinRangeArchetypes - 1;

        if (Kin >= mLineages.GetSize())
        {
            mLineages.Advance(Kin + 1 - mLineages.GetSize());
        }
        return mLineages[Kin];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Layout::GetKey(ConstSpan<UInt32> Signature, UInt32 Base)
    {
        return Hash(reinterpret_cast<ConstPtr<Char>>(Signature.GetData()), Signature.GetSizeInBytes()) ^ Hash(Base);
    }
}