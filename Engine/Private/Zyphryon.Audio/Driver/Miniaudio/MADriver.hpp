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

#include "MAVoice.hpp"
#include "Zyphryon.Audio/Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Encapsulate the audio driver implementation using 'Mini Audio'.
    class MADriver final : public Driver
    {
    public:

        /// \brief Constructs a new audio driver.
        MADriver();

        /// \brief Destroys the audio driver.
        ~MADriver() override;

        /// \copydoc Driver::Initialize(Device)
        Bool Initialize(ConstStr8 Device) override;

        /// \copydoc Driver::GetCapabilities()
        ConstRef<Capabilities> GetCapabilities() const override;

        /// \copydoc Driver::Tick()
        void Tick() override;

        /// \copydoc Driver::Suspend()
        void Suspend() override;

        /// \copydoc Driver::Restore()
        void Restore() override;

        /// \copydoc Driver::SetMasterVolume(Real32)
        void SetMasterVolume(Real32 Volume) override;

        /// \copydoc Driver::GetMasterVolume()
        Real32 GetMasterVolume() override;

        /// \copydoc Driver::SetSubmixVolume(Category, Real32)
        void SetSubmixVolume(Category Category, Real32 Volume) override;

        /// \copydoc Driver::GetSubmixVolume(Category)
        Real32 GetSubmixVolume(Category Category) override;

        /// \copydoc Driver::SetListenerPose(ConstRef<Pose>)
        void SetListenerPose(ConstRef<Pose> Pose) override;

        /// \copydoc Driver::SetListenerCone(Angle, Angle, Real32)
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain) override;

        /// \copydoc Driver::Play(Category, ConstTracker<Track>, Real32, Real32)
        Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch) override;

        /// \copydoc Driver::Play(Category, ConstTracker<Track>, Real32, Real32, ConstTracker<Emitter>, ConstRef<Pose>)
        Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch, ConstTracker<Emitter> Emitter, ConstRef<Pose> Pose) override;

        /// \copydoc Driver::SetPlaybackLooping(Object, Bool)
        void SetPlaybackLooping(Object Handle, Bool Looping) override;

        /// \copydoc Driver::SetPlaybackPitch(Object, Real32)
        void SetPlaybackPitch(Object Handle, Real32 Pitch) override;

        /// \copydoc Driver::SetPlaybackVolume(Object, Real32)
        void SetPlaybackVolume(Object Handle, Real32 Volume) override;

        /// \copydoc Driver::SetPlaybackPose(Object, ConstRef<Pose>)
        void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose) override;

        /// \copydoc Driver::Stop(Object)
        void Stop(Object Handle) override;

        /// \copydoc Driver::Pause(Object)
        void Pause(Object Handle) override;

        /// \copydoc Driver::Resume(Object)
        void Resume(Object Handle) override;

    private:

        /// \brief A collection of submixes for each audio category.
        using Submixes  = Array<ma_sound_group, Enum::Count<Category>()>;

        /// \brief A collection of available voices.
        struct Voices final
        {
            /// Mutex that protects access to the voice pool.
            Mutex                        Mutex;

            /// Pool that manages voice instances.
            Pool<MAVoice, kMaxInstances> Pool;

            /// List of available voice handles that needs to be reused.
            Vector<Object>               Availables;

            /// \brief Ticks the voice pool, freeing any available voices.
            ZYPHRYON_INLINE void Tick()
            {
                Guard Lock(Mutex);

                for (const UInt32 Handle : Availables)
                {
                    Pool.Free(Handle);
                }
                Availables.clear();
            }

            /// \brief Allocates a new voice from the pool.
            ///
            /// \return The handle of the allocated voice.
            ZYPHRYON_INLINE Object Allocate()
            {
                Guard Lock(Mutex);
                return Pool.AllocateWithID();
            }

            /// \brief Fetches a voice from the pool using its handle.
            ///
            /// \param Handle The handle of the voice to fetch.
            /// \return The reference to the voice instance.
            ZYPHRYON_INLINE Ref<MAVoice> Fetch(Object Handle)
            {
                return Pool[Handle];
            }

            /// \brief Releases a voice back to the pool for later reuse.
            ///
            /// \param Handle The handle of the voice to release.
            ZYPHRYON_INLINE void Release(Object Handle)
            {
                Guard Lock(Mutex);
                Availables.push_back(Handle);
            }
        };

        /// \brief Loads the audio backend capabilities.
        void LoadCapabilities();

        /// \brief Handle when a voice finishes playback.
        void OnVoiceFinish(Ref<MAVoice> Voice);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ma_context   mContext;
        ma_engine    mEngine;
        Capabilities mCapabilities;
        Submixes     mSubmixes;
        Voices       mVoices;
    };
}