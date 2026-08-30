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
#include "Filter.hpp"
#include "Zyphryon.Math/Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Runtime state for a single active playback instance, owned by the mixer's voice pool.
    struct Voice final
    {
        /// The decoder that owns and supplies the source PCM (already at the mixer sample rate).
        Unique<Decoder> Decoder;

        /// The spatial configuration used when \c Spatial is set.
        Emitter         Emitter;

        /// The source's world-space position (spatial voices only).
        Vector3         Position;

        /// The source's normalized forward axis (spatial voices only).
        Vector3         Forward   = -Vector3::UnitZ();

        /// The base linear volume of the voice (range [0, 1]).
        Real32          Volume       = 1.0f;

        /// The current smoothed per-channel gains, used to deramp toward the target and avoid clicks.
        Gains           Gain;

        /// The low-pass memory the left channel carries, or the mono image of a spatial voice.
        Filter::State   FilterLeft   = { };

        /// The low-pass memory the right channel carries, unused by a spatial voice.
        Filter::State   FilterRight  = { };

        /// The cutoff frequency, in hertz, the low-pass currently runs at.
        Real32          Cutoff       = static_cast<Real32>(kMixerCutoff);

        /// The cutoff frequency, in hertz, the low-pass glides toward, so occlusion ramps instead of switching.
        Real32          CutoffTarget = static_cast<Real32>(kMixerCutoff);

        /// The handle that identifies this voice, mirrored here so completed voices can be reaped by handle.
        Object          Handle;

        /// The number of interleaved samples the decoder produces per frame (stride).
        UInt16          Stride       = 2;

        /// The submix category this voice is routed through.
        Category        Category     = Category::Effect;

        /// `true` when the voice is spatialized, `false` for a direct stereo voice.
        Bool            Spatial      = false;

        /// `true` when the voice loops on reaching the end of its stream.
        Bool            Looping      = false;

        /// `true` when the voice is paused and should not advance or contribute audio.
        Bool            Paused       = false;

        /// `true` once the voice has produced audio at least once, so gains prime instead of ramping from zero.
        Bool            Primed       = false;

        /// `true` when the voice has ended and is awaiting reaping.
        Bool            Finished     = false;

        /// The reason the voice ended, reported to the game thread once it is reaped.
        Reason          Reason       = Reason::Completed;
    };
}