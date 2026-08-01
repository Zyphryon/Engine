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

#include "Profile.hpp"
#include "Sample.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Sound
{
    /// \brief Writes decoded samples out in the engine's native sound format.
    class Exporter final
    {
    public:

        /// \brief The file extension a baked sound is written with.
        static constexpr Text   kOutput  = "snd";

        /// \brief The four-character code every baked sound starts with, stored little-endian as `ZSND`.
        static constexpr UInt32 kMagic   = 'Z' | ('S' << 8) | ('N' << 16) | ('D' << 24);

        /// \brief The layout revision of the header this exporter writes.
        static constexpr UInt16 kVersion = 1;

    public:

        /// \brief Serializes decoded samples into a native sound blob.
        ///
        /// \param Source  The decoded samples to write, already clocked at the mixer's rate.
        /// \param Profile The settings controlling encoding and compression.
        /// \return A blob holding the sound file bytes, or an empty blob on failure.
        static Blob Export(ConstRef<Sample> Source, ConstRef<Profile> Profile);

    private:

        /// \brief Quantises interleaved samples to signed 16-bit.
        ///
        /// \param Source The samples to quantise.
        /// \return The quantised payload.
        static Blob EncodeLinear(ConstRef<Sample> Source);

        /// \brief Compresses interleaved samples into IMA ADPCM blocks, one run per channel per block.
        ///
        /// \param Source The samples to compress.
        /// \return The compressed payload.
        static Blob EncodeAdaptive(ConstRef<Sample> Source);

        /// \brief Compresses interleaved samples into Opus packets, preceded by the directory that indexes them.
        ///
        /// \param Source  The samples to compress.
        /// \param Bitrate The rate the encoder targets across every channel, in bits per second.
        /// \return The compressed payload, or an empty blob on failure.
        static Blob EncodeOpus(ConstRef<Sample> Source, SInt32 Bitrate);
    };
}
