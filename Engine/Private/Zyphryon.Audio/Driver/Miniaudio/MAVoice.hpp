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

#include "MASource.hpp"
#include "Zyphryon.Audio/Emitter.hpp"
#include "Zyphryon.Audio/Pose.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Voice implementation for Miniaudio audio driver.
    class MAVoice final
    {
    public:

        /// \brief Constructs a voice with the specified handle.
        ///
        /// \param Handle The unique object handle for this voice.
        ZYPHRYON_INLINE MAVoice(Object Handle)
            : mObject { },
              mHandle { Handle }
        {
        }
        
        /// \brief Destructor, cleans up the voice resources.
        ZYPHRYON_INLINE ~MAVoice()
        {
            ma_sound_uninit(&mObject);
        }

        /// \brief Gets the unique handle of this voice.
        ///
        /// \return The object handle.
        ZYPHRYON_INLINE Object GetHandle() const
        {
            return mHandle;
        }

        /// \brief Loads a track into the voice.
        ///
        /// \param Engine      The audio engine instance.
        /// \param Submix      The submix group to route the voice to.
        /// \param Track       The track to load into the voice.
        /// \param Spatialized Whether the voice should be spatialized.
        /// \return `true` if the track was loaded successfully, `false` otherwise.
        ZYPHRYON_INLINE Bool Load(Ref<ma_engine> Engine, Ref<ma_sound_group> Submix, ConstTracker<Track> Track, Bool Spatialized)
        {
            // Load the track into the source.
            mSource.Load(Track);

            // Initialize the sound object.
            const ma_uint32 Flags  = (!Spatialized ? MA_SOUND_FLAG_NO_SPATIALIZATION : 0);
            const ma_result Result = ma_sound_init_from_data_source(
                &Engine,
                &mSource,
                Flags,
                &Submix,
                &mObject);
            return (Result == MA_SUCCESS);
        }

        /// \brief Stops playback of the voice and invokes the end callback.
        ZYPHRYON_INLINE void Stop()
        {
            ma_sound_stop(&mObject);

            if (mObject.endCallback)    // NOTE: ma_sound_set_at_end is not exposed by the library
            {
                mObject.endCallback(mObject.pEndCallbackUserData, &mObject);
            }
        }

        /// \brief Pauses playback of the voice.
        ZYPHRYON_INLINE void Pause()
        {
            ma_sound_stop(&mObject);
        }

        /// \brief Resumes playback of the voice.
        ZYPHRYON_INLINE void Resume()
        {
            ma_sound_start(&mObject);
        }

        /// \brief Sets whether the voice should loop.
        ///
        /// \param Looping `true` to enable looping, `false` to disable.
        ZYPHRYON_INLINE void SetLooping(Bool Looping)
        {
            ma_sound_set_looping(&mObject, Looping ? MA_TRUE : MA_FALSE);
        }

        /// \brief Sets the pitch for the voice.
        ///
        /// \param Pitch The new pitch value (1.0 = normal pitch).
        ZYPHRYON_INLINE void SetPitch(Real32 Pitch)
        {
            ma_sound_set_pitch(&mObject, Pitch);
        }

        /// \brief Sets the volume for the voice.
        ///
        /// \param Volume The new volume value (0.0 = silent, 1.0 = full volume).
        ZYPHRYON_INLINE void SetVolume(Real32 Volume)
        {
            ma_sound_set_volume(&mObject, Volume);
        }

        /// \brief Sets the pose for the voice.
        ///
        /// \param Pose The new pose of the voice.
        ZYPHRYON_INLINE void SetPose(ConstRef<Pose> Pose)
        {
            const Vector3 Position = Pose.GetPosition();
            ma_sound_set_position(&mObject, Position.GetX(), Position.GetY(), Position.GetZ());

            const Vector3 Velocity = Pose.GetVelocity();
            ma_sound_set_velocity(&mObject, Velocity.GetX(), Velocity.GetY(), Velocity.GetZ());

            const Vector3 Forward = Pose.GetOrientation().GetForward();
            ma_sound_set_direction(&mObject, Forward.GetX(), Forward.GetY(), Forward.GetZ());
        }

        /// \brief Sets the emitter properties for spatialization.
        ///
        /// \param Emitter The emitter defining spatial properties.
        ZYPHRYON_INLINE void SetEmitter(ConstTracker<Emitter> Emitter)
        {
            ma_attenuation_model Attenuation = ma_attenuation_model_none;

            switch (Emitter->GetAttenuation())
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

            ma_sound_set_attenuation_model(&mObject, Attenuation);
            ma_sound_set_min_distance(&mObject, Emitter->GetInnerRadius());
            ma_sound_set_max_distance(&mObject, Emitter->GetOuterRadius());
            ma_sound_set_cone(&mObject,
                Emitter->GetInnerAngle().GetRadians(),
                Emitter->GetOuterAngle().GetRadians(),
                Emitter->GetOuterGain());
            ma_sound_set_doppler_factor(&mObject, Emitter->GetDopplerFactor());
        }

        /// \brief Sets a callback to be invoked when the voice finishes playback.
        ///
        /// \tparam Method   The member function to be called on completion.
        /// \param  Instance The instance of the object to call the method on.
        template<typename Type, void (Type::*Method)(Ref<MAVoice>)>
        ZYPHRYON_INLINE void SetCallback(Ptr<Type> Instance)
        {
            static constexpr auto Callback = [](Ptr<void> Context, Ptr<ma_sound> Object)
            {
                (static_cast<Ptr<Type>>(Context)->*Method)(* reinterpret_cast<Ptr<MAVoice>>(Object));
            };
            ma_sound_set_end_callback(&mObject, Callback, Instance);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ma_sound mObject;
        MASource mSource;
        Object   mHandle;
    };
}