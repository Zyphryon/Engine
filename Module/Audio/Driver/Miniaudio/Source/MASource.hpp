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

#include "Zyphryon.Audio/Emitter.hpp"
#include "Zyphryon.Audio/Pose.hpp"
#include <miniaudio.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Wraps a miniaudio sound instance with lifecycle and spatial audio management.
    class MASource final
    {
    public:

        /// \brief Constructs an uninitialized source holding no sound.
        ZY_INLINE MASource()
            : mActive { false }
        {
        }

        /// \brief Destroys the source, releasing any active resources.
        ZY_INLINE ~MASource()
        {
            Reset();
        }

        /// \brief Checks whether the source has reached the end of its audio data.
        ///
        /// \return `true` if source has finished, `false` otherwise.
        ZY_INLINE Bool HasFinished() const
        {
            return ma_sound_at_end(AddressOf(mObject));
        }

        /// \brief Initializes the source from a data source, routing output through a submix group.
        ///
        /// \param Engine       The engine instance that manages the audio graph.
        /// \param Submix       The sound group this source will be routed through.
        /// \param Stream       The data source providing decoded audio frames.
        /// \param Spatialized  Whether to enable 3D spatialization for this source.
        /// \return `true` if initialization succeeded, `false` otherwise.
        ZY_INLINE Bool Setup(Ptr<ma_engine> Engine, Ptr<ma_sound_group> Submix, Ptr<ma_data_source> Stream, Bool Spatialized)
        {
            const ma_result Result = ma_sound_init_from_data_source(
                Engine,
                Stream,
                Spatialized ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION,
                Submix,
                AddressOf(mObject));
            mActive = (Result == MA_SUCCESS);

            return mActive;
        }

        /// \brief Resets the source, releasing any active Miniaudio resources.
        ZY_INLINE void Reset()
        {
            if (mActive)
            {
                ma_sound_uninit(AddressOf(mObject));

                mActive = false;
            }
        }

        /// \brief Begins or resumes playback from the current position.
        ZY_INLINE void Start()
        {
            ma_sound_start(AddressOf(mObject));
        }

        /// \brief Pauses playback without resetting the playback position.
        ZY_INLINE void Pause()
        {
            ma_sound_stop(AddressOf(mObject));
        }

        /// \brief Stops playback and marks the source as finished.
        ZY_INLINE void Stop()
        {
            ma_sound_stop(AddressOf(mObject));

            // Sets the internal atEnd flag since miniaudio does not expose a public API for this.
            AtomicRef(mObject.atEnd).store(MA_TRUE, std::memory_order_seq_cst);
        }

        /// \brief Sets whether the source loops when it reaches the end of its audio data.
        ///
        /// \param Looping `true` to enable looping, `false` to play once.
        ZY_INLINE void SetLooping(Bool Looping)
        {
            ma_sound_set_looping(AddressOf(mObject), Looping ? MA_TRUE : MA_FALSE);
        }

        /// \brief Sets the playback pitch multiplier.
        ///
        /// \param Pitch The pitch scale factor.
        ZY_INLINE void SetPitch(Real32 Pitch)
        {
            ma_sound_set_pitch(AddressOf(mObject), Pitch);
        }

        /// \brief Sets the playback volume.
        ///
        /// \param Volume The volume scale factor.
        ZY_INLINE void SetVolume(Real32 Volume)
        {
            ma_sound_set_volume(AddressOf(mObject), Volume);
        }

        /// \brief Sets the spatial position, velocity, and orientation of this source.
        ///
        /// \param Pose The pose describing the source's location and movement in world space.
        ZY_INLINE void SetPose(ConstRef<Pose> Pose)
        {
            const Vector3 Position = Pose.GetPosition();
            ma_sound_set_position(AddressOf(mObject), Position.GetX(), Position.GetY(), Position.GetZ());

            const Vector3 Velocity = Pose.GetVelocity();
            ma_sound_set_velocity(AddressOf(mObject), Velocity.GetX(), Velocity.GetY(), Velocity.GetZ());

            const Vector3 Forward = Pose.GetOrientation().GetForward();
            ma_sound_set_direction(AddressOf(mObject), Forward.GetX(), Forward.GetY(), Forward.GetZ());
        }

        /// \brief Configures the spatial attenuation and cone parameters from an emitter descriptor.
        ///
        /// \param Emitter The emitter properties.
        ZY_INLINE void SetEmitter(ConstRef<Emitter> Emitter)
        {
            ma_attenuation_model Attenuation = ma_attenuation_model_none;

            switch (Emitter.GetAttenuation())
            {
            case Attenuation::Linear:
                Attenuation = ma_attenuation_model_linear;
                break;
            case Attenuation::Inverse:
                Attenuation = ma_attenuation_model_inverse;
                break;
            case Attenuation::Exponential:
                Attenuation = ma_attenuation_model_exponential;
                break;
            }

            ma_sound_set_attenuation_model(AddressOf(mObject), Attenuation);
            ma_sound_set_min_distance(AddressOf(mObject), Emitter.GetInnerRadius());
            ma_sound_set_max_distance(AddressOf(mObject), Emitter.GetOuterRadius());
            ma_sound_set_cone(AddressOf(mObject),
                Emitter.GetInnerAngle().GetRadians(),
                Emitter.GetOuterAngle().GetRadians(),
                Emitter.GetOuterGain());
            ma_sound_set_doppler_factor(AddressOf(mObject), Emitter.GetDopplerFactor());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ma_bool32 mActive;
        ma_sound  mObject;
    };
}