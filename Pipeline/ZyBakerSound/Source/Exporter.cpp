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

#include "Exporter.hpp"
#include <Zyphryon.Audio/Decoder/AdaptiveDecoder.hpp>
#include <opus.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Sound
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static SInt16 Quantize(Real32 Value)
    {
        return static_cast<SInt16>(Clamp(Value, -1.0f, 1.0f) * 32767.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static UInt8 Compress(Ref<SInt32> Predictor, Ref<SInt32> Step, SInt16 Sample)
    {
        SInt32 Delta  = Sample - Predictor;
        UInt8  Nibble = 0;

        if (Delta < 0)
        {
            Nibble = 0x8;
            Delta  = -Delta;
        }

        SInt32 Size  = Audio::AdaptiveDecoder::kStepTable[Step];
        SInt32 Total = Size >> 3;

        if (Delta >= Size)
        {
            Nibble |= 0x4;
            Delta  -= Size;
            Total  += Size;
        }

        Size >>= 1;

        if (Delta >= Size)
        {
            Nibble |= 0x2;
            Delta  -= Size;
            Total  += Size;
        }

        Size >>= 1;

        if (Delta >= Size)
        {
            Nibble |= 0x1;
            Total  += Size;
        }

        Predictor = Clamp<SInt32>(Predictor + ((Nibble & 0x8) ? -Total : Total), -32768, 32767);
        Step      = Clamp<SInt32>(Step + Audio::AdaptiveDecoder::kStepIndex[Nibble], 0, Audio::AdaptiveDecoder::kStepLimit);
        return Nibble;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::EncodeLinear(ConstRef<Sample> Source)
    {
        const ConstSpan<Real32> Samples = Source.GetSamples();

        Blob              Payload = Blob::Allocate<SInt16>(Samples.GetSize());
        const Ptr<SInt16> Output  = Payload.GetData<SInt16>();

        for (UInt Index = 0; Index < Samples.GetSize(); ++Index)
        {
            Output[Index] = Quantize(Samples[Index]);
        }
        return Payload;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::EncodeAdaptive(ConstRef<Sample> Source)
    {
        // The block geometry is the decoder's, so the two stay in step by construction rather than by comment.
        constexpr UInt32 kBlockFrames = Audio::AdaptiveDecoder::kBlockFrames;
        constexpr UInt32 kBlockStride = Audio::AdaptiveDecoder::kBlockStride;

        const ConstSpan<Real32> Samples = Source.GetSamples();
        const UInt16            Stride  = Source.GetStride();
        const UInt64            Frames  = Source.GetFrames();
        const UInt64            Blocks  = (Frames + kBlockFrames - 1) / kBlockFrames;

        Blob            Payload = Blob::Allocate<Byte>(Blocks * Stride * kBlockStride);
        const Ptr<Byte> Output  = Payload.GetData<Byte>();

        Zero(Output, Blocks * Stride * kBlockStride);

        // The step index carries across blocks, which keeps a block boundary from resetting the quantiser to
        // its coarsest setting; the preamble records whatever it reached so the decoder resumes from it.
        Array<SInt32, Audio::kMixerStride> Steps { };

        for (UInt64 Block = 0; Block < Blocks; ++Block)
        {
            const UInt64 Origin = Block * kBlockFrames;

            for (UInt16 Channel = 0; Channel < Stride; ++Channel)
            {
                const Ptr<Byte> Cursor    = Output + (Block * Stride + Channel) * kBlockStride;
                SInt32          Predictor = Quantize(Samples[Origin * Stride + Channel]);

                Cursor[0] = static_cast<Byte>(Predictor & 0xFF);
                Cursor[1] = static_cast<Byte>((Predictor >> 8) & 0xFF);
                Cursor[2] = static_cast<Byte>(Steps[Channel]);
                Cursor[3] = 0;

                const UInt64 Length = Min<UInt64>(kBlockFrames, Frames - Origin);

                for (UInt64 Frame = 1; Frame < Length; ++Frame)
                {
                    const SInt16 Value  = Quantize(Samples[(Origin + Frame) * Stride + Channel]);
                    const UInt8  Nibble = Compress(Predictor, Steps[Channel], Value);
                    const UInt64 Slot   = 4 + ((Frame - 1) >> 1);

                    // Two samples share a byte, the earlier one in the low nibble.
                    Cursor[Slot] |= ((Frame - 1) & 1) ? (Nibble << 4) : Nibble;
                }
            }
        }
        return Payload;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::EncodeOpus(ConstRef<Sample> Source, SInt32 Bitrate)
    {
        constexpr UInt32 kPacketFrames = 960;
        constexpr UInt32 kPacketBytes  = 4000;

        const UInt16 Stride = Source.GetStride();

        SInt32               Result    = 0;
        const Ptr<OpusEncoder> Encoder = ::opus_encoder_create(
            Source.GetFrequency(), Stride, OPUS_APPLICATION_AUDIO, AddressOf(Result));

        if (Result != OPUS_OK)
        {
            LOG_E("Audio: failed to create the Opus encoder ({0})", StrConvert(::opus_strerror(Result)));
            return Blob();
        }

        ::opus_encoder_ctl(Encoder, OPUS_SET_BITRATE(Bitrate));

        // The encoder's lookahead is exactly the lead-in the decoder has to discard, so it becomes the preskip.
        opus_int32 Lookahead = 0;
        ::opus_encoder_ctl(Encoder, OPUS_GET_LOOKAHEAD(AddressOf(Lookahead)));

        const ConstSpan<Real32> Samples = Source.GetSamples();
        const UInt64            Frames  = Source.GetFrames();

        // Feeding the lookahead again as silence flushes the tail, so the final frames survive the delay.
        const UInt64 Total   = Frames + static_cast<UInt64>(Lookahead);
        const UInt64 Packets = (Total + kPacketFrames - 1) / kPacketFrames;

        Sequence<UInt16> Lengths(Packets);
        Sequence<Byte>   Encoded(Packets * 256);

        Blob            Staging = Blob::Allocate<Real32>(kPacketFrames * Stride);
        Blob            Packet  = Blob::Allocate<Byte>(kPacketBytes);
        const Ptr<Real32> Input = Staging.GetData<Real32>();

        for (UInt64 Index = 0; Index < Packets; ++Index)
        {
            const UInt64 Origin = Index * kPacketFrames;

            // Opus only accepts whole frames, so the final packet is padded rather than shortened.
            Zero(Input, kPacketFrames * Stride);

            if (Origin < Frames)
            {
                const UInt64 Length = Min<UInt64>(kPacketFrames, Frames - Origin);
                Copy(Input, Length * Stride, Samples.GetData() + Origin * Stride);
            }

            const SInt32 Written = ::opus_encode_float(Encoder, Input, kPacketFrames, Packet.GetData<Byte>(), kPacketBytes);

            if (Written < 0)
            {
                LOG_E("Audio: failed to encode an Opus packet ({0})", StrConvert(::opus_strerror(Written)));

                ::opus_encoder_destroy(Encoder);
                return Blob();
            }

            Lengths.Append(static_cast<UInt16>(Written));

            for (SInt32 Byte = 0; Byte < Written; ++Byte)
            {
                Encoded.Append(Packet.GetData<::Byte>()[Byte]);
            }
        }

        ::opus_encoder_destroy(Encoder);

        // The directory precedes the packets so a seek resolves without walking them.
        Writer Output(Encoded.GetSize() + Lengths.GetSize() * sizeof(UInt16) + 16);
        Output.Write<UInt16>(static_cast<UInt16>(Lookahead));
        Output.Write<UInt16>(kPacketFrames);
        Output.Write<UInt32>(static_cast<UInt32>(Lengths.GetSize()));

        for (const UInt16 Length : Lengths)
        {
            Output.Write<UInt16>(Length);
        }

        // The two-argument overload appends raw bytes; the one-argument form would serialize the span itself.
        Output.Write<Byte>(Encoded.GetData(), static_cast<UInt32>(Encoded.GetSize()));
        return Output.Detach();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Exporter::Export(ConstRef<Sample> Source, ConstRef<Profile> Profile)
    {
        if (Source.IsEmpty())
        {
            LOG_E("Audio: refusing to write a sound with no frames");
            return Blob();
        }

        if (Source.GetStride() > Audio::kMixerStride)
        {
            LOG_E("Audio: {0} channels exceeds what the mixer consumes", Source.GetStride());
            return Blob();
        }

        Blob Payload;

        switch (Profile.Encoding)
        {
        case Audio::Encoding::Adaptive:
            Payload = EncodeAdaptive(Source);
            break;
        case Audio::Encoding::Opus:
            Payload = EncodeOpus(Source, Profile.Bitrate);
            break;
        case Audio::Encoding::Linear:
            Payload = EncodeLinear(Source);
            break;
        }

        if (Payload == nullptr)
        {
            return Blob();
        }

        const ConstSpan<Byte> Bytes  = ConstSpan(Payload.GetData<Byte>(), Payload.GetSize());
        const UInt32          Length = static_cast<UInt32>(Bytes.GetSize());

        Writer Output(Length + 32);
        Output.Write<UInt32>(kMagic);
        Output.Write<UInt16>(kVersion);
        Output.Write<Audio::Encoding>(Profile.Encoding);
        Output.Write<UInt16>(Source.GetStride());
        Output.Write<UInt32>(Source.GetFrequency());
        Output.Write<UInt64>(Source.GetFrames());
        Output.Write<UInt32>(Length);

        // The loader reads a payload the same size as the raw count as uncompressed, so only a payload that
        // actually shrank is worth keeping.
        if (Profile.Compress)
        {
            Blob         Scratch = Blob::Allocate<Byte>(LZ4Bound(Length));
            const UInt32 Size    = LZ4Encode(Bytes, Scratch.GetData<Byte>(), LZ4Bound(Length), kCompression);

            if (Size > 0 && Size < Length)
            {
                Output.WriteBlock<UInt32, Byte>(ConstSpan(Scratch.GetData<Byte>(), Size));
                return Output.Detach();
            }
        }

        Output.WriteBlock<UInt32, Byte>(Bytes);
        return Output.Detach();
    }
}
