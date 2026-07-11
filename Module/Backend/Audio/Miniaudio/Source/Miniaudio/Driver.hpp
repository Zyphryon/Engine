// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Source.hpp"
#include "Stream.hpp"
#include "Zyphryon.Audio/Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Miniaudio
{
    /// \brief A miniaudio driver implementation.
    class Driver final : public Audio::Driver
    {
    public:

        /// \brief Constructs the miniaudio driver.
        Driver();

        /// \brief Destroys the miniaudio driver.
        ~Driver() override;

        /// \see Driver::Initialize(Device)
        Bool Initialize(Text Device) override;

        /// \see Driver::Probe(Ref<Description>)
        void Probe(Ref<Description> Output) const override;

        /// \see Driver::Tick()
        void Tick(Real64 Delta) override;

        /// \see Driver::Drain(Ref<Completion>)
        void Drain(Ref<Completion> Completion) override;

        /// \see Driver::Suspend()
        void Suspend() override;

        /// \see Driver::Restore()
        void Restore() override;

        /// \see Driver::SetMasterVolume(Real32)
        void SetMasterVolume(Real32 Volume) override;

        /// \see Driver::GetMasterVolume()
        Real32 GetMasterVolume() const override;

        /// \see Driver::SetSubmixVolume(Category, Real32)
        void SetSubmixVolume(Category Category, Real32 Volume) override;

        /// \see Driver::GetSubmixVolume(Category)
        Real32 GetSubmixVolume(Category Category) const override;

        /// \see Driver::SetListenerPose(ConstRef<Pose>)
        void SetListenerPose(ConstRef<Pose> Pose) override;

        /// \see Driver::SetListenerCone(Angle, Angle, Real32)
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain) override;

        /// \see Driver::Play(Object, Category, Unique<Decoder>, Real32, Real32)
        Bool Play(Object Handle, Category Category, Unique<Decoder> Decoder, Real32 Volume, Real32 Pitch) override;

        /// \see Driver::Play(Object, Category, Unique<Decoder>, Real32, Real32, ConstRef<Emitter>, ConstRef<Pose>)
        Bool Play(Object Handle, Category Category, Unique<Decoder> Decoder, Real32 Volume, Real32 Pitch, ConstRef<Emitter> Emitter, ConstRef<Pose> Pose) override;

        /// \see Driver::SetPlaybackLooping(Object, Bool)
        void SetPlaybackLooping(Object Handle, Bool Looping) override;

        /// \see Driver::SetPlaybackPitch(Object, Real32)
        void SetPlaybackPitch(Object Handle, Real32 Pitch) override;

        /// \see Driver::SetPlaybackVolume(Object, Real32)
        void SetPlaybackVolume(Object Handle, Real32 Volume) override;

        /// \see Driver::SetPlaybackPose(Object, ConstRef<Pose>)
        void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose) override;

        /// \see Driver::Stop(Object)
        void Stop(Object Handle) override;

        /// \see Driver::Pause(Object)
        void Pause(Object Handle) override;

        /// \see Driver::Resume(Object)
        void Resume(Object Handle) override;

    private:

        /// \brief A collection of submixes for each audio category.
        using Submixes  = Array<ma_sound_group, Enum::Count<Category>()>;

        /// \brief Loads the audio backend capabilities.
        void LoadCapabilities();

        ma_device_id FindDevice(Text Device);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ma_context                   mContext;
        ma_engine                    mEngine;
        Description                  mDescription;
        Submixes                     mSubmixes;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Stream, kMaxInstances> mStreams;
        Array<Source, kMaxInstances> mSources;
        Sequence<Object>             mPlaybacks;
    };
}