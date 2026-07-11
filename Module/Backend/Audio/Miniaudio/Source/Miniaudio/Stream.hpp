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

#include "Zyphryon.Audio/Decoder.hpp"
#include <miniaudio.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio::Miniaudio
{
    /// \brief A miniaudio data source backed by a decoder.
    class Stream final : public ma_data_source_base
    {
    public:

        /// \brief Constructs an empty audio stream.
        Stream();

        /// \brief Destroys the audio stream.
        ~Stream();

        /// \brief Binds a decoder to this stream, replacing any previously assigned one.
        ///
        /// \param Decoder The decoder that provides audio data for this stream.
        ZY_INLINE void Setup(Unique<Decoder> Decoder)
        {
            mDecoder = Move(Decoder);
        }

    private:

        /// \brief Implements the data source read callback, decoding audio frames into \p Output.
        ma_result OnRead(Ptr<void> Output, ma_uint64 Count, Ptr<ma_uint64> OutFrames);

        /// \brief Implements the data source seek callback, repositioning the decoder to \p Frame.
        ma_result OnSeek(ma_uint64 Frame);

        /// \brief Implements the data source describe callback, reporting the stream format and channel layout.
        ma_result OnDescribe(
            Ptr<ma_format> OutFormat,
            Ptr<ma_uint32> OutWidth,
            Ptr<ma_uint32> OutFrequency, Ptr<ma_channel> OutLayout, size_t LayoutCount);

        /// \brief Implements the data source cursor callback, reporting the current playback position.
        ma_result OnGetCursor(Ptr<ma_uint64> OutCursor);

        /// \brief Implements the data source length callback, reporting the total number of frames.
        ma_result OnGetLength(Ptr<ma_uint64> OutLength);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Unique<Decoder> mDecoder;
    };
}