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

namespace Tool::Baker::Audio
{
    /// \brief One decoded sound, held as interleaved floating-point frames.
    class Sample final
    {
    public:

        /// \brief Constructs an empty sample.
        ZY_INLINE Sample()
            : mFrequency { 0 },
              mStride    { 0 },
              mFrames    { 0 }
        {
        }

        /// \brief Constructs a sample over decoded frames.
        ///
        /// \param Frames    The interleaved frames, as 32-bit floating-point values.
        /// \param Frequency The rate the frames are clocked at, in hertz.
        /// \param Stride    The number of channels per frame.
        /// \param Count     The number of frames the block holds.
        ZY_INLINE Sample(AnyRef<Blob> Frames, UInt32 Frequency, UInt16 Stride, UInt64 Count)
            : mSamples   { Move(Frames) },
              mFrequency { Frequency },
              mStride    { Stride },
              mFrames    { Count }
        {
        }

        /// \brief Gets the interleaved frames.
        ///
        /// \return A view over the frames, as 32-bit floating-point values.
        ZY_INLINE ConstSpan<Real32> GetSamples() const
        {
            return ConstSpan(mSamples.GetData<Real32>(), mFrames * mStride);
        }

        /// \brief Gets the rate the frames are clocked at.
        ///
        /// \return The frequency, in hertz.
        ZY_INLINE UInt32 GetFrequency() const
        {
            return mFrequency;
        }

        /// \brief Gets the number of channels per frame.
        ///
        /// \return The stride.
        ZY_INLINE UInt16 GetStride() const
        {
            return mStride;
        }

        /// \brief Gets the number of frames held.
        ///
        /// \return The frame count.
        ZY_INLINE UInt64 GetFrames() const
        {
            return mFrames;
        }

        /// \brief Checks whether the sample holds no frames.
        ///
        /// \return `true` when the sample is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mFrames == 0 || mStride == 0;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Blob   mSamples;
        UInt32 mFrequency;
        UInt16 mStride;
        UInt64 mFrames;
    };
}
