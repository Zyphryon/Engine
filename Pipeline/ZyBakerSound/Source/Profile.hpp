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

#include <Zyphryon.Audio/Types.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Sound
{
    /// \brief The settings that steer one bake.
    struct Profile final
    {
        /// The encoding the samples are stored with.
        Audio::Encoding Encoding = Audio::Encoding::Linear;

        /// The rate opus targets across every channel, in bits per second; the other encodings ignore it.
        SInt32          Bitrate  = 96000;

        /// Whether the payload is LZ4-compressed when that shrinks the output.
        Bool            Compress = true;

        /// \brief Derives a profile from a parsed command line.
        ///
        /// \param Environment The parsed command line to read the switches from.
        /// \return The profile the switches describe.
        static Profile From(ConstRef<Environment> Environment)
        {
            Profile Result;
            Result.Encoding = Environment.Contains("opus")  ? Audio::Encoding::Opus
                            : Environment.Contains("adpcm") ? Audio::Encoding::Adaptive : Audio::Encoding::Linear;
            Result.Bitrate  = Environment.GetNumber<SInt32>("bitrate", Result.Bitrate);
            Result.Compress = Environment.GetBool("compressed", Result.Compress);
            return Result;
        }
    };
}