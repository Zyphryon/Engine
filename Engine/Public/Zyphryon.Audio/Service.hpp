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

#include "Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Provides high-level management of the audio subsystem.
    class Service final : public AbstractService<Service>
    {
        // TODO: MulticastDelegates (OnDeviceConnected, OnDeviceDisconnected, OnSoundFinished)

    public:

        /// \brief Constructs the audio service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \copydoc Service::OnTick(Time)
        void OnTick(Time Time) override;

        /// \brief Initializes the audio service with the specified backend and device.
        ///
        /// \param Backend The audio backend to use.
        /// \param Device  The identifier of the audio device to use.
        Bool Initialize(Backend Backend, ConstStr8 Device);

        /// \brief Retrieves the capabilities of the audio driver.
        ///
        /// \return The driver's capabilities.
        ConstRef<Capabilities> GetCapabilities() const;

        /// \brief Suspends audio processing.
        void Suspend();

        /// \brief Restores audio processing after being suspended.
        void Restore();

        /// \brief Sets the master volume for all audio output.
        ///
        /// \param Volume The master volume level (0.0 = silent, 1.0 = full volume).
        void SetMasterVolume(Real32 Volume);

        /// \brief Gets the current master volume level.
        ///
        /// \return The master volume level (0.0 = silent, 1.0 = full volume).
        Real32 GetMasterVolume() const;

        /// \brief Sets the volume for a specific audio category.
        ///
        /// \param Category The audio category to adjust.
        /// \param Volume   The volume level for the category (0.0 = silent, 1.0 = full volume).
        void SetSubmixVolume(Category Category, Real32 Volume);

        /// \brief Gets the current volume level for a specific audio category.
        ///
        /// \param Category The audio category to query.
        /// \return The volume level for the category (0.0 = silent, 1.0 = full volume).
        Real32 GetSubmixVolume(Category Category) const;

        /// \brief Sets the listener's pose in 3D space.
        ///
        /// \param Pose The new pose of the listener.
        void SetListenerPose(ConstRef<Pose> Pose);

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The inner angle of the cone in degrees.
        /// \param OuterAngle The outer angle of the cone in degrees.
        /// \param OuterGain  The gain applied outside the outer cone (0.0 = silent, 1.0 = full volume).
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain);

        /// \brief Plays an audio track with specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch);

        /// \brief Plays a spatial audio track with specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \param Emitter  The audio emitter defining spatial properties.
        /// \param Pose     The pose of the audio source in 3D space.
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch, ConstTracker<Emitter> Emitter, ConstRef<Pose> Pose);

        /// \brief Sets whether a specific audio playback instance should loop.
        ///
        /// \param Handle  The handle of the playback instance.
        /// \param Looping `true` to enable looping, `false` to disable.
        void SetPlaybackLooping(Object Handle, Bool Looping);

        /// \brief Sets the pitch for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance
        /// \param Pitch  The new pitch value (1.0 = normal pitch).
        void SetPlaybackPitch(Object Handle, Real32 Pitch);

        /// \brief Sets the volume for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        void SetPlaybackVolume(Object Handle, Real32 Volume);

        /// \brief Sets the pose for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pose   The new pose of the playback instance.
        void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose);

        /// \brief Stops a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to stop.
        void Stop(Object Handle);

        /// \brief Pauses a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to pause.
        void Pause(Object Handle);

        /// \brief Resumes a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to resume.
        void Resume(Object Handle);
        
    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Driver> mDriver;
    };
}