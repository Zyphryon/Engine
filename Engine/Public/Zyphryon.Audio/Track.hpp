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

#include "Common.hpp"
#include "Codec.hpp"
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Represents an audio track resource containing encoded audio data and metadata.
    class Track final : public Content::AbstractResource<Track>
    {
        friend class AbstractResource;

    public:

        /// \brief Constructs the track resource with a unique key.
        ///
        /// \param Key The unique content key identifying this track.
        explicit Track(AnyRef<Content::Uri> Key);

        /// \brief Loads the track with specified parameters and encoded data.
        ///
        /// \param Duration  The duration of the track in frames.
        /// \param Frequency The sampling frequency of the track in Hz.
        /// \param Topology  The topology of the track.
        /// \param Data      The encoded audio data blob.
        /// \param Codec     The codec used to decode the audio data.
        void Load(UInt64 Duration, UInt32 Frequency, Topology Topology, AnyRef<Blob> Data, Ptr<Codec> Codec);

        /// \brief Gets the duration of the track in frames.
        ///
        /// \return The duration in frames.
        ZYPHRYON_INLINE UInt64 GetDuration() const
        {
            return mDuration;
        }

        /// \brief Gets the sampling frequency of the track in Hz.
        ///
        /// \return The frequency in Hz.
        ZYPHRYON_INLINE UInt32 GetFrequency() const
        {
            return mFrequency;
        }

        /// \brief Gets the number of channels in the track.
        ///
        /// \return The number of channels.
        ZYPHRYON_INLINE UInt32 GetWidth() const
        {
            return static_cast<UInt32>(mTopology);
        }

        /// \brief Gets the topology of the track.
        ///
        /// \return The track topology.
        ZYPHRYON_INLINE Topology GetTopology() const
        {
            return mTopology;
        }

        /// \brief Decodes the track's audio data into a decoder instance.
        ///
        /// \return A decoder object.
        ZYPHRYON_INLINE Tracker<Decoder> Decode() const
        {
            return mCodec->Decode(mData, *this);
        }

    private:

        /// \copydoc Resource::OnCreate(Ref<Service::Host>)
        Bool OnCreate(Ref<Service::Host> Host) override;

        /// \copydoc Resource::OnDelete(Ref<Service::Host>)
        void OnDelete(Ref<Service::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64     mDuration;
        UInt32     mFrequency;
        Topology   mTopology;
        Blob       mData;
        Ptr<Codec> mCodec;
    };
}