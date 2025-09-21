// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Base.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Constants used across the audio module.
    enum
    {
        /// \brief Number of audio frames processed per streaming block.
        kBlock        = 4096,

        /// \brief Maximum number of concurrent audio buffers.
        kMaxBuffers   = 3,

        /// \brief Maximum number of active audio instances.
        kMaxInstances = 256,

        /// \brief Maximum number of submixes allowed.
        kMaxSubmixes  = 16,
    };

    /// \brief Specifies the backend used for audio playback and mixing.
    enum class Backend : UInt8
    {
        None,   ///< No backend is initialized.
        FAudio, ///< Uses the FAudio backend for audio processing.
    };

    /// \brief Represents a high-level logical audio category for routing and volume control.
    enum class Category : UInt8
    {
        Music,      ///< Background music tracks.
        Effect,     ///< Sound effects like explosions or footsteps.
        Voice,      ///< Dialogue or voice-over audio.
        Ambient,    ///< Environmental or ambient background sounds.
        Interface,  ///< User interface sounds such as clicks or notifications.
    };

    /// \brief Specifies the audio data format.
    enum class Format : UInt8
    {
        ADPCM, ///< Adaptive Differential Pulse-Code Modulation.
        PCM,   ///< Standard uncompressed Pulse-Code Modulation.
        IEEE,  ///< IEEE 754 floating-point audio format.
    };

    /// \brief Identifier for an audio object (e.g., a source or buffer).
    using Object = UInt16;

    /// \brief Describes an audio adapter.
    struct Adapter final
    {
        /// \brief Human-readable name of the audio adapter.
        Str8 Name;
    };

    /// \brief Describes the capabilities and state of the audio backend.
    struct Capabilities final
    {
        /// \brief Active audio backend in use.
        Backend         Backend;

        /// \brief Name of the currently selected audio device.
        Str8            Device;

        /// \brief List of available audio adapters.
        Vector<Adapter> Adapters;
    };

    /// \brief Describes the schema of an audio stream.
    struct Schema final
    {
        /// \brief Constructs a schema with default values.
        ZYPHRYON_INLINE constexpr Schema() = default;

        /// \brief Constructs a schema with the given properties.
        ///
        /// \param Duration  Total number of sample frames in the stream.
        /// \param Frequency Sample rate in Hertz (samples per second).
        /// \param Format    Audio sample format (e.g., PCM, float).
        /// \param Layout    Physical channel layout (e.g., mono, stereo).
        /// \param Stride    Number of bytes per sample for a single channel.
        ZYPHRYON_INLINE constexpr Schema(UInt64 Duration, UInt32 Frequency, Format Format, UInt8 Layout, UInt8 Stride)
            : Duration  { Duration  },
              Frequency { Frequency },
              Format    { Format },
              Layout    { Layout },
              Stride    { Stride }
        {
        }

        /// \brief Total number of sample frames in the stream.
        UInt64 Duration;

        /// \brief Sampling rate in Hertz (samples per second).
        UInt32 Frequency;

        /// \brief Audio sample format.
        Format Format;

        /// \brief Physical channel layout (e.g., mono, stereo).
        UInt8  Layout;

        /// \brief Number of bytes per sample for a single channel.
        UInt8  Stride;
    };
}