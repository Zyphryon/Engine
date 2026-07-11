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

#include "Decoder.hpp"
#include "Emitter.hpp"
#include "Pose.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Abstract interface for low-level audio driver backends.
    class Driver
    {
    public:

        /// \brief A sequence of playback handles that have completed.
        using Completion = Sequence<Object>;

    public:

        /// \brief Ensures derived class can be destroyed polymorphically.
        virtual ~Driver() = default;

        /// \brief Initializes the audio driver with the specified device.
        ///
        /// \param Device The identifier of the audio device to use.
        /// \return `true` if initialization was successful, otherwise `false`.
        virtual Bool Initialize(Text Device)
        {
            return true;
        }

        /// \brief Queries the driver and fills the provided structure with the backend, adapter, and available endpoints.
        ///
        /// \param Output The structure to populate with the driver's current device information.
        virtual void Probe(Ref<Description> Output) const
        {

        }

        /// \brief Advances the audio driver's internal state and processes audio.
        ///
        /// \param Delta The time elapsed since the last tick, in seconds.
        virtual void Tick(Real64 Delta)
        {

        }

        /// \brief Gets all playback operations that completed during the current tick.
        ///
        /// \param Output The sequence that receives the completed playback handles.
        virtual void Drain(Ref<Completion> Output)
        {

        }

        /// \brief Suspends audio processing.
        virtual void Suspend()
        {

        }

        /// \brief Restores audio processing after being suspended.
        virtual void Restore()
        {

        }

        /// \brief Sets the master volume for all audio output.
        ///
        /// \param Volume The master volume level (0.0 = silent, 1.0 = full volume).
        virtual void SetMasterVolume(Real32 Volume)
        {

        }

        /// \brief Gets the current master volume level.
        ///
        /// \return The master volume level (0.0 = silent, 1.0 = full volume).
        virtual Real32 GetMasterVolume() const
        {
            return 0.0f;
        }

        /// \brief Sets the volume for a specific audio category.
        ///
        /// \param Category The audio category to adjust.
        /// \param Volume   The volume level for the category (0.0 = silent, 1.0 = full volume).
        virtual void SetSubmixVolume(Category Category, Real32 Volume)
        {

        }

        /// \brief Gets the current volume level for a specific audio category.
        ///
        /// \param Category The audio category to query.
        /// \return The volume level for the category (0.0 = silent, 1.0 = full volume).
        virtual Real32 GetSubmixVolume(Category Category) const
        {
            return 0.0f;
        }

        /// \brief Sets the listener's pose in 3D space.
        ///
        /// \param Pose The new pose of the listener.
        virtual void SetListenerPose(ConstRef<Pose> Pose)
        {

        }

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The inner angle of the cone in degrees.
        /// \param OuterAngle The outer angle of the cone in degrees.
        /// \param OuterGain  The gain applied outside the outer cone (0.0 = silent, 1.0 = full volume).
        virtual void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain)
        {

        }

        /// \brief Plays audio as a non-spatial sound with the specified parameters.
        ///
        /// \param Handle   The handle of the playback instance.
        /// \param Category The audio category for the playback.
        /// \param Decoder  The decoder providing PCM frames for playback.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \return `true` if playback started successfully, otherwise `false`.
        virtual Bool Play(Object Handle, Category Category, Unique<Decoder> Decoder, Real32 Volume, Real32 Pitch)
        {
            return false;
        }

        /// \brief Plays audio as a spatial sound with the specified parameters.
        ///
        /// \param Handle   The handle of the playback instance.
        /// \param Category The audio category for the playback.
        /// \param Decoder  The decoder providing PCM frames for playback.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \param Emitter  The spatial configuration for the audio source.
        /// \param Pose     The initial pose of the audio source in 3D space.
        /// \return `true` if playback started successfully, otherwise `false`.
        virtual Bool Play(Object Handle, Category Category, Unique<Decoder> Decoder, Real32 Volume, Real32 Pitch, ConstRef<Emitter> Emitter, ConstRef<Pose> Pose)
        {
            return false;
        }

        /// \brief Sets whether a specific audio playback instance should loop.
        ///
        /// \param Handle  The handle of the playback instance.
        /// \param Looping `true` to enable looping, `false` to disable.
        virtual void SetPlaybackLooping(Object Handle, Bool Looping)
        {

        }

        /// \brief Sets the pitch for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pitch  The new pitch value (1.0 = normal pitch).
        virtual void SetPlaybackPitch(Object Handle, Real32 Pitch)
        {

        }

        /// \brief Sets the volume for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        virtual void SetPlaybackVolume(Object Handle, Real32 Volume)
        {

        }

        /// \brief Sets the pose for a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pose   The new pose of the playback instance.
        virtual void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose)
        {

        }

        /// \brief Stops a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to stop.
        virtual void Stop(Object Handle)
        {

        }

        /// \brief Pauses a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to pause.
        virtual void Pause(Object Handle)
        {

        }

        /// \brief Resumes a specific audio playback instance.
        ///
        /// \param Handle The handle of the playback instance to resume.
        virtual void Resume(Object Handle)
        {

        }
    };
}