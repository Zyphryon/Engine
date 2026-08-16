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

#include "Zyphryon.Audio/Decoder.hpp"
#include "Zyphryon.Audio/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Codec
{
    /// \brief Expands the IMA ADPCM blocks of a baked sound into the mixer's floating-point frames.
    class Adaptive final : public Decoder
    {
    public:

        /// \brief The number of frames one ADPCM block carries for a single channel.
        static constexpr UInt32 kBlockFrames   = 512;

        /// \brief The bytes one ADPCM block occupies for a single channel.
        static constexpr UInt32 kBlockStride   = 4 + kBlockFrames / 2;

        /// \brief The step index adjustment each nibble applies, as defined by IMA ADPCM.
        static constexpr SInt32 kStepIndex[16] =
        {
            -1, -1, -1, -1, 2, 4, 6, 8, -1, -1, -1, -1, 2, 4, 6, 8
        };

        /// \brief The largest index the step table accepts.
        static constexpr SInt32 kStepLimit     = 88;

        /// \brief The quantiser step each index selects, as defined by IMA ADPCM.
        static constexpr SInt32 kStepTable[89] =
        {
            7,     8,     9,     10,    11,    12,    13,    14,    16,    17,
            19,    21,    23,    25,    28,    31,    34,    37,    41,    45,
            50,    55,    60,    66,    73,    80,    88,    97,    107,   118,
            130,   143,   157,   173,   190,   209,   230,   253,   279,   307,
            337,   371,   408,   449,   494,   544,   598,   658,   724,   796,
            876,   963,   1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,
            2272,  2499,  2749,  3024,  3327,  3660,  4026,  4428,  4871,  5358,
            5894,  6484,  7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
            15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
        };

    public:

        /// \brief Constructs a decoder over the blocks of a baked sound.
        ///
        /// \param Samples   The block runs to read from.
        /// \param Frequency The rate the blocks were baked at, in hertz.
        /// \param Stride    The number of channels per frame.
        /// \param Frames    The number of frames the blocks hold.
        Adaptive(ConstSpan<Byte> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames);

        /// \see Decoder::Probe(Ptr<UInt32>, Ptr<UInt32>, Ptr<UInt64>) const
        void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const override;

        /// \see Decoder::Seek(UInt64)
        Bool Seek(UInt64 Frame) override;

        /// \see Decoder::Tell() const
        UInt64 Tell() const override;

        /// \see Decoder::Read(Span<Real32>)
        UInt64 Read(Span<Real32> Output) override;

    private:

        /// \brief Holds the state one channel's decoder carries from each sample to the next.
        struct Channel final
        {
            /// The most recently reconstructed sample.
            SInt32 Predictor = 0;

            /// The index into the step table the next nibble is scaled by.
            SInt32 Step      = 0;
        };

        /// \brief Restarts every channel from the preamble of the given block.
        ///
        /// \param Block The zero-based index of the block to restart from.
        void Prime(UInt64 Block);

        /// \brief Fetches one nibble from a channel's run within a block.
        ///
        /// \param Block   The zero-based index of the block to read.
        /// \param Channel The channel whose run holds the nibble.
        /// \param Index   The zero-based position of the nibble within the run.
        /// \return The four bits at that position, or zero when the block runs past the payload.
        UInt8 Fetch(UInt64 Block, UInt16 Channel, UInt32 Index) const;

        /// \brief Reconstructs one sample from a single nibble, advancing the channel that produced it.
        ///
        /// \param State  The channel state to advance.
        /// \param Nibble The four bits to expand.
        /// \return The reconstructed sample.
        static SInt32 Expand(Ref<Channel> State, UInt8 Nibble);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstSpan<Byte>              mSamples;
        UInt32                       mFrequency;
        UInt16                       mStride;
        UInt64                       mFrames;
        UInt64                       mCursor;
        Array<Channel, kMixerStride> mChannels;
    };
}