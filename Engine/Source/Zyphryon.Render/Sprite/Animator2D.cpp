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

#include "Animator2D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Sheet2D::Clip> Animator2D::Resolve(Real64 Timestamp)
    {
        if (!mSheet || !mSheet->HasCompleted() || mClip == 0)
        {
            return nullptr;
        }

        const ConstSpan<Sheet2D::Clip> Clips = mSheet->GetClips();

        if (mSlot < Clips.GetSize() && Clips[mSlot].Name == mClip)
        {
            return AddressOf(Clips[mSlot]);
        }

        const ConstPtr<Sheet2D::Clip> Entry = mSheet->FindClip(mClip);

        if (Entry == nullptr)
        {
            mSlot = kUnresolved;
            return nullptr;
        }

        mSlot = static_cast<UInt16>(Entry - Clips.GetData());
        Restart(Entry->Frames, Timestamp, mPhase);
        return Entry;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator2D::Restart(ConstRef<Animation2D> Sequence, Real64 Timestamp, Real64 Phase)
    {
        const Real64 Duration = Sequence.GetDuration();
        const Real32 Rate     = GetRate();

        // Move the clock to the start time first, so playback is anchored there and not at the last advance.
        mPlayback.Advance(Timestamp);
        mPlayback.SetDuration(Duration);

        switch (Sequence.GetStatus())
        {
        case Animation2D::Status::Forward:
            mPlayback.SetRepeat(Repeat::Once);
            mPlayback.SetSpeed(Rate);
            mPlayback.Seek(Phase * Duration);
            mPlayback.Play();
            break;
        case Animation2D::Status::Backward:
            mPlayback.SetRepeat(Repeat::Once);
            mPlayback.SetSpeed(-Rate);
            mPlayback.Seek((1.0 - Phase) * Duration);
            mPlayback.Play();
            break;
        case Animation2D::Status::Repeat:
            mPlayback.SetRepeat(Repeat::Loop);
            mPlayback.SetSpeed(Rate);
            mPlayback.Seek(Phase * Duration);
            mPlayback.Play();
            break;
        case Animation2D::Status::Mirror:
            mPlayback.SetRepeat(Repeat::Mirror);
            mPlayback.SetSpeed(Rate);
            mPlayback.Seek(Phase * Duration);
            mPlayback.Play();
            break;
        }

        mKeyframe = Locate(Sequence);

        // A clip joined partway is already on its frame, so that frame is not reported as entered.
        if (Phase > 0.0)
        {
            mPlayback.Advance(Timestamp);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator2D::Step(Real64 Time, ConstRef<Animation2D> Sequence, Ref<Visited> Visited)
    {
        // A cursor just started or moved has entered the frame it stands on.
        if (mPlayback.IsFresh())
        {
            Visited.Append(mKeyframe);
        }

        const UInt32 Count = Sequence.GetCount();

        if (Count < 2)
        {
            mPlayback.Advance(Time);
            mKeyframe = 0;
            return;
        }

        const Real64 From = mPlayback.GetElapsed();
        mPlayback.Advance(Time);
        const Real64 Into = mPlayback.GetElapsed();

        UInt8 At  = mKeyframe;
        mKeyframe = Locate(Sequence);

        // Report every frame passed on the way, lap by lap, not only the one landed on.
        const Bool   Rising = Into >= From;
        const UInt8  Last   = static_cast<UInt8>(Count - 1);
        const SInt64 Final  = GetPass(Into);
        const SInt64 Step   = Rising ? 1 : -1;

        for (SInt64 Pass = GetPass(From); Pass != Final;)
        {
            const Bool  Ascending = IsAscending(Pass, Rising);
            const UInt8 Exit      = Ascending ? Last : 0;
            Walk(Visited, At, Exit, Ascending);

            Pass += Step;

            // A loop wraps onto the far edge, while a mirror turns on the edge it reached.
            const UInt8 Entry = IsAscending(Pass, Rising) ? 0 : Last;

            if (Entry != Exit)
            {
                Visited.Append(Entry);
            }
            At = Entry;

            // Beyond one whole lap nothing new is entered, and a mirror skips laps in pairs to keep its direction.
            SInt64 Skip = (Final - Pass) * Step - 1;

            if (mPlayback.GetRepeat() == Repeat::Mirror)
            {
                Skip &= ~static_cast<SInt64>(1);
            }

            if (Skip > 0)
            {
                Pass += Step * Skip;
            }
        }
        Walk(Visited, At, mKeyframe, IsAscending(Final, Rising));
    }
}