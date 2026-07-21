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
#include "Zyphryon.Audio/Resampler.hpp"
#include <dr_mp3.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Decoder implementation that streams and decodes MP3-encoded audio on demand from an in-memory blob.
    class MP3Decoder final : public Audio::Decoder
    {
    public:

        /// \brief Number of source frames decoded from dr_mp3 per refill.
        static constexpr UInt32 kBlock = 1024;

    public:

        /// \brief Initializes the MP3 decoder from an in-memory encoded audio blob.
        ///
        /// \param Samples The span of bytes containing the raw MP3-encoded audio data.
        MP3Decoder(ConstSpan<Byte> Samples);

        /// \brief Releases all resources associated with the MP3 decoder.
        ~MP3Decoder() override;

        /// \see Decoder::Probe(Ptr<UInt32>, Ptr<UInt32>, Ptr<UInt64>)
        void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const override;

        /// \see Decoder::Seek(UInt64)
        Bool Seek(UInt64 Frame) override;

        /// \see Decoder::Tell()
        UInt64 Tell() const override;

        /// \see Decoder::Read(Span<Real32>)
        UInt64 Read(Span<Real32> Output) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        drmp3            mDecoder;
        Audio::Resampler mResampler;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32           mScratch[kBlock * Audio::Resampler::kMaxStride];
        UInt32           mAvailable;
        UInt32           mConsumed;
    };
}