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

    void Animator2D::Play(UInt64 Clip, Real64 Timestamp, Real32 Phase, Bool Back, Real64 Length)
    {
        // A clip played once over another keeps the loop the first one cut, as only a loop is gone back to.
        if (!Back)
        {
            mReturn = 0;
        }
        else if (const ConstPtr<Sheet2D::Clip> Leaving = GetClip(); Leaving && !Leaving->Frames.IsOnce())
        {
            mReturn = Leaving->Name;
            mResume = GetPhase();
        }

        mClip   = Clip;
        mPhase  = Phase;
        mLength = Length;
        mSlot   = kUnresolved;

        Resolve(Timestamp);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Sheet2D::Clip> Animator2D::Resolve(Real64 Timestamp)
    {
        if (const ConstPtr<Sheet2D::Clip> Playing = GetClip())
        {
            return Playing;
        }

        // A slot that lost its clip is let go, so a sheet loading again starts the clip over once it lands.
        mSlot = kUnresolved;

        if (!mSheet || !mSheet->HasCompleted() || mClip == 0)
        {
            return nullptr;
        }

        const ConstPtr<Sheet2D::Clip> Entry = mSheet->FindClip(mClip);

        if (Entry == nullptr)
        {
            return nullptr;
        }

        mSlot = static_cast<UInt16>(Entry - mSheet->GetClips().GetData());
        Restart(Entry->Frames, Timestamp, mPhase);
        return Entry;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator2D::Restart(ConstRef<Animation2D> Frames, Real64 Timestamp, Real32 Phase)
    {
        // The repeat each status plays under, in declared order, and the backward one alone runs from its end.
        static constexpr Array kRepeats(Repeat::Once, Repeat::Once, Repeat::Loop, Repeat::Mirror);

        const Real64 Duration = Frames.GetDuration();

        // A speed of zero compares the same either way, so the direction is kept apart from it.
        mBackward = (Frames.GetStatus() == Animation2D::Status::Backward);

        // Move the clock to the start time first, so playback is anchored there and not at the last advance.
        mPlayback.Advance(Timestamp);
        mPlayback.SetDuration(Duration);
        mPlayback.SetRepeat(kRepeats[ZyEnum::Cast(Frames.GetStatus())]);
        mPlayback.SetSpeed(GetSpeed());
        mPlayback.Seek((mBackward ? 1.0 - Phase : Phase) * Duration);
        mPlayback.Play();

        mKeyframe = Locate(Frames);

        // A clip joined partway is already on its frame, so that frame is not reported as entered.
        if (Phase > 0.0)
        {
            mPlayback.Advance(Timestamp);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator2D::Step(Real64 Time, ConstRef<Animation2D> Frames, Ref<Visited> Visited)
    {
        // A cursor just started or moved has entered the frame it stands on.
        if (mPlayback.IsFresh() && !Visited.IsFull())
        {
            Visited.Append(mKeyframe);
        }

        const UInt32 Count = Frames.GetCount();

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
        mKeyframe = Locate(Frames);

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

            if (Entry != Exit && !Visited.IsFull())
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