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
#include "Mixer.hpp"
#include "Sound.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief High-level audio service that owns the mixer and its output device driver.
    class Service final : public Engine::Subsystem
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

        /// \brief Initializes the audio service by creating the platform driver and opening the specified device.
        ///
        /// \param Device The name of the audio output device to open, or an empty string to use the system default.
        /// \return `true` if the device was successfully opened, `false` otherwise.
        Bool Initialize(Text Device);

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

        /// \brief Sets the listener's world transform in 3D space.
        ///
        /// \param Transform The new world transform of the listener.
        void SetListenerPose(ConstRef<Matrix4x4> Transform);

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The inner angle of the cone in degrees.
        /// \param OuterAngle The outer angle of the cone in degrees.
        /// \param OuterGain  The gain applied outside the outer cone (0.0 = silent, 1.0 = full volume).
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain);

        /// \brief Plays a sound as a non-spatial source with the specified parameters.
        ///
        /// \param Category The audio category for the playback.
        /// \param Sound    The audio sound to play; the service retains it for the playback's lifetime.
        /// \param Volume   The playback volume (0.0 = silent, 1.0 = full volume).
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstRetainer<Sound> Sound, Real32 Volume);

        /// \brief Plays a sound as a spatial source with the specified parameters.
        ///
        /// \param Category  The audio category for the playback.
        /// \param Sound     The audio sound to play; the service retains it for the playback's lifetime.
        /// \param Volume    The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Emitter   The spatial configuration for the audio source.
        /// \param Transform The initial world transform of the audio source in 3D space.
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(Category Category, ConstRetainer<Sound> Sound, Real32 Volume, ConstRef<Emitter> Emitter, ConstRef<Matrix4x4> Transform);

        /// \brief Sets whether a specific playback instance should loop.
        ///
        /// \param Handle  The handle of the playback instance.
        /// \param Looping `true` to enable looping, `false` to disable.
        void SetPlaybackLooping(Object Handle, Bool Looping);

        /// \brief Sets the volume for a specific playback instance.
        ///
        /// \param Handle The handle of the playback instance.
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        void SetPlaybackVolume(Object Handle, Real32 Volume);

        /// \brief Sets the world transform for a specific spatial playback instance.
        ///
        /// \param Handle    The handle of the playback instance.
        /// \param Transform The new world transform of the audio source in 3D space.
        void SetPlaybackPose(Object Handle, ConstRef<Matrix4x4> Transform);

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

        Driver                         mDriver;
        Mixer                          mMixer;
        Sequence<Object>               mNotifications;
        Table<Object, Callback>        mSubscriptions;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<Object, Retainer<Sound>> mResources;
        Freelist<kMaxInstances>        mInstances;
    };
}
