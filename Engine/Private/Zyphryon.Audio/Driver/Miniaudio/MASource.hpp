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

#include "Zyphryon.Audio/Track.hpp"
#include <miniaudio.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    /// \brief Data source implementation Miniaudio audio driver.
    class MASource final : public ma_data_source_base
    {
    public:

        /// \brief Constructor, initializes the data source.
        MASource();

        /// \brief Destructor, cleans up the data source.
        ~MASource();

        /// \bried Loads a track into the source.
        ///
        /// \param Track The track to load.
        ZYPHRYON_INLINE void Load(ConstTracker<Track> Track)
        {
            mTrack   = Track;
            mDecoder = Track->Decode();
        }

    private:

        /// \brief OnRead callback implementation.
        ma_result OnRead(Ptr<void> Output, ma_uint64 Count, Ptr<ma_uint64> OutFrames);

        /// \brief OnSeek callback implementation.
        ma_result OnSeek(ma_uint64 Frame);

        /// \brief OnDescribe callback implementation.
        ma_result OnDescribe(
            Ptr<ma_format> OutFormat,
            Ptr<ma_uint32> OutWidth,
            Ptr<ma_uint32> OutFrequency, Ptr<ma_channel> OutLayout, size_t LayoutCount);

        /// \brief OnGetCursor callback implementation.
        ma_result OnGetCursor(Ptr<ma_uint64> OutCursor);

        /// \brief OnGetLength callback implementation.
        ma_result OnGetLength(Ptr<ma_uint64> OutLength);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tracker<Track>   mTrack;
        Tracker<Decoder> mDecoder;
    };
}