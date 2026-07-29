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

#include "Animator.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Skeleton::Pose Animator::Scratch::Acquire(UInt Bones, UInt Slot)
    {
        ZY_ASSERT(Slot < kSources, "Scratch hands out one pose per source");

        // Every pose shares one set of buffers, so a wider rig grows all of them together.
        if (const UInt Total = Bones * kSources; mPosition.GetSize() < Total)
        {
            mPosition.Resize(Total);
            mScale.Resize(Total);
            mRotation.Resize(Total);
        }

        const UInt Offset = Slot * Bones;

        return Skeleton::Pose(
            Span(mPosition.GetData() + Offset, Bones),
            Span(mScale.GetData() + Offset, Bones),
            Span(mRotation.GetData() + Offset, Bones));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Animator::Animator()
        : mClock  { 0.0 },
          mEpoch  { 0.0 },
          mFade   { 0.0 },
          mActive { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator::SetModel(ConstRetainer<Model> Model)
    {
        mModel = Model;

        // The bone count is unknown until the rig lands, so Update sizes the palette instead.
        mPalette.Clear();

        // Force a rebind: a freed rig replaced at the same address would compare equal.
        for (Ref<Source> Entry : mSources)
        {
            Entry.Bound = false;
        }

        // The pose being faded out stands on bones the arriving rig need not carry.
        Retire(mSources[mActive ^ 1]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator::Play(ConstRetainer<Animation> Animation, Repeat Mode, Real64 Fade)
    {
        // Only a clip already paired with the rig has a pose worth fading out of.
        if (Fade > 0.0 && mSources[mActive].Clip && mSources[mActive].Bound)
        {
            mActive ^= 1;
            mEpoch   = mClock;
            mFade    = Fade;
        }
        else
        {
            Retire(mSources[mActive ^ 1]);
        }

        Ref<Source> Target = mSources[mActive];
        Target.Clip  = Animation;
        Target.Bound = false;

        // A clip still loading carries neither its length nor its lanes; Rebind adopts both once they land.
        Target.Cursor.SetRepeat(Mode);
        Target.Cursor.Play();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator::Update(Real64 Time, Ref<Scratch> Scratch)
    {
        mClock = Time;

        if (!mModel)
        {
            return;
        }

        ConstRetainer<Skeleton> Rig = mModel->GetSkeleton();

        // Model and rig both load in the background; there is no pose to build until the bones arrive.
        if (!Rig || Rig->IsEmpty())
        {
            return;
        }

        const UInt Bones = Rig->GetBones().GetSize();

        if (mPalette.GetSize() != Bones)
        {
            mPalette.Resize(Bones);

            for (Ref<Source> Entry : mSources)
            {
                Entry.Bound = false;
            }
        }

        Ref<Source> Active = mSources[mActive];

        // Advanced before the pairing, so the cursor Rebind anchors is measured against this moment.
        Active.Cursor.Advance(Time);

        const Bool Ready = Rebind(Active, * Rig);

        // A clip still loading, or none at all, leaves the model in its rest pose.
        if (!Ready || !Active.Clip)
        {
            Rig->Skin(mPalette);
            return;
        }

        const Real32 Weight = mFade > 0.0
            ? static_cast<Real32>(Clamp((Time - mEpoch) / mFade, 0.0, 1.0))
            : 1.0f;

        Ref<Source> Outgoing = mSources[mActive ^ 1];

        // The outgoing clip keeps playing, so a stride caught mid-swing finishes instead of sliding.
        Bool Blending = Weight < 1.0f && Outgoing.Clip;

        if (Blending)
        {
            Outgoing.Cursor.Advance(Time);

            Blending = Rebind(Outgoing, * Rig);
        }

        if (!Blending && (mFade > 0.0 || Outgoing.Clip))
        {
            Retire(Outgoing);
        }

        Skeleton::Pose Pose = Scratch.Acquire(Bones, 0);
        Evaluate(Active, * Rig, Pose);

        if (Blending)
        {
            const Skeleton::Pose Fading = Scratch.Acquire(Bones, 1);

            Evaluate(Outgoing, * Rig, Fading);

            // Toward the outgoing clip by what is left of the fade.
            Skeleton::Blend(Pose, Fading, 1.0f - Weight);
        }

        // Folded in place, since skinning reads only each slot's own transform.
        Rig->Compose(Pose, mPalette);
        Rig->Skin(mPalette, mPalette);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Animator::Rebind(Ref<Source> Entry, ConstRef<Skeleton> Rig)
    {
        if (Entry.Bound)
        {
            return true;
        }

        // A clip still in flight binds no lane, and nothing would revisit the pairing once its bytes land.
        if (Entry.Clip && Entry.Clip->IsEmpty() && !Entry.Clip->HasFinished())
        {
            return false;
        }

        if (Entry.Clip)
        {
            Entry.Covered = Entry.Clip->Resolve(Rig, Entry.Binding);
        }
        else
        {
            Entry.Covered = false;
            Entry.Binding.Clear();
        }

        // Anchored here rather than in Play, so a clip that spent frames loading opens on its first sample.
        Entry.Cursor.SetDuration(Entry.Clip ? Entry.Clip->GetDuration() : 0.0);
        Entry.Cursor.Seek(0.0);

        Entry.Bound = true;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator::Evaluate(ConstRef<Source> Entry, ConstRef<Skeleton> Rig, ConstRef<Skeleton::Pose> Output) const
    {
        // Seeded first, so a bone the clip never names rests where the hierarchy puts it.
        if (!Entry.Covered)
        {
            Rig.Rest(Output);
        }
        Entry.Clip->Sample(Entry.Cursor.GetTime(), Entry.Binding, Output);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator::Retire(Ref<Source> Entry)
    {
        mFade = 0.0;

        Entry.Clip = Retainer<Animation>();
        Entry.Binding.Clear();
        Entry.Bound   = false;
        Entry.Covered = false;
    }
}
