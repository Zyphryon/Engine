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
    /// \brief Constants used across the audio module.
    enum
    {
        /// \brief Maximum number of active audio instances.
        kMaxInstances = 256,
    };

    /// \brief Enumeration of distance attenuation models.
    enum class Attenuation : UInt8
    {
        Linear,         ///< Linear distance attenuation model.
        Inverse,        ///< Inverse distance attenuation model.
        Exponential,    ///< Exponential distance attenuation model.
    };

    /// \brief Enumeration of available audio backends.
    enum class Backend : UInt8
    {
        None,      ///< No backend is initialized.
        Miniaudio, ///< Uses the default library for audio processing.
    };

    /// \brief Enumeration of audio track categories.
    enum class Category : UInt8
    {
        Music,      ///< Background music tracks.
        Effect,     ///< Sound effects like explosions or footsteps.
        Voice,      ///< Dialogue or voice-over audio.
        Ambient,    ///< Environmental or ambient background sounds.
        Interface,  ///< User interface sounds such as clicks or notifications.
    };

    /// \brief Enumeration of audio channel topologies.
    enum class Topology : UInt8
    {
        Mono        = 1,    ///< Single audio channel.
        Stereo      = 2,    ///< Two audio channels (left and right).
        Surround4_0 = 4,    ///< Four audio channels (4.0 surround sound).
        Surround5_1 = 6,    ///< Five audio channels with one subwoofer (5.1 surround sound).
        Surround7_1 = 8,    ///< Seven audio channels with one subwoofer (7.1 surround sound).
    };

    /// \brief Describes the capabilities and state of the audio backend.
    struct Capabilities final
    {
        /// The audio backend currently in use.
        Backend      Backend  = Backend::None;

        /// The audio device currently selected.
        Str8         Output;

        /// List of available audio output devices.
        Vector<Str8> Devices;
    };

    /// \brief A handle representing an active audio playback instance.
    using Object = UInt32;
}