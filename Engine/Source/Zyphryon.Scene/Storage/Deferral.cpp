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

#include "Deferral.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Deferral::Deferral()
        : mDepth     { 0 },
          mFlushing  { false },
          mSpreading { false },
          mShares    { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Deferral::~Deferral()
    {
        // Applying a change moved or destroyed its value, so only the memory is left.
        Clear();

        mValues.Release();

        for (Ref<Lane> Share : mLanes)
        {
            Share.Values.Release();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Spread(UInt32 Shares, UInt32 Slots)
    {
        ZY_ASSERT(!mSpreading, "A spread walk cannot start inside another");

        // Grown here, on one thread, since a worker marks and holds without ever growing either.
        if (mLanes.GetSize() < Shares)
        {
            mLanes.Advance(Shares - mLanes.GetSize());
        }

        if (mMarks.GetSize() < Slots)
        {
            mMarks.Advance(Slots - mMarks.GetSize());
        }

        mShares    = Shares;
        mSpreading = true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Bind(UInt32 Share)
    {
        ZY_ASSERT(mSpreading && Share < mShares, "A share is bound only while its walk is spread");

        GetShare() = AddressOf(mLanes[Share]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Gather()
    {
        ZY_ASSERT(mSpreading, "Only a spread walk gathers its shares");

        // Share order is row order, so the list reads as the same walk on one thread would have held it.
        for (UInt32 Share = 0; Share < mShares; ++Share)
        {
            Ref<Sequence<Command>> Held = mLanes[Share].Commands;

            for (ConstRef<Command> Change : Held)
            {
                mCommands.Append(Change);
            }
            Held.Clear();
        }

        mShares    = 0;
        mSpreading = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Queue(ConstRef<Command> Change, Bool Mark)
    {
        const UInt32 Index = Change.Actor.GetIndex();

        if (mSpreading)
        {
            ZY_ASSERT(GetShare(), "A change held while a walk is spread comes from one of its shares");

            if (Mark)
            {
                mMarks[Index] = 1;
            }
            GetShare()->Commands.Append(Change);
            return;
        }

        if (Mark)
        {
            if (Index >= mMarks.GetSize())
            {
                mMarks.Advance(Index + 1 - mMarks.GetSize());
            }
            mMarks[Index] = 1;
        }
        mCommands.Append(Change);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Deferral::Allocate(UInt Size, UInt Alignment)
    {
        if (mSpreading)
        {
            ZY_ASSERT(GetShare(), "A value held while a walk is spread comes from one of its shares");

            return GetShare()->Values.Allocate(Size, Alignment);
        }
        return mValues.Allocate(Size, Alignment);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Clear()
    {
        for (ConstRef<Command> Change : mCommands)
        {
            if (Change.Actor.GetIndex() < mMarks.GetSize())
            {
                mMarks[Change.Actor.GetIndex()] = 0;
            }
        }

        mCommands.Clear();
        mValues.Reset();

        for (Ref<Lane> Share : mLanes)
        {
            Share.Values.Reset();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<Ptr<Deferral::Lane>> Deferral::GetShare()
    {
        thread_local Ptr<Lane> Share = nullptr;
        return Share;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> Deferral::Room::Allocate(UInt Size, UInt Alignment)
    {
        ZY_ASSERT(Alignment <= kPageAlignment, "A value is aligned wider than a page of values");

        // A value larger than half a page gets its own block, so pages stay mostly full.
        if (Size > kPageBytes / 2)
        {
            return Oversized.Append(ZyBase::Allocate<Byte>(Size, kPageAlignment));
        }

        UInt Offset = Align(Used, Alignment);

        if (Page == Pages.GetSize() || Offset + Size > kPageBytes)
        {
            if (Page < Pages.GetSize() && Used > 0)
            {
                ++Page;
            }

            if (Page == Pages.GetSize())
            {
                Pages.Append(ZyBase::Allocate<Byte>(kPageBytes, kPageAlignment));
            }
            Offset = 0;
        }

        Used = static_cast<UInt32>(Offset + Size);
        return Pages[Page] + Offset;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Room::Reset()
    {
        for (const Ptr<Byte> Block : Oversized)
        {
            Free(Block, kPageAlignment);
        }

        Oversized.Clear();
        Page = 0;
        Used = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Deferral::Room::Release()
    {
        Reset();

        for (const Ptr<Byte> Block : Pages)
        {
            Free(Block, kPageAlignment);
        }
        Pages.Clear();
    }
}