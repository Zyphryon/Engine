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
    /// \brief Defines the interface for audio data decoders.
    class Decoder : public Trackable<Decoder>
    {
    public:

        /// \brief Ensures derived class can be destroyed polymorphically.
        virtual ~Decoder() = default;

        /// \brief Seeks to a specific frame in the audio stream.
        ///
        /// \param Frame The target frame to seek to.
        /// \return `true` if the seek operation was successful, otherwise `false`.
        virtual Bool Seek(UInt64 Frame) = 0;

        /// \brief Gets the current frame position in the audio stream.
        ///
        /// \return The current frame position.
        virtual UInt64 Tell() const = 0;

        /// \brief Reads decoded audio samples into the provided output buffer.
        ///
        /// \param Output The buffer to store the decoded audio samples.
        /// \param Count  The number of samples to read.
        /// \return The actual number of samples read.
        virtual UInt64 Read(Ptr<Real32> Output, UInt64 Count) = 0;
    };
}