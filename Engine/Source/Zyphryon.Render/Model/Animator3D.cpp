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

#include "Animator3D.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Skeleton3D::Pose Animator3D::Scratch::Acquire(UInt Bones, UInt Slot)
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

        return Skeleton3D::Pose(
            Span(mPosition.GetData() + Offset, Bones),
            Span(mScale.GetData() + Offset, Bones),
            Span(mRotation.GetData() + Offset, Bones));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Animator3D::Animator3D()
        : mEpoch  { 0.0 },
          mFade   { 0.0 },
          mBack   { 0.0 },
          mReturn { 0 },
          mResume { 0.0f },
          mRate   { 1.0f },
          mActive { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::SetModel(ConstRetainer<Model3D> Model)
    {
        mModel  = Model;
        mReturn = 0;

        // The bone count is unknown until the rig lands, so sizing the palette again pairs every source with it anew.
        mPalette.Clear();

        // The pose being faded out stands on bones the arriving rig need not carry.
        Retire(mSources[mActive ^ 1]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Play(UInt64 Clip, Real64 Timestamp, Real32 Phase, Real64 Fade, Bool Back)
    {
        // A clip played once over another keeps the loop the first one cut, as only a loop is gone back to.
        if (!Back)
        {
            mReturn = 0;
        }
        else if (const ConstPtr<Model3D::Clip> Leaving = GetClip(); Leaving && Leaving->Mode == Repeat::Loop)
        {
            mReturn = Leaving->Name;
            mResume = GetPhase();
            mBack   = Fade;
        }

        // Only a clip already paired with the rig has a pose worth fading out of.
        if (Fade > 0.0 && mSources[mActive].Motion && mSources[mActive].Bound)
        {
            mActive ^= 1;
            mEpoch   = Timestamp;
            mFade    = Fade;
        }
        else
        {
            Retire(mSources[mActive ^ 1]);
        }

        Ref<Source> Incoming = mSources[mActive];
        Incoming.Name      = Clip;
        Incoming.Timestamp = Timestamp;
        Incoming.Phase     = Phase;
        Incoming.Bound     = false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Skeleton3D::Pose Animator3D::Sample(Real64 Time, Ref<Scratch> Scratch)
    {
        // The rig and every clip load with the model, so there is nothing to pose until it has landed.
        if (!mModel || !mModel->HasCompleted())
        {
            mPalette.Clear();
            return Skeleton3D::Pose();
        }

        ConstRetainer<Skeleton3D> Rig = mModel->GetSkeleton();

        if (!Rig || Rig->IsEmpty())
        {
            return Skeleton3D::Pose();
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

        // A clip played once that ran out on the last advance hands the rig back to the loop it cut.
        if (mReturn != 0 && HasEnded())
        {
            Play(mReturn, Time, mResume, mBack);
        }

        Ref<Source> Active = mSources[mActive];

        // Paired before the advance, so the clip runs from its timestamp however long the model took to land.
        Rebind(Active, * Rig);
        Active.Cursor.Advance(Time);

        // No clip, or one the model does not list, leaves the model in its rest pose.
        if (!Active.Motion)
        {
            return Skeleton3D::Pose();
        }

        const Real32 Weight = mFade > 0.0
            ? static_cast<Real32>(Clamp((Time - mEpoch) / mFade, 0.0, 1.0))
            : 1.0f;

        Ref<Source> Outgoing = mSources[mActive ^ 1];

        // The outgoing clip keeps playing, so a stride caught mid-swing finishes instead of sliding.
        const Bool Blending = Weight < 1.0f && Outgoing.Motion;

        if (Blending)
        {
            Rebind(Outgoing, * Rig);
            Outgoing.Cursor.Advance(Time);
        }
        else if (mFade > 0.0 || Outgoing.Motion)
        {
            Retire(Outgoing);
        }

        Skeleton3D::Pose Pose = Scratch.Acquire(Bones, 0);
        Evaluate(Active, * Rig, Pose);

        if (Blending)
        {
            const Skeleton3D::Pose Fading = Scratch.Acquire(Bones, 1);

            Evaluate(Outgoing, * Rig, Fading);

            // Toward the outgoing clip by what is left of the fade.
            Skeleton3D::Blend(Pose, Fading, 1.0f - Weight);
        }

        return Pose;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Compose(ConstRef<Skeleton3D::Pose> Pose)
    {
        if (mPalette.IsEmpty())
        {
            return;
        }

        ConstRetainer<Skeleton3D> Rig = mModel->GetSkeleton();

        if (Pose.GetSize() == 0)
        {
            Rig->Skin(mPalette);
        }
        else
        {
            // Folded in place, since skinning reads only each slot's own transform.
            Rig->Compose(Pose, mPalette);
            Rig->Skin(mPalette, mPalette);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Update(Real64 Time, Ref<Scratch> Scratch)
    {
        Compose(Sample(Time, Scratch));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Rebind(Ref<Source> Entry, ConstRef<Skeleton3D> Rig)
    {
        if (Entry.Bound)
        {
            return;
        }

        const ConstPtr<Model3D::Clip> Clip = mModel->FindClip(Entry.Name);

        // A name the model does not list plays nothing, as a source holding no clip does.
        Entry.Slot    = Clip ? static_cast<UInt16>(Clip - mModel->GetClips().GetData()) : kUnresolved;
        Entry.Motion  = Clip ? Clip->Motion : Retainer<Animation3D>();
        Entry.Rate    = Clip ? Clip->Rate : 1.0f;
        Entry.Covered = Entry.Motion && Entry.Motion->Resolve(Rig, Entry.Binding);

        const Real64 Duration = Entry.Motion ? Entry.Motion->GetDuration() : 0.0;

        // Move the clock to the start time first, so playback is anchored there and not at the last advance.
        Entry.Cursor.Advance(Entry.Timestamp);
        Entry.Cursor.SetDuration(Duration);
        Entry.Cursor.SetRepeat(Clip ? Clip->Mode : Repeat::Loop);
        Entry.Cursor.SetSpeed(mRate * Entry.Rate);
        Entry.Cursor.Seek(Entry.Phase * Duration);
        Entry.Cursor.Play();

        Entry.Bound = true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Evaluate(ConstRef<Source> Entry, ConstRef<Skeleton3D> Rig, ConstRef<Skeleton3D::Pose> Output) const
    {
        // Seeded first, so a bone the clip never names rests where the hierarchy puts it.
        if (!Entry.Covered)
        {
            Rig.Rest(Output);
        }
        Entry.Motion->Sample(Entry.Cursor.GetTime(), Entry.Binding, Output);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Animator3D::Retire(Ref<Source> Entry)
    {
        mFade = 0.0;

        Entry.Name    = 0;
        Entry.Slot    = kUnresolved;
        Entry.Motion  = Retainer<Animation3D>();
        Entry.Binding.Clear();
        Entry.Bound   = false;
        Entry.Covered = false;
    }
}