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

#include "Motion2D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Motion2D::Lane> Motion2D::Find(UInt64 Bone) const
    {
        for (ConstRef<Lane> Entry : mLanes)
        {
            if (Entry.Bone == Bone)
            {
                return AddressOf(Entry);
            }
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Motion2D::Arrange(ConstSpan<SInt32> Binding, UInt32 Bones)
    {
        ZY_ASSERT(Binding.GetSize() == mLanes.GetSize(), "Every lane must be bound to a bone or to none");

        Sequence<Lane> Arranged;
        Arranged.Advance(Bones);

        Sequence<Lane> Unplaced;

        for (UInt Index = 0; Index < mLanes.GetSize(); ++Index)
        {
            const SInt32 Bone = Binding[Index];

            if (Bone >= 0 && Arranged[Bone].Bone == 0)
            {
                Arranged[Bone] = Move(mLanes[Index]);
            }
            else if (!mLanes[Index].Keys.IsEmpty())
            {
                Unplaced.Append(Move(mLanes[Index]));
            }
        }

        // A lane with no bone is kept past the last one, so bones given later can still claim it.
        const Bool Complete = Unplaced.IsEmpty();

        for (Ref<Lane> Entry : Unplaced)
        {
            Arranged.Append(Move(Entry));
        }

        mLanes = Move(Arranged);
        return Complete;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Motion2D::Sample(UInt32 Bone, UInt8 Row, UInt8 Keyframe, Ref<Placement> Output) const
    {
        if (Bone >= mLanes.GetSize())
        {
            return false;
        }

        const ConstPtr<Lane> Entry = AddressOf(mLanes[Bone]);

        ConstPtr<Key> Before = nullptr;
        ConstPtr<Key> After  = nullptr;
        ConstPtr<Key> First  = nullptr;
        ConstPtr<Key> Last   = nullptr;

        for (ConstRef<Key> Each : Entry->Keys)
        {
            if (Each.Row != Row)
            {
                continue;
            }

            if (Each.Keyframe <= Keyframe && (Before == nullptr || Each.Keyframe > Before->Keyframe))
            {
                Before = AddressOf(Each);
            }

            if (Each.Keyframe >= Keyframe && (After == nullptr || Each.Keyframe < After->Keyframe))
            {
                After = AddressOf(Each);
            }

            if (First == nullptr || Each.Keyframe < First->Keyframe)
            {
                First = AddressOf(Each);
            }

            if (Last == nullptr || Each.Keyframe > Last->Keyframe)
            {
                Last = AddressOf(Each);
            }
        }

        if (First == nullptr)
        {
            return false;
        }

        // Keyframes are counted on one line, so a key wrapped in from the other end of a lap sits a cycle away.
        const Bool Wraps = mCycle > 0;

        SInt32 From = Before ? Before->Keyframe : 0;
        SInt32 Into = After  ? After->Keyframe  : 0;

        if (Before == nullptr)
        {
            Before = Wraps ? Last : After;
            From   = Wraps ? static_cast<SInt32>(Last->Keyframe) - static_cast<SInt32>(mCycle) : Into;
        }
        else if (After == nullptr)
        {
            After = Wraps ? First : Before;
            Into  = Wraps ? static_cast<SInt32>(First->Keyframe) + static_cast<SInt32>(mCycle) : From;
        }

        const Real32 Span  = static_cast<Real32>(Into - From);
        const Real32 Share = Span > 0.0f ? static_cast<Real32>(Keyframe - From) / Span : 0.0f;

        Output.Offset = Before->Offset + (After->Offset - Before->Offset) * Share;
        Output.Roll   = Before->Roll + Angle::Between(Before->Roll, After->Roll) * Share;
        Output.Turn   = Before->Turn + (After->Turn - Before->Turn) * Share;
        Output.Scale  = Lerp(Before->Scale, After->Scale, Share);
        Output.Layer  = Share < 0.5f ? Before->Layer : After->Layer;
        return true;
    }
}