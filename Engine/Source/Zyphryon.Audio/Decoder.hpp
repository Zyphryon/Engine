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

namespace Audio
{
    /// \brief Abstract interface for decoding audio data into PCM sample frames.
    class Decoder
    {
    public:

        /// \brief Destructs the decoder and releases any associated resources.
        virtual ~Decoder() = default;

        /// \brief Probes the audio data to determine its frequency, stride, and total frame count.
        ///
        /// \param Frequency A pointer to receive the audio frequency in hertz (Hz).
        /// \param Stride    A pointer to receive the audio stride in bytes per frame.
        /// \param Frames    A pointer to receive the total number of audio frames available.
        virtual void Probe(Ptr<UInt32> Frequency, Ptr<UInt32> Stride, Ptr<UInt64> Frames) const = 0;

        /// \brief Seeks to the specified frame index within the audio data.
        ///
        /// \param Frame The zero-based index of the frame to seek to.
        /// \return `true` if the seek operation was successful, `false` otherwise.
        virtual Bool Seek(UInt64 Frame) = 0;

        /// \brief Gets the current frame index within the audio data.
        ///
        /// \return The zero-based index of the current frame position.
        virtual UInt64 Tell() const = 0;

        /// \brief Reads audio data into the provided output buffer, from the current frame position.
        ///
        /// \param Output A span to receive the decoded audio samples as 32-bit floating-point values.
        /// \return The number of frames successfully read into the output buffer.
        virtual UInt64 Read(Span<Real32> Output) = 0;
    };
}