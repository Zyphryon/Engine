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

#include "MP3Decoder.hpp"
#include "Zyphryon.Audio/Types.hpp"

#define DR_MP3_IMPLEMENTATION
#include <dr_mp3.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    MP3Decoder::MP3Decoder(ConstSpan<Byte> Samples)
        : mDecoder   { },
          mAvailable { 0 },
          mConsumed  { 0 }
    {
        drmp3_init_memory(AddressOf(mDecoder), Samples.GetData(), Samples.GetSizeInBytes(), nullptr);

        // The mixer consumes a single fixed clock, so the decoder streams and resamples to that rate on the fly.
        mResampler = Audio::Resampler(mDecoder.sampleRate, Audio::kMixerFrequency, static_cast<UInt16>(mDecoder.channels));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    MP3Decoder::~MP3Decoder()
    {
        drmp3_uninit(AddressOf(mDecoder));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void MP3Decoder::Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const
    {
        if (Frequency)
        {
            (* Frequency) = Audio::kMixerFrequency;
        }
        if (Stride)
        {
            (* Stride) = mDecoder.channels;
        }
        if (Frames)
        {
            (* Frames) = Audio::Resampler::Estimate(mDecoder.totalPCMFrameCount, mDecoder.sampleRate, Audio::kMixerFrequency);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool MP3Decoder::Seek(UInt64 Frame)
    {
        if (mResampler.IsActive())
        {
            if (!drmp3_seek_to_pcm_frame(AddressOf(mDecoder), mResampler.GetSource(Frame)))
            {
                return false;
            }

            mAvailable = 0;
            mConsumed  = 0;
            mResampler.Reset(Frame);
            return true;
        }
        else
        {
            return drmp3_seek_to_pcm_frame(AddressOf(mDecoder), Frame);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 MP3Decoder::Tell() const
    {
        return mResampler.IsActive() ? mResampler.Tell() : mDecoder.currentPCMFrame;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 MP3Decoder::Read(Span<Real32> Output)
    {
        if (mResampler.IsActive())
        {
            const UInt64 Limit = Audio::Resampler::Estimate(mDecoder.totalPCMFrameCount, mDecoder.sampleRate, Audio::kMixerFrequency);

            // Provide source frames from a decoded block, refilling from dr_mp3 in bulk instead of one frame per call.
            const auto Provider = [this](Ptr<Real32> Frame) -> Bool
            {
                if (mConsumed >= mAvailable)
                {
                    mAvailable = static_cast<UInt32>(drmp3_read_pcm_frames_f32(AddressOf(mDecoder), kBlock, mScratch));
                    mConsumed  = 0;

                    if (mAvailable == 0)
                    {
                        return false;
                    }
                }

                const ConstPtr<Real32> Source = mScratch + mConsumed * mDecoder.channels;

                for (UInt16 Element = 0; Element < mDecoder.channels; ++Element)
                {
                    Frame[Element] = Source[Element];
                }
                ++mConsumed;
                return true;
            };
            return mResampler.Generate(Output, Limit, Provider);
        }
        else
        {
            return drmp3_read_pcm_frames_f32(AddressOf(mDecoder), Output.GetSize(), Output.GetData());
        }
    }
}