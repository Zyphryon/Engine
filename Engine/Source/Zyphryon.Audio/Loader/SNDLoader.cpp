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

#include "SNDLoader.hpp"
#include "Zyphryon.Audio/Decoder/Adaptive.hpp"
#include "Zyphryon.Audio/Decoder/Opus.hpp"
#include "Zyphryon.Audio/Decoder/Linear.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Unique<Decoder> DecodeLinear(ConstRef<Blob> Data, ConstRef<Sound> Asset)
    {
        const ConstSpan Samples(Data.GetData<SInt16>(), Data.GetSize() / sizeof(SInt16));
        return Unique<Codec::Linear>::Create(Samples, Asset.GetFrequency(), Asset.GetStride(), Asset.GetFrames());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Unique<Decoder> DecodeAdaptive(ConstRef<Blob> Data, ConstRef<Sound> Asset)
    {
        const ConstSpan Samples(Data.GetData<Byte>(), Data.GetSize());
        return Unique<Codec::Adaptive>::Create(Samples, Asset.GetFrequency(), Asset.GetStride(), Asset.GetFrames());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Unique<Decoder> DecodeOpus(ConstRef<Blob> Data, ConstRef<Sound> Asset)
    {
        const ConstSpan Samples(Data.GetData<Byte>(), Data.GetSize());
        return Unique<Codec::Opus>::Create(Samples, Asset.GetFrequency(), Asset.GetStride(), Asset.GetFrames());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SNDLoader::Load(Ref<Content::Service> Service, Ref<Content::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data.GetData(), Data.GetSize());

        return Parse(Input, * Retainer<Sound>::Cast(Scope.GetResource()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SNDLoader::Parse(Ref<Reader> Input, Ref<Sound> Asset)
    {
        if (Input.Read<UInt32>() != ('Z' | ('S' << 8) | ('N' << 16) | ('D' << 24)))
        {
            LOG_W("'{0}' is not a ZSND file (bad magic)", Asset.GetKey());
            return false;
        }

        if (Input.Read<UInt16>() != 1)
        {
            LOG_W("'{0}' has an unsupported ZSND version", Asset.GetKey());
            return false;
        }

        const Encoding        Format    = Input.Read<Encoding>();
        const UInt16          Stride    = Input.Read<UInt16>();
        const UInt32          Frequency = Input.Read<UInt32>();
        const UInt64          Frames    = Input.Read<UInt64>();
        const UInt32          Size      = Input.Read<UInt32>();
        const ConstSpan<Byte> Payload   = Input.ReadBlock<UInt32, Byte>();

        if (Size == 0 || Payload.IsEmpty())
        {
            LOG_W("'{0}' has empty sound data", Asset.GetKey());
            return false;
        }

        if (Stride == 0 || Stride > kMixerStride)
        {
            LOG_W("'{0}' declares {1} channels, which the mixer cannot consume", Asset.GetKey(), Stride);
            return false;
        }

        // `Size` is the uncompressed byte count, so a payload smaller than `Size` is decompressed.
        Blob Buffer = Blob::Allocate<Byte>(Size);

        if (Size != Payload.GetSize())
        {
            if (LZ4Decode(Payload, Buffer.GetData<Byte>(), Size) != Size)
            {
                LOG_W("'{0}' failed to decompress ({1} != {2})", Asset.GetKey(), Payload.GetSize(), Size);
                return false;
            }
        }
        else
        {
            Buffer.Copy<Byte>(Payload.GetData(), Size);
        }

        // The baker resamples to the mixer's clock, so the decoder is a reader rather than a converter.
        Sound::Codec Codec;

        switch (Format)
        {
        case Encoding::Linear:
            Codec = DecodeLinear;
            break;
        case Encoding::Adaptive:
            Codec = DecodeAdaptive;
            break;
        case Encoding::Opus:
            Codec = DecodeOpus;
            break;
        default:
            break;
        }

        Asset.Load(Frames, Frequency, Stride, Move(Codec), Move(Buffer));
        return true;
    }
}
