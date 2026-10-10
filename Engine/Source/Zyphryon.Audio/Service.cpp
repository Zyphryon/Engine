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

#include "Service.hpp"
#include "Loader/SNDLoader.hpp"
#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAudio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Service::Service(Ref<Host> Host)
        : Subsystem { Host }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::OnTick(Real64 Delta)
    {
        ZY_PROFILE_SCOPE("Audio::Tick");

        // Only a driver without a thread of its own does its work here.
        mDriver.Advance(Delta);

        mMixer.Drain(mNotifications);

        // Releases each ended playback's sound and tells its subscriber.
        for (ConstRef<Completion> Completion : mNotifications)
        {
            // A superseded slot was accounted for the moment it changed hands, so the mixer's word on it is stale.
            if (!mInstances.IsAllocated(Completion.Handle))
            {
                continue;
            }

            mInstances.Free(Completion.Handle);
            mResources.Erase(Completion.Handle);

            if (Callback Delegate; mSubscriptions.Extract(Completion.Handle, Delegate))
            {
                Delegate(Completion.Handle, Completion.Reason);
            }
        }
        mNotifications.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Service::Initialize(Text Device)
    {
        ZY_PROFILE_SCOPE("Audio::Initialize");

        Driver::Callback Callback = [this](Span<Real32> Output, UInt32 Frames)
        {
            mMixer.Render(Output, Frames);
        };

        if (!mDriver.Open(Device, Move(Callback)))
        {
            LOG_E("Audio: Failed to open output device '{0}'", Device);
            return false;
        }

        Description Description;
        mDriver.Probe(Description);

        LOG_I("Audio: Backend '{0}' on device '{1}'", Description.Backend, Description.Adapter);

        for (Text Endpoint : Description.Endpoints)
        {
            LOG_I("Audio: Found endpoint '{0}'", Endpoint);
        }

        RegisterBuiltinLoaders();
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Probe(Ref<Description> Output) const
    {
        mDriver.Probe(Output);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Suspend()
    {
        mDriver.Suspend();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Restore()
    {
        mDriver.Restore();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetMasterVolume(Real32 Volume)
    {
        ZY_ASSERT(IsBetween(Volume, 0.0f, 1.0f), "Volume must be between 0.0 and 1.0");

        mMixer.SetMasterVolume(Volume);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Service::GetMasterVolume() const
    {
        return mMixer.GetMasterVolume();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetSubmixVolume(Category Category, Real32 Volume)
    {
        ZY_ASSERT(IsBetween(Volume, 0.0f, 1.0f), "Volume must be between 0.0 and 1.0");

        mMixer.SetSubmixVolume(Category, Volume);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Service::GetSubmixVolume(Category Category) const
    {
        return mMixer.GetSubmixVolume(Category);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetListenerPose(ConstRef<Matrix4x3> Transform)
    {
        // The service keeps its own listener so a newcomer can be placed without reading the audio thread's copy.
        mListener.SetListener(
            Transform.GetTranslation(),
            Vector3::Normalize(Transform.GetForward()),
            Vector3::Normalize(Transform.GetRight()));

        mMixer.SetListener(Transform);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain)
    {
        ZY_ASSERT(InnerAngle.IsValid(), "Inner angle must be normalized (0 <= angle < 2π)");
        ZY_ASSERT(OuterAngle.IsValid(), "Outer angle must be normalized (0 <= angle < 2π)");
        ZY_ASSERT(InnerAngle <= OuterAngle, "Inner angle cannot exceed outer angle");
        ZY_ASSERT(IsBetween(OuterGain, 0.0f, 1.0f), "Outer gain must be [0,1]");

        mListener.SetListenerCone(InnerAngle, OuterAngle, OuterGain);

        mMixer.SetListenerCone(InnerAngle, OuterAngle, OuterGain);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::Play(
        Category             Category,
        ConstRetainer<Sound> Sound,
        Real32               Volume,
        ConstPtr<Emitter>    Emitter,
        ConstPtr<Matrix4x3>  Transform)
    {
        ZY_ASSERT(Sound->HasCompleted(), "Sound must be valid and loaded");
        ZY_ASSERT(Sound->GetFrequency() == kMixerFrequency, "Sound must be baked to the mixer sample rate");
        ZY_ASSERT(IsBetween(Volume, 0.0f, 1.0f), "Volume must be between 0.0 and 1.0");
        ZY_ASSERT(!Emitter == !Transform, "A spatial playback needs both an emitter and a transform");

        Unique<Decoder> Decoder = Sound->Decode();

        if (!Decoder)
        {
            return Object();
        }

        Real32 Gain = Volume * mMixer.GetSubmixVolume(Category);

        if (Emitter)
        {
            const Vector2 Placement = mListener.Compute(
                Transform->GetTranslation(),
                Vector3::Normalize(Transform->GetForward()),
                * Emitter);

            Gain *= Max(Placement.GetX(), Placement.GetY());
        }

        const Object Playback = Reserve(Category, Gain);

        if (!Playback)
        {
            return Object();
        }

        if (!mMixer.Play(Playback, Category, Decoder.Grab(), Sound->GetStride(), Volume, Emitter, Transform))
        {
            mInstances.Free(Playback);
            return Object();
        }

        Decoder.Release();
        mResources.Assign(Playback, Sound);
        return Playback;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetPlaybackLooping(Object Handle, Bool Looping)
    {
        mMixer.SetLooping(Handle, Looping);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetPlaybackVolume(Object Handle, Real32 Volume)
    {
        mMixer.SetVolume(Handle, Volume);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetPlaybackPose(Object Handle, ConstRef<Matrix4x3> Transform)
    {
        mMixer.SetTransform(Handle, Transform);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::SetPlaybackCutoff(Object Handle, Real32 Cutoff)
    {
        ZY_ASSERT(Cutoff >= 0.0f, "Cutoff must be non-negative");

        mMixer.SetCutoff(Handle, Cutoff);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Subscribe(Object Handle, Callback Callback)
    {
        ZY_ASSERT(Handle.IsValid(), "Handle must be valid");

        mSubscriptions.Assign(Handle, Move(Callback));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Unsubscribe(Object Handle)
    {
        ZY_ASSERT(Handle.IsValid(), "Handle must be valid");

        mSubscriptions.Erase(Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Stop(Object Handle)
    {
        mMixer.Stop(Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Pause(Object Handle)
    {
        mMixer.Pause(Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Resume(Object Handle)
    {
        mMixer.Resume(Handle);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::Silence(ConstRef<Sound> Asset)
    {
        mResources.ForEach([&](Object Playback, ConstRetainer<Sound> Resource)
        {
            if (AddressOf(* Resource) == AddressOf(Asset))
            {
                mMixer.Stop(Playback);
            }
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Object Service::Reserve(Category Category, Real32 Gain)
    {
        if (const Object Handle = mInstances.Allocate())
        {
            return Handle;
        }

        // With every slot taken the newcomer outranks one of them or it does not play, so arrival order stops deciding.
        Real32 Weakest = Mixer::Rank(Category, Gain);
        Object Victim;

        for (Object::Slot Slot = 1; Slot <= mInstances.GetTop(); ++Slot)
        {
            if (!mInstances.IsOccupied(Slot))
            {
                continue;
            }

            if (const Real32 Rank = mMixer.GetRank(Slot); Rank > 0.0f && Rank < Weakest)
            {
                Weakest = Rank;
                Victim  = mInstances.GetKey(Slot);
            }
        }

        if (!Victim)
        {
            return Object();
        }

        mResources.Erase(Victim);
        mInstances.Free(Victim);

        // The slot is secured before the owner hears of it, so a callback that plays cannot take it back.
        const Object Handle = mInstances.Allocate();

        if (Callback Delegate; mSubscriptions.Extract(Victim, Delegate))
        {
            Delegate(Victim, Reason::Superseded);
        }
        return Handle;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Service::RegisterBuiltinLoaders()
    {
        ConstRetainer<ZyContent::Service> Content = GetHost().GetService<ZyContent::Service>();

        Content->AddLoader(SNDLoader::kTypes, Retainer<SNDLoader>::Create());
    }
}