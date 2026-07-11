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

#include "Decoder.hpp"
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents an audio track resource containing raw audio data and its associated metadata.
    class Track final : public Content::AbstractResource<Track>
    {
    public:

        /// \brief The factory codec for creating decoders to process the track's audio data.
        using Codec = Delegate<Unique<Decoder>(ConstRef<Blob>, ConstRef<Track>), DelegateInlineSize::Smallest>;

    public:

        /// \brief Constructs the track resource with a unique key.
        ///
        /// \param Key The unique content key identifying this track.
        explicit Track(AnyRef<Content::Uri> Key);

        /// \brief Populates the track with audio data and its associated metadata.
        ///
        /// \param Frames    The total number of PCM frames in the track.
        /// \param Frequency The sample rate of the audio data, in Hz.
        /// \param Stride    The number of channels per PCM frame.
        /// \param Codec     The codec used to create decoders for this track's audio data.
        /// \param Samples   The raw audio data for the track, either PCM or encoded bytes.
        void Load(UInt64 Frames, UInt32 Frequency, UInt16 Stride, AnyRef<Codec> Codec, AnyRef<Blob> Samples);

        /// \brief Creates a new decoder instance for this track's audio data using the associated codec.
        ///
        /// \return A unique pointer to a new decoder instance, or `nullptr` if the codec fails to create one.
        ZY_INLINE Unique<Decoder> Decode() const
        {
            return mCodec(mSamples, * this);
        }

        /// \brief Gets the total number of PCM frames in the track.
        ///
        /// \return The total frame count.
        ZY_INLINE UInt64 GetFrames() const
        {
            return mFrames;
        }

        /// \brief Gets the total duration of the track.
        ///
        /// \return The duration in microseconds.
        ZY_INLINE UInt64 GetDuration() const
        {
            return mFrames * 1000000ULL / mFrequency;
        }

        /// \brief Gets the sample rate of the track.
        ///
        /// \return The frequency in Hz.
        ZY_INLINE UInt32 GetFrequency() const
        {
            return mFrequency;
        }

        /// \brief Gets the number of channels per PCM frame.
        ///
        /// \return The number of channels per PCM frame.
        ZY_INLINE UInt16 GetStride() const
        {
            return mStride;
        }

        /// \brief Gets the raw audio data held by this track.
        ///
        /// \return The span of bytes containing either PCM or encoded audio data.
        template<typename Type>
        ZY_INLINE ConstSpan<Type> GetSamples() const
        {
            return mSamples;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mFrames;
        UInt32 mFrequency;
        UInt16 mStride;
        Blob   mSamples;
        Codec  mCodec;
    };
}