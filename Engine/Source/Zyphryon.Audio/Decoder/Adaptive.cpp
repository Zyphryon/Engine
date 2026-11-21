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

namespace ZyAudio::Codec
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Adaptive::Adaptive(ConstSpan<Byte> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames)
        : Decoder  { Frequency, Stride, Frames },
          mSamples { Samples },
          mStale   { false }
    {
        ZY_ASSERT(Stride > 0 && Stride <= kMixerStride, "ADPCM carries at most the mixer's channel count");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Adaptive::Seek(UInt64 Frame)
    {
        if (Frame > mFrames)
        {
            return false;
        }

        mCursor = Frame;
        mStale  = true;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Adaptive::Read(Span<Real32> Output)
    {
        const UInt64 Count = Min<UInt64>(Output.GetSize() / mStride, mFrames - mCursor);

        for (UInt64 Done = 0; Done < Count; )
        {
            const UInt64 Position = mCursor + Done;
            const UInt64 Block    = Position / kBlockFrames;
            const UInt32 Offset   = static_cast<UInt32>(Position % kBlockFrames);
            const UInt32 Batch    = static_cast<UInt32>(Min<UInt64>(Count - Done, kBlockFrames - Offset));

            // A block decodes only from its preamble, which restarts every channel with the block's first frame.
            if (Offset == 0 || mStale)
            {
                Prime(Block);

                // A cursor moved without decoding stands inside a block, whose frames before it are walked over.
                if (Offset > 0)
                {
                    Array<Real32, kBlockFrames * kMixerStride> Discard;

                    Unpack(Block, 0, Offset, Discard.GetData());
                }
                mStale = false;
            }

            Unpack(Block, Offset, Batch, Output.GetData() + Done * mStride);
            Done += Batch;
        }

        mCursor += Count;
        return Count;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Adaptive::Skip(UInt64 Frames)
    {
        const UInt64 Advance = Min(Frames, mFrames - mCursor);

        // The channels are left behind for the next read to walk up, so the frames skipped are never decoded.
        mCursor += Advance;
        mStale   = true;
        return Advance;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt8 Adaptive::Compress(Ref<Channel> State, SInt16 Value)
    {
        SInt32 Delta  = Value - State.Predictor;
        UInt8  Nibble = 0;

        if (Delta < 0)
        {
            Nibble = 0x8;
            Delta  = -Delta;
        }

        SInt32 Size = kStepTable[State.Step];

        for (UInt8 Bit = 0x4; Bit > 0; Bit >>= 1, Size >>= 1)
        {
            if (Delta >= Size)
            {
                Nibble |= Bit;
                Delta  -= Size;
            }
        }

        Expand(State, Nibble);
        return Nibble;
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

    void Adaptive::Unpack(UInt64 Block, UInt32 First, UInt32 Count, Ptr<Real32> Output)
    {
        const UInt64 Size   = mSamples.GetSize();
        const UInt32 Stride = mStride;

        for (UInt32 Index = 0; Index < Stride; ++Index)
        {
            const UInt64         Base = Min<UInt64>((Block * Stride + Index) * kBlockStride + 4, Size);
            const ConstPtr<Byte> Run  = mSamples.GetData() + Base;
            const UInt64         Held = (Size - Base) * 2;

            Channel     State  = mChannels[Index];
            Ptr<Real32> Target = Output + Index;

            for (UInt32 Frame = First; Frame < First + Count; ++Frame, Target += Stride)
            {
                if (Frame > 0)
                {
                    const UInt32 Nibble = Frame - 1;
                    const Byte   Packed = (Nibble < Held ? Run[Nibble >> 1] : 0);

                    Expand(State, (Nibble & 1) ? (Packed >> 4) : (Packed & 0x0F));
                }
                (* Target) = static_cast<Real32>(State.Predictor) * (1.0f / 32768.0f);
            }
            mChannels[Index] = State;
        }
    }
}