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
#include "Zyphryon.Audio/Decoder/Linear.hpp"
#include "Zyphryon.Audio/Decoder/Opus.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAudio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type, typename Sample>
    static Unique<Decoder> Decode(ConstRef<Blob> Data, ConstRef<Sound> Asset)
    {
        const ConstSpan Samples(Data.GetData<Sample>(), Data.GetSize() / sizeof(Sample));
        return Unique<Type>::Create(Samples, Asset.GetFrequency(), Asset.GetStride(), Asset.GetFrames());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool SNDLoader::Load(Ref<ZyContent::Service> Service, Ref<ZyContent::Scope> Scope, AnyRef<Blob> Data)
    {
        Reader Input(Data);

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

        const Encoding Format    = Input.Read<Encoding>();
        const UInt16   Stride    = Input.Read<UInt16>();
        const UInt32   Frequency = Input.Read<UInt32>();
        const UInt64   Frames    = Input.Read<UInt64>();

        if (Stride == 0 || Stride > kMixerStride)
        {
            LOG_W("'{0}' declares {1} channels, which the mixer cannot consume", Asset.GetKey(), Stride);
            return false;
        }

        Blob Buffer = Input.ReadBlockCompressed<UInt32>();

        if (!Buffer)
        {
            LOG_W("'{0}' has an invalid sound payload", Asset.GetKey());
            return false;
        }

        // The baker resamples to the mixer's clock, so the decoder is a reader rather than a converter.
        Sound::Codec Factory;

        switch (Format)
        {
        case Encoding::Linear:
            Factory = Decode<Codec::Linear, SInt16>;
            break;
        case Encoding::Adaptive:
            Factory = Decode<Codec::Adaptive, Byte>;
            break;
        case Encoding::Opus:
            Factory = Decode<Codec::Opus, Byte>;
            break;
        default:
            break;
        }

        Asset.Load(Frames, Frequency, Stride, Move(Factory), Move(Buffer));
        return true;
    }
}