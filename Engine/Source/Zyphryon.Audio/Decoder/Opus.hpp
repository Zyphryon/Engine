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
    /// \brief Expands the Opus packets of a baked sound into the mixer's floating-point frames.
    class Opus final : public Decoder
    {
    public:

        /// \brief Constructs a decoder over the packets of a baked sound.
        ///
        /// \param Samples   The payload to read, opening with the packet directory.
        /// \param Frequency The rate the packets were encoded at, in hertz.
        /// \param Stride    The number of channels per frame.
        /// \param Frames    The number of frames the packets hold, excluding the preskip.
        Opus(ConstSpan<Byte> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames);

        /// \brief Releases the native decoder.
        ~Opus() override;

        /// \see Decoder::Probe(Ptr<UInt32>, Ptr<UInt32>, Ptr<UInt64>) const
        void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const override;

        /// \see Decoder::Seek(UInt64)
        Bool Seek(UInt64 Frame) override;

        /// \see Decoder::Tell() const
        UInt64 Tell() const override;

        /// \see Decoder::Read(Span<Real32>)
        UInt64 Read(Span<Real32> Output) override;

    private:

        /// \brief The largest frame count a single Opus packet can carry at 48 kHz, being 120 milliseconds.
        static constexpr UInt32 kMaxPacketFrames = 5760;

        /// \brief Decodes the packet at the given index into the staging buffer.
        ///
        /// \param Packet The zero-based index of the packet to decode.
        /// \return The number of frames the packet produced, or zero on failure.
        UInt32 Expand(UInt32 Packet);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<void>         mHandle;
        ConstSpan<UInt16> mLengths;
        ConstSpan<Byte>   mPackets;
        Blob              mOffsets;
        UInt32            mFrequency;
        UInt16            mStride;
        UInt64            mFrames;
        UInt64            mCursor;
        UInt32            mPreskip;
        UInt32            mPacketFrames;
        UInt32            mStaged;
        UInt32            mStagedFrames;
        Blob              mStaging;
    };
}
