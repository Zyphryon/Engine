// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Opus.hpp"
#include "Zyphryon.Audio/Types.hpp"
#include <opus.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Codec
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Opus::Opus(ConstSpan<Byte> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames)
        : mHandle       { nullptr },
          mFrequency    { Frequency },
          mStride       { Stride },
          mFrames       { Frames },
          mCursor       { 0 },
          mPreskip      { 0 },
          mPacketFrames { 0 },
          mStaged       { 0 },
          mStagedFrames { 0 }
    {
        ZY_ASSERT(Stride > 0 && Stride <= kMixerStride, "Opus carries at most the mixer's channel count");

        // The directory sits at the head of the payload, so the packets can be addressed without a scan.
        constexpr UInt32 kHeader = sizeof(UInt16) + sizeof(UInt16) + sizeof(UInt32);

        if (Samples.GetSize() < kHeader)
        {
            LOG_E("Audio: the Opus payload is too small to hold its directory");
            return;
        }

        Reader Input(Samples.GetData(), Samples.GetSize());

        mPreskip               = Input.Read<UInt16>();
        mPacketFrames          = Input.Read<UInt16>();
        const UInt32 Count     = Input.Read<UInt32>();
        const UInt32 Directory = kHeader + Count * sizeof(UInt16);

        if (mPacketFrames == 0 || mPacketFrames > kMaxPacketFrames || Samples.GetSize() < Directory)
        {
            LOG_E("Audio: the Opus payload declares a directory it cannot hold");
            return;
        }

        mLengths = ConstSpan(reinterpret_cast<ConstPtr<UInt16>>(Samples.GetData() + kHeader), Count);
        mPackets = ConstSpan(Samples.GetData() + Directory, Samples.GetSize() - Directory);

        // The directory holds lengths, so the byte offsets are accumulated once here rather than per decode.
        mOffsets = Blob::Allocate<UInt32>(Count);

        for (UInt32 Index = 0, Origin = 0; Index < Count; ++Index)
        {
            mOffsets.GetData<UInt32>()[Index] = Origin;
            Origin                           += mLengths[Index];
        }

        SInt32 Result = 0;
        mHandle       = ::opus_decoder_create(static_cast<opus_int32>(Frequency), Stride, AddressOf(Result));

        if (Result != OPUS_OK)
        {
            LOG_E("Audio: failed to create the Opus decoder ({0})", StrConvert(::opus_strerror(Result)));

            mHandle = nullptr;
            return;
        }

        mStaging = Blob::Allocate<Real32>(kMaxPacketFrames * Stride);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Opus::~Opus()
    {
        if (mHandle)
        {
            ::opus_decoder_destroy(static_cast<Ptr<::OpusDecoder>>(mHandle));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Opus::Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const
    {
        (* Frequency) = mFrequency;
        (* Stride)    = mStride;
        (* Frames)    = mFrames;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Opus::Seek(UInt64 Frame)
    {
        if (Frame > mFrames || mHandle == nullptr)
        {
            return false;
        }

        ::opus_decoder_ctl(static_cast<Ptr<::OpusDecoder>>(mHandle), OPUS_RESET_STATE);

        mStaged = 0;
        mCursor = Frame;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Opus::Tell() const
    {
        return mCursor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Opus::Expand(UInt32 Packet)
    {
        if (mStaged == Packet + 1)
        {
            return mStagedFrames;
        }

        if (Packet >= mLengths.GetSize())
        {
            return 0;
        }

        const UInt32 Origin = mOffsets.GetData<UInt32>()[Packet];

        const SInt32 Produced = ::opus_decode_float(
            static_cast<Ptr<::OpusDecoder>>(mHandle),
            mPackets.GetData() + Origin,
            mLengths[Packet],
            mStaging.GetData<Real32>(),
            kMaxPacketFrames,
            0);

        if (Produced < 0)
        {
            LOG_E("Audio: failed to decode an Opus packet ({0})", StrConvert(::opus_strerror(Produced)));
            return 0;
        }

        mStaged       = Packet + 1;
        mStagedFrames = static_cast<UInt32>(Produced);
        return mStagedFrames;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Opus::Read(Span<Real32> Output)
    {
        if (mHandle == nullptr)
        {
            return 0;
        }

        const UInt64 Count = Min<UInt64>(Output.GetSize() / mStride, mFrames - mCursor);

        UInt64 Done = 0;

        while (Done < Count)
        {
            const UInt64 Position = mCursor + Done + mPreskip;
            const UInt32 Packet   = static_cast<UInt32>(Position / mPacketFrames);
            const UInt32 Offset   = static_cast<UInt32>(Position % mPacketFrames);
            const UInt32 Produced = Expand(Packet);

            if (Produced <= Offset)
            {
                break;
            }

            const UInt64 Batch = Min<UInt64>(Count - Done, Produced - Offset);

            const Ptr<Real32> Target = Output.GetData() + Done * mStride;
            const Ptr<Real32> Source = mStaging.GetData<Real32>() + Offset * mStride;
            Copy(Target, Batch * mStride, Source);

            Done += Batch;
        }

        mCursor += Done;
        return Done;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Opus::Skip(UInt64 Frames)
    {
        const UInt64 Advance = Min(Frames, mFrames - mCursor);
        return (Advance > 0 && Seek(mCursor + Advance) ? Advance : 0);
    }
}