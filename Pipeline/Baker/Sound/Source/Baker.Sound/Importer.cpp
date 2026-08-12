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

#include "Importer.hpp"
#include <Zyphryon.Audio/Resampler.hpp>
#include <Zyphryon.Audio/Types.hpp>

#define DR_FLAC_IMPLEMENTATION
#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include <dr_flac.h>
#include <dr_mp3.h>
#include <dr_wav.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Sound
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Sample Resample(AnyRef<Blob> Samples, UInt32 Frequency, UInt16 Stride, UInt64 Frames)
    {
        if (Frequency == Audio::kMixerFrequency)
        {
            return Sample(Move(Samples), Frequency, Stride, Frames);
        }

        const ConstSpan Source(Samples.GetData<Real32>(), Frames * Stride);

        UInt64 Produced = 0;
        Blob   Data     = Audio::Resampler::Convert(Source, Stride, Frames, Frequency, Audio::kMixerFrequency, Produced);
        return Sample(Move(Data), Audio::kMixerFrequency, Stride, Produced);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Sample ImportFLAC(ConstRef<Blob> Data)
    {
        const Ptr<drflac> Description = drflac_open_memory(Data.GetData(), Data.GetSize(), nullptr);

        if (Description == nullptr)
        {
            LOG_E("Audio: the source is not a readable FLAC stream");
            return Sample();
        }

        const UInt64 Frames    = Description->totalPCMFrameCount;
        const UInt32 Frequency = Description->sampleRate;
        const UInt16 Stride    = static_cast<UInt16>(Description->channels);

        Blob Samples = Blob::Allocate<Real32>(Frames * Stride);
        drflac_read_pcm_frames_f32(Description, Frames, Samples.GetData<Real32>());
        drflac_close(Description);

        return Resample(Move(Samples), Frequency, Stride, Frames);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Sample ImportMP3(ConstRef<Blob> Data)
    {
        drmp3 Description;

        if (!drmp3_init_memory(AddressOf(Description), Data.GetData(), Data.GetSize(), nullptr))
        {
            LOG_E("Audio: the source is not a readable MP3 stream");
            return Sample();
        }

        const UInt64 Frames    = drmp3_get_pcm_frame_count(AddressOf(Description));
        const UInt32 Frequency = Description.sampleRate;
        const UInt16 Stride    = static_cast<UInt16>(Description.channels);

        Blob Samples = Blob::Allocate<Real32>(Frames * Stride);
        drmp3_read_pcm_frames_f32(AddressOf(Description), Frames, Samples.GetData<Real32>());
        drmp3_uninit(AddressOf(Description));

        return Resample(Move(Samples), Frequency, Stride, Frames);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Sample ImportWAV(ConstRef<Blob> Data)
    {
        drwav Description;

        if (!drwav_init_memory(AddressOf(Description), Data.GetData(), Data.GetSize(), nullptr))
        {
            LOG_E("Audio: the source is not a readable WAV stream");
            return Sample();
        }

        const UInt64 Frames    = Description.totalPCMFrameCount;
        const UInt32 Frequency = Description.sampleRate;
        const UInt16 Stride    = Description.channels;

        Blob Samples = Blob::Allocate<Real32>(Frames * Stride);
        drwav_read_pcm_frames_f32(AddressOf(Description), Frames, Samples.GetData<Real32>());
        drwav_uninit(AddressOf(Description));

        return Resample(Move(Samples), Frequency, Stride, Frames);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sample Importer::Import(ConstRef<Blob> Data, Text Extension)
    {
        switch (Hash(Extension))
        {
        case "flac"_Hash:
            return ImportFLAC(Data);
        case "mp3"_Hash:
            return ImportMP3(Data);
        case "wav"_Hash:
            return ImportWAV(Data);
        default:
            LOG_E("Audio: '{0}' is not a source this baker reads", Extension);
            break;
        }
        return Sample();
    }
}