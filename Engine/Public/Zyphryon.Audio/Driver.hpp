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

#include "Emitter.hpp"
#include "Pose.hpp"
#include "Track.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Abstract interface for low-level audio driver backends.
    class Driver
    {
    public:

        /// \brief Ensures derived class can be destroyed polymorphically.
        virtual ~Driver() = default;

        /// \brief Initializes the audio driver with the specified device.
        ///
        /// \param Device The identifier of the audio device to use.
        /// \return `true` if initialization was successful, otherwise `false`.
        virtual Bool Initialize(ConstStr8 Device) = 0;

        /// \brief Retrieves the capabilities of the audio driver.
        ///
        /// \return The driver's capabilities.
        virtual ConstRef<Capabilities> GetCapabilities() const = 0;

        /// \brief Advances the audio driver's internal state and processes audio.
        virtual void Tick() = 0;

        /// \brief Suspends audio processing.
        virtual void Suspend() = 0;

        /// \brief Restores audio processing after being suspended.
        virtual void Restore() = 0;

        /// \brief Sets the master volume for all audio output.
        ///
        /// \param Volume The master volume level (0.0 = silent, 1.0 = full volume).
        virtual void SetMasterVolume(Real32 Volume) = 0;

        /// \brief Gets the current master volume level.
        ///
        /// \return The master volume level (0.0 = silent, 1.0 = full volume).
        virtual Real32 GetMasterVolume() = 0;

        /// \brief Sets the volume for a specific audio category.
        ///
        /// \param Category The audio category to adjust.
        /// \param Volume   The volume level for the category (0.0 = silent, 1.0 = full volume).
        virtual void SetSubmixVolume(Category Category, Real32 Volume) = 0;

        /// \brief Gets the current volume level for a specific audio category.
        ///
        /// \param Category The audio category to query.
        /// \return The volume level for the category (0.0 = silent, 1.0 = full volume).
        virtual Real32 GetSubmixVolume(Category Category) = 0;

        /// \brief Sets the listener's pose in 3D space.
        ///
        /// \param Pose The new pose of the listener.
        virtual void SetListenerPose(ConstRef<Pose> Pose) = 0;

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The inner angle of the cone in degrees.
        /// \param OuterAngle The outer angle of the cone in degrees.
        /// \param OuterGain  The gain applied outside the outer cone (0.0 = silent, 1.0 = full volume).
        virtual void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain) = 0;

        /// \brief Plays an audio track with specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \return A handle to the playback instance.
        virtual Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch) = 0;

        /// \brief Plays a spatial audio track with specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \param Emitter  The audio emitter defining spatial properties.
        /// \param Pose     The pose of the audio source in 3D space.
        /// \return A handle to the playback instance.
        virtual Object Play(Category Category, ConstTracker<Track> Track, Real32 Volume, Real32 Pitch, ConstTracker<Emitter> Emitter, ConstRef<Pose> Pose) = 0;

        /// \brief Sets whether a specific audio playback instance should loop.
        ///
        /// \param Handle  The handle of the playback instance.
        /// \param Looping `true` to enable looping, `false` to disable.
        virtual void SetPlaybackLooping(Object Handle, Bool Looping) = 0;

        /// \brief Sets the pitch for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance
        /// \param Pitch  The new pitch value (1.0 = normal pitch).
        virtual void SetPlaybackPitch(Object Handle, Real32 Pitch) = 0;

        /// \brief Sets the volume for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        virtual void SetPlaybackVolume(Object Handle, Real32 Volume) = 0;

        /// \brief Sets the pose for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pose   The new pose of the playback instance.
        virtual void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose) = 0;

        /// \brief Stops a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to stop.
        virtual void Stop(Object Handle) = 0;

        /// \brief Pauses a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to pause.
        virtual void Pause(Object Handle) = 0;

        /// \brief Resumes a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to resume.
        virtual void Resume(Object Handle) = 0;
    };
}