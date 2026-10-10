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

namespace ZyAudio
{
    /// \brief Represents the high-level audio service that owns the mixer and its output device driver.
    class ZY_API Service final : public ZyEngine::Subsystem
    {
    public:

        /// \brief Callback invoked when playback ends, carrying the reason it did.
        using Callback = Delegate<void(Object, Reason)>;

    public:

        /// \brief Constructs the audio service.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Advances the audio driver state and processes pending audio operations.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Opens the given output device and registers the audio loaders.
        ///
        /// \param Device The name of the audio output device to open, or an empty string to use the system default.
        /// \return `true` if the device was successfully opened, `false` otherwise.
        Bool Initialize(Text Device);

        /// \brief Queries the driver and fills the provided structure with the backend, adapter, and available endpoints.
        ///
        /// \param Output Receives the device description.
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
        void SetListenerPose(ConstRef<Matrix4x3> Transform);

        /// \brief Sets the listener's directional cone parameters.
        ///
        /// \param InnerAngle The width of the cone heard at full gain.
        /// \param OuterAngle The width of the cone past which only `OuterGain` is heard.
        /// \param OuterGain  The gain applied outside the outer cone (0.0 = silent, 1.0 = full volume).
        void SetListenerCone(Angle InnerAngle, Angle OuterAngle, Real32 OuterGain);

        /// \brief Plays a sound, as a spatial source when an emitter and a transform are given.
        ///
        /// \param Category  The audio category for the playback.
        /// \param Sound     The audio sound to play; the service retains it for the playback's lifetime.
        /// \param Volume    The playback volume (0.0 = silent, 1.0 = full volume).
        /// \param Emitter   The spatial configuration of the source, or `nullptr` for a non-spatial playback.
        /// \param Transform The initial world transform of the source, or `nullptr` for a non-spatial playback.
        /// \return A handle to the playback instance, or `0` if the operation failed.
        Object Play(
            Category             Category,
            ConstRetainer<Sound> Sound,
            Real32               Volume,
            ConstPtr<Emitter>    Emitter   = nullptr,
            ConstPtr<Matrix4x3>  Transform = nullptr);

        /// \brief Sets whether a specific playback instance should loop.
        ///
        /// \param Handle  The handle of the playback to change.
        /// \param Looping `true` to enable looping, `false` to disable.
        void SetPlaybackLooping(Object Handle, Bool Looping);

        /// \brief Sets the volume for a specific playback instance.
        ///
        /// \param Handle The handle of the playback to change.
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        void SetPlaybackVolume(Object Handle, Real32 Volume);

        /// \brief Sets the world transform for a specific spatial playback instance.
        ///
        /// \param Handle    The handle of the spatial playback to move.
        /// \param Transform The new world transform of the audio source in 3D space.
        void SetPlaybackPose(Object Handle, ConstRef<Matrix4x3> Transform);

        /// \brief Sets the low-pass cutoff for a specific playback instance.
        ///
        /// \param Handle The handle of the playback to change.
        /// \param Cutoff The cutoff frequency in hertz, or `0` to leave the source unfiltered.
        void SetPlaybackCutoff(Object Handle, Real32 Cutoff);

        /// \brief Subscribes to end-of-playback notifications for a playback handle.
        ///
        /// \note A superseded playback reports before \ref Play returns, since the slot changes hands there.
        ///
        /// \param Handle   The playback handle to monitor.
        /// \param Callback The callback to invoke when the playback ends.
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

        /// \brief Stops every playback of the given sound.
        ///
        /// \param Asset The sound to silence every playback of.
        void Silence(ConstRef<Sound> Asset);

    private:

        /// \brief Reserves a playback slot, taking the least important voice's when the pool is full.
        ///
        /// \param Category The submix category the newcomer is routed through.
        /// \param Gain     The gain the newcomer would reach the listener at.
        /// \return The reserved handle, or `0` when nothing playing ranks below the newcomer.
        Object Reserve(Category Category, Real32 Gain);

        /// \brief Registers built-in resource loaders for audio resources.
        void RegisterBuiltinLoaders();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mixer                          mMixer;
        Spatializer                    mListener;
        Sequence<Completion>           mNotifications;
        Table<Object, Callback>        mSubscriptions;
        Table<Object, Retainer<Sound>> mResources;
        Freelist<kMaxInstances>        mInstances;
        Driver                         mDriver;
    };
}