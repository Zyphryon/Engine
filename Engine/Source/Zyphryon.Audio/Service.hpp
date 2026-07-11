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
#include "Track.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief High-level audio service that manages the driver lifecycle and exposes the audio playback API.
    class Service final : public Engine::Subsystem, public Switchable<Driver>
    {
    public:

        /// \brief Callback invoked when playback completes.
        using Callback = Delegate<void(Object)>;

    public:

        /// \brief Constructs the audio service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Advances the audio driver state and processes pending audio operations.
        ///
        /// \param Delta The elapsed time since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Initializes the audio service by selecting a registered adapter and opening the specified device.
        ///
        /// The driver must have been previously registered via a plugin before calling this method.
        ///
        /// \param Adapter The name of the registered audio adapter to use.
        /// \param Device  The name of the audio output device to open, or an empty string to use the system default.
        /// \return `true` if the adapter was found and the device was successfully opened, `false` otherwise.
        Bool Initialize(Text Adapter, Text Device);

        /// \brief Queries the driver and fills the provided structure with the backend, adapter, and available endpoints.
        ///
        /// \param Output The structure to populate with the driver's current device information.
        void Probe(Ref<Description> Output) const;

        /// \brief Suspends all audio processing.
        void Suspend();

        /// \brief Restores audio processing after a suspension.
        void Restore();

        /// \brief Sets the master volume applied to all audio output.
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

        /// \brief Plays a track as a non-spatial sound with the specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstRef<Track> Track, Real32 Volume, Real32 Pitch);

        /// \brief Plays a track as a spatial sound with the specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Track    The audio track to play.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Pitch    The playback pitch (1.0 = normal pitch).
        /// \param Emitter  The spatial configuration for the audio source.
        /// \param Pose     The initial pose of the audio source in 3D space.
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstRef<Track> Track, Real32 Volume, Real32 Pitch, ConstRef<Emitter> Emitter, ConstRef<Pose> Pose);

        /// \brief Sets whether a specific playback instance should loop.
        ///
        /// \param Handle  The handle of the playback instance.
        /// \param Looping `true` to enable looping, `false` to disable.
        void SetPlaybackLooping(Object Handle, Bool Looping);

        /// \brief Sets the pitch for a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pitch  The new pitch value (1.0 = normal pitch).
        void SetPlaybackPitch(Object Handle, Real32 Pitch);

        /// \brief Sets the volume for a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        void SetPlaybackVolume(Object Handle, Real32 Volume);

        /// \brief Sets the pose for a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Pose   The new pose of the audio source in 3D space.
        void SetPlaybackPose(Object Handle, ConstRef<Pose> Pose);

        /// \brief Subscribes to completion notifications for a playback handle.
        ///
        /// \param Handle   The playback handle to monitor.
        /// \param Callback The callback to invoke when the playback completes.
        void Subscribe(Object Handle, Callback Callback);

        /// \brief Unsubscribes from completion notifications for a playback handle.
        ///
        /// \param Handle The playback handle to stop monitoring.
        void Unsubscribe(Object Handle);

        /// \brief Stops a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance to stop.
        void Stop(Object Handle);

        /// \brief Pauses a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance to pause.
        void Pause(Object Handle);

        /// \brief Resumes a paused playback instance.
        ///
        /// \param Handle The handle of the playback instance to resume.
        void Resume(Object Handle);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Driver>          mDriver;
        Driver::Completion      mNotifications;
        Table<Object, Callback> mSubscriptions;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Freelist<kMaxInstances> mInstances;
    };
}