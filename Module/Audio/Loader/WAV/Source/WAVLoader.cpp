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

#include "WAVLoader.hpp"
#include "WAVDecoder.hpp"
#include "Zyphryon.Audio/Resampler.hpp"
#include "Zyphryon.Audio/Track.hpp"
#include "Zyphryon.Audio/Types.hpp"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Unique<Audio::Decoder> Decode(ConstRef<Blob> Data, ConstRef<Audio::Track> Track)
    {
        ConstSpan Samples(Data.GetData<Real32>(), Data.GetSize() / sizeof(Real32));
        return Unique<WAVDecoder>::Create(Samples, Track.GetStride(), Track.GetFrequency(), Track.GetFrames());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool WAVLoader::Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data)
    {
        drwav Description;

        if (drwav_init_memory(AddressOf(Description), Data.GetData(), Data.GetSize(), nullptr))
        {
            UInt64       Frames    = Description.totalPCMFrameCount;
            const UInt32 Frequency = Description.sampleRate;
            const UInt32 Stride    = Description.channels;

            // Allocate a buffer for the decoded PCM frames and read them into memory.
            Blob Samples = Blob::Allocate<Real32>(Frames * Stride);
            drwav_read_pcm_frames_f32(AddressOf(Description), Frames, Samples.GetData<Real32>());
            drwav_uninit(AddressOf(Description));

            // Bake the PCM to the mixer's fixed sample rate, since the mixer consumes every voice at that clock.
            if (Frequency != Audio::kMixerFrequency)
            {
                const ConstSpan Source(Samples.GetData<Real32>(), Frames * Stride);
                Samples = Audio::Resampler::Convert(Source, Stride, Frames, Frequency, Audio::kMixerFrequency, Frames);
            }

            // Load the decoded PCM data into the track resource, now clocked at the mixer sample rate.
            const Retainer<Audio::Track> Asset = Retainer<Audio::Track>::Cast(Scope.GetResource());
            Asset->Load(Frames, Audio::kMixerFrequency, Stride, Decode, Move(Samples));
            return true;
        }
        return false;
    }
}