// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Specifies the audio configuration constants.
    enum : UInt16
    {
        /// \brief Number of frames the mixer processes per internal sub-block.
        kMixerBlock     = 256,

        /// \brief Sample rate, in hertz, at which the mixer runs its internal graph.
        kMixerFrequency = 48000,

        /// \brief The preferred number of frames per device period (buffer length hint).
        kMixerPeriod    = 1024,

        /// \brief Number of interleaved samples per frame produced by the mixer (stereo stride).
        kMixerStride    = 2,

        /// \brief Capacity of the control and completion command queues.
        kMaxCommands    = 512,

        /// \brief Maximum number of active audio instances.
        kMaxInstances   = 256,

        /// \brief Cutoff frequency, in hertz, at or above which a voice's low-pass is fully open.
        kMixerCutoff    = 20000,
    };

    /// \brief Specifies the audio distance attenuation models.
    enum class Attenuation : UInt8
    {
        Linear,         ///< Linear distance attenuation model.
        Inverse,        ///< Inverse distance attenuation model.
        Exponential,    ///< Exponential distance attenuation model.
    };

    /// \brief Specifies the audio track categories.
    enum class Category : UInt8
    {
        Music,          ///< Background music tracks.
        Effect,         ///< Sound effects like explosions or footsteps.
        Voice,          ///< Dialogue or voice-over audio.
        Ambient,        ///< Environmental or ambient background sounds.
        Interface,      ///< User interface sounds such as clicks or notifications.
    };

    /// \brief Specifies why a playback instance ended.
    enum class Reason : UInt8
    {
        Completed,      ///< The source stream reached its end.
        Stopped,        ///< The playback was stopped by an explicit request.
        Superseded,     ///< The playback lost its mixing slot to a more important sound.
    };

    /// \brief Specifies how a baked sound stores its samples.
    enum class Encoding : UInt8
    {
        Linear,     ///< Signed 16-bit samples, widened on read.
        Adaptive,   ///< IMA ADPCM, four bits per sample, decoded a block at a time.
        Opus,       ///< Opus packets, decoded a packet at a time.
    };

    /// \brief Describes all audio hardware and backend information.
    struct Description final
    {
        /// The name of the audio backend in use.
        Str16           Backend = "None";

        /// The name of the selected audio adapter or output device.
        Str             Adapter = "Default";

        /// The list of available audio output endpoint names on the selected adapter.
        Sequence<Str>   Endpoints;
    };

    /// \brief A handle representing an active audio playback instance.
    using Object = Key<kMaxInstances>;

    /// \brief Describes a playback instance that ended and the reason it did.
    struct Completion final
    {
        /// The handle of the playback that ended.
        Object Handle;

        /// The reason the playback ended.
        Reason Reason = Reason::Completed;
    };

    /// \brief Holds the per-channel linear output gains for a stereo source.
    struct Gains final
    {
        /// The linear gain applied to the left output channel (range [0, 1]).
        Real32 Left  = 0.0f;

        /// The linear gain applied to the right output channel (range [0, 1]).
        Real32 Right = 0.0f;
    };
}