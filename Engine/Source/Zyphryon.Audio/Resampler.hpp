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
    /// \brief Converts interleaved PCM between two sample rates with per-lane linear interpolation.
    class Resampler final
    {
    public:

        /// \brief Maximum interleaved stride (samples per frame) the interpolation window can hold.
        static constexpr UInt16 kMaxStride = 8;

    public:

        /// \brief Constructs an identity resampler that performs no conversion.
        ZY_INLINE Resampler()
            : mStep     { 1.0 },
              mOutput   { 0 },
              mSource   { 0 },
              mStride   { 0 },
              mActive   { false },
              mPrimed   { false },
              mPrevious { },
              mNext     { }
        {
        }

        /// \brief Constructs a resampler converting from a source rate to a target rate.
        ///
        /// \param Source The sample rate of the incoming frames, in Hz.
        /// \param Target The sample rate the output frames are produced at, in Hz.
        /// \param Stride The number of interleaved samples per frame.
        ZY_INLINE Resampler(UInt32 Source, UInt32 Target, UInt16 Stride)
            : mStep     { static_cast<Real64>(Source) / Target },
              mOutput   { 0 },
              mSource   { 0 },
              mStride   { Stride },
              mActive   { Source != Target },
              mPrimed   { false },
              mPrevious { },
              mNext     { }
        {
            ZY_ASSERT(Stride > 0 && Stride <= kMaxStride, "Stride is out of range");
        }

        /// \brief Checks whether the resampler actually changes the sample rate.
        ///
        /// \return `true` when the source and target rates differ, `false` for an identity pass-through.
        ZY_INLINE Bool IsActive() const
        {
            return mActive;
        }

        /// \brief Gets the conversion ratio expressed as source frames advanced per output frame.
        ///
        /// \return The number of source frames consumed for each output frame produced.
        ZY_INLINE Real64 GetStep() const
        {
            return mStep;
        }

        /// \brief Maps an output-domain frame index onto the source frame it reads from.
        ///
        /// \param Frame The output frame index at the target rate.
        /// \return The corresponding source frame index at the source rate.
        ZY_INLINE UInt64 GetSource(UInt64 Frame) const
        {
            return static_cast<UInt64>(Frame * mStep);
        }

        /// \brief Gets the index of the next output frame the streaming path will produce.
        ///
        /// \return The current output-domain cursor, in frames.
        ZY_INLINE UInt64 Tell() const
        {
            return mOutput;
        }

        /// \brief Repositions the streaming cursor so the next \ref Generate resumes at the given output frame.
        ///
        /// \param Frame The output frame index to resume producing from.
        ZY_INLINE void Reset(UInt64 Frame)
        {
            mOutput = Frame;
            mSource = GetSource(Frame);
            mPrimed = false;
        }

        /// \brief Streams output frames, pulling source frames on demand through a provider.
        ///
        /// \param Output   The destination buffer; its size is the maximum number of output frames to produce.
        /// \param Limit    The total output-frame length of the stream; production stops once the cursor reaches it.
        /// \param Provider A callable that writes one interleaved source frame and returns `false` at end-of-stream.
        /// \return The number of output frames written into \p Output.
        template<typename Callable>
        ZY_INLINE UInt64 Generate(Span<Real32> Output, UInt64 Limit, AnyRef<Callable> Provider)
        {
            const Ptr<Real32> Target   = Output.GetData();
            const UInt64      Capacity = Output.GetSize();
            UInt64            Produced = 0;

            // Seed both endpoints of the interpolation window; a single-frame stream mirrors its only sample.
            if (!mPrimed)
            {
                if (!Provider(mPrevious))
                {
                    return 0;
                }
                if (!Provider(mNext))
                {
                    Copy(mNext, mPrevious);
                }
                mPrimed = true;
            }

            while (Produced < Capacity && mOutput < Limit)
            {
                const Real64 Position = mOutput * mStep;
                const UInt64 Base     = static_cast<UInt64>(Position);
                const Real32 Fraction = static_cast<Real32>(Position - Base);

                // Walk the source forward until the window brackets the requested position. On end-of-stream the
                // last frame is held in place, which only tints the final fractional sample.
                while (mSource < Base)
                {
                    Copy(mPrevious, mNext);
                    Provider(mNext);

                    ++mSource;
                }

                for (UInt16 Element = 0; Element < mStride; ++Element)
                {
                    const Real32 Source = mPrevious[Element];
                    Target[Produced * mStride + Element] = Source + (mNext[Element] - Source) * Fraction;
                }
                ++Produced;
                ++mOutput;
            }
            return Produced;
        }

    public:

        /// \brief Estimates how many output frames a source of the given length yields at a target rate.
        ///
        /// \param Frames The number of source frames at the source rate.
        /// \param Source The sample rate of the source, in Hz.
        /// \param Target The sample rate to convert to, in Hz.
        /// \return The number of output frames produced at the target rate.
        ZY_INLINE static UInt64 Estimate(UInt64 Frames, UInt32 Source, UInt32 Target)
        {
            return Source == Target ? Frames : static_cast<UInt64>(Frames * static_cast<Real64>(Target) / Source);
        }

        /// \brief Converts an entire interleaved buffer to the target sample rate in a single call.
        ///
        /// \param Input    The interleaved source samples.
        /// \param Stride   The number of interleaved samples per frame.
        /// \param Frames   The number of source frames in \p Input.
        /// \param Source   The sample rate of \p Input, in Hz.
        /// \param Target   The sample rate to convert to, in Hz.
        /// \param Produced Receives the number of output frames written into the returned buffer.
        /// \return A newly allocated buffer holding the resampled interleaved PCM.
        ZY_INLINE static Blob Convert(ConstSpan<Real32> Input, UInt16 Stride, UInt64 Frames, UInt32 Source, UInt32 Target, Ref<UInt64> Produced)
        {
            const UInt64 Total  = Estimate(Frames, Source, Target);
            Blob         Result = Blob::Allocate<Real32>(Total * Stride);

            UInt64 Cursor = 0;

            const auto Reader = [&](Ptr<Real32> Frame) -> Bool
            {
                if (Cursor >= Frames)
                {
                    return false;
                }
                for (UInt16 Element = 0; Element < Stride; ++Element)
                {
                    Frame[Element] = Input[Cursor * Stride + Element];
                }
                ++Cursor;
                return true;
            };

            Resampler State(Source, Target, Stride);
            Produced = State.Generate(Span(Result.GetData<Real32>(), Total), Total, Reader);
            return Result;
        }

    private:

        /// \brief TODO_DOC
        ZY_INLINE void Copy(Ptr<Real32> Destination, ConstPtr<Real32> Source) const
        {
            for (UInt16 Element = 0; Element < mStride; ++Element)
            {
                Destination[Element] = Source[Element];
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real64 mStep;
        UInt64 mOutput;
        UInt64 mSource;
        UInt16 mStride;
        Bool   mActive;
        Bool   mPrimed;
        Real32 mPrevious[kMaxStride];
        Real32 mNext[kMaxStride];
    };
}
