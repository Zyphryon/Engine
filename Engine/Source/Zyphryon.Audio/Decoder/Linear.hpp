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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Codec
{
    /// \brief Widens the signed 16-bit samples of a baked sound into the mixer's floating-point frames.
    class Linear final : public Decoder
    {
    public:

        /// \brief Constructs a decoder over the samples of a baked sound.
        ///
        /// \param Samples   The interleaved samples to read from.
        /// \param Frequency The rate the samples were baked at, in hertz.
        /// \param Stride    The number of channels per frame.
        /// \param Frames    The number of frames the samples hold.
        Linear(ConstSpan<SInt16> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames);

        /// \see Decoder::Probe(Ptr<UInt32>, Ptr<UInt32>, Ptr<UInt64>) const
        void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const override;

        /// \see Decoder::Seek(UInt64)
        Bool Seek(UInt64 Frame) override;

        /// \see Decoder::Tell() const
        UInt64 Tell() const override;

        /// \see Decoder::Read(Span<Real32>)
        UInt64 Read(Span<Real32> Output) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstSpan<SInt16> mSamples;
        UInt32            mFrequency;
        UInt16            mStride;
        UInt64            mFrames;
        UInt64            mCursor;
    };
}