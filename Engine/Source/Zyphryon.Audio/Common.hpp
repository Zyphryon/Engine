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
        /// \brief Maximum number of active audio instances.
        kMaxInstances = 256,
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
    using Object = UInt16;
}