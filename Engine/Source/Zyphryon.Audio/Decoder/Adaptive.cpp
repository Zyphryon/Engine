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

#include "Adaptive.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Codec
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Adaptive::Adaptive(ConstSpan<Byte> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames)
        : mSamples   { Samples },
          mFrequency { Frequency },
          mStride    { Stride },
          mFrames    { Frames },
          mCursor    { 0 }
    {
        ZY_ASSERT(Stride > 0 && Stride <= kMixerStride, "ADPCM carries at most the mixer's channel count");

        Prime(0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Adaptive::Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const
    {
        (* Frequency) = mFrequency;
        (* Stride)    = mStride;
        (* Frames)    = mFrames;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Adaptive::Seek(UInt64 Frame)
    {
        if (Frame > mFrames)
        {
            return false;
        }

        // A block only decodes from its preamble, so restart there and walk forward to the frame.
        const UInt64 Block = Frame / kBlockFrames;

        Prime(Block);
        mCursor = Block * kBlockFrames;

        Array<Real32, kMixerStride> Discard;

        while (mCursor < Frame)
        {
            Read(Span(Discard.GetData(), mStride));
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Adaptive::Tell() const
    {
        return mCursor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Adaptive::Read(Span<Real32> Output)
    {
        const UInt64 Count = Min<UInt64>(Output.GetSize() / mStride, mFrames - mCursor);

        for (UInt64 Frame = 0; Frame < Count; ++Frame)
        {
            const UInt64 Position = mCursor + Frame;
            const UInt64 Block    = Position / kBlockFrames;
            const UInt32 Offset   = static_cast<UInt32>(Position % kBlockFrames);

            // The preamble carries the block's first frame verbatim, and restarts the channel state with it.
            if (Offset == 0)
            {
                Prime(Block);
            }

            for (UInt16 Index = 0; Index < mStride; ++Index)
            {
                Ref<Channel> State = mChannels[Index];

                const SInt32 Value = (Offset == 0) ? State.Predictor : Expand(State, Fetch(Block, Index, Offset - 1));
                Output[Frame * mStride + Index] = static_cast<Real32>(Value) * (1.0f / 32768.0f);
            }
        }

        mCursor += Count;
        return Count;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Adaptive::Skip(UInt64 Frames)
    {
        constexpr UInt32 kChunk = 64;

        Array<Real32, kChunk * kMixerStride> Discard;

        UInt64 Done = 0;

        while (Done < Frames)
        {
            const UInt64 Request = Min(Frames - Done, static_cast<UInt64>(kChunk)) * mStride;
            const UInt64 Decoded = Read(Span(Discard.GetData(), Request));

            if (Decoded == 0)
            {
                break;
            }
            Done += Decoded;
        }
        return Done;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Adaptive::Prime(UInt64 Block)
    {
        for (UInt16 Index = 0; Index < mStride; ++Index)
        {
            const UInt64 Base = (Block * mStride + Index) * kBlockStride;

            if (Base + 4 > mSamples.GetSize())
            {
                continue;
            }

            Ref<Channel> State = mChannels[Index];
            State.Predictor    = static_cast<SInt16>(mSamples[Base] | (mSamples[Base + 1] << 8));
            State.Step         = Clamp<SInt32>(mSamples[Base + 2], 0, kStepLimit);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt8 Adaptive::Fetch(UInt64 Block, UInt16 Channel, UInt32 Index) const
    {
        const UInt64 Base = (Block * mStride + Channel) * kBlockStride + 4 + (Index >> 1);

        if (Base < mSamples.GetSize())
        {
            const Byte Packed = mSamples[Base];
            return (Index & 1) ? (Packed >> 4) : (Packed & 0x0F);
        }
        return 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SInt32 Adaptive::Expand(Ref<Channel> State, UInt8 Nibble)
    {
        const SInt32 Step = kStepTable[State.Step];

        // The magnitude is the step scaled by each set bit, which is the reconstruction IMA specifies.
        SInt32 Delta = Step >> 3;

        if (Nibble & 0x1)
        {
            Delta += Step >> 2;
        }
        if (Nibble & 0x2)
        {
            Delta += Step >> 1;
        }
        if (Nibble & 0x4)
        {
            Delta += Step;
        }

        State.Predictor = Clamp<SInt32>(State.Predictor + ((Nibble & 0x8) ? -Delta : Delta), -32768, 32767);
        State.Step      = Clamp<SInt32>(State.Step + kStepIndex[Nibble], 0, kStepLimit);
        return State.Predictor;
    }
}