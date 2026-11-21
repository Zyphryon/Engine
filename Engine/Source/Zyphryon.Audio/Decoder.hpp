// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAudio
{
    /// \brief Defines the interface for decoding audio data into PCM sample frames.
    class ZY_API Decoder
    {
    public:

        /// \brief Constructs a decoder at the start of a stream of the given shape.
        ///
        /// \param Frequency The rate the stream was baked at, in hertz.
        /// \param Stride    The number of interleaved samples in each frame.
        /// \param Frames    The total number of frames the stream holds.
        ZY_INLINE Decoder(UInt32 Frequency, UInt16 Stride, UInt64 Frames)
            : mFrequency { Frequency },
              mStride    { Stride },
              mFrames    { Frames },
              mCursor    { 0 }
        {
        }

        /// \brief Destructs the decoder and releases any associated resources.
        virtual ~Decoder() = default;

        /// \brief Probes the audio data to determine its frequency, stride, and total frame count.
        ///
        /// \param Frequency Receives the audio frequency in hertz.
        /// \param Stride    Receives the number of interleaved samples in each frame.
        /// \param Frames    Receives the total number of frames available.
        ZY_INLINE void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const
        {
            (* Frequency) = mFrequency;
            (* Stride)    = mStride;
            (* Frames)    = mFrames;
        }

        /// \brief Seeks to the specified frame index within the audio data.
        ///
        /// \param Frame The zero-based index of the frame to seek to.
        /// \return `true` if the seek operation was successful, `false` otherwise.
        virtual Bool Seek(UInt64 Frame) = 0;

        /// \brief Gets the current frame index within the audio data.
        ///
        /// \return The zero-based index of the current frame position.
        ZY_INLINE UInt64 Tell() const
        {
            return mCursor;
        }

        /// \brief Reads audio data into the provided output buffer, from the current frame position.
        ///
        /// \param Output Receives the decoded samples as 32-bit floating-point values.
        /// \return The number of frames successfully read into the output buffer.
        virtual UInt64 Read(Span<Real32> Output) = 0;

        /// \brief Advances past a number of frames without producing any samples.
        ///
        /// \param Frames The number of frames to advance past.
        /// \return The number of frames actually advanced, fewer than \p Frames once the stream ended.
        virtual UInt64 Skip(UInt64 Frames) = 0;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32 mFrequency;
        UInt16 mStride;
        UInt64 mFrames;
        UInt64 mCursor;
    };
}