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
    /// \brief Represents a second-order low-pass section applied to one stream of samples.
    class ZY_API Filter final
    {
    public:

        /// \brief The memory one channel carries from one block into the next.
        using State = Array<Real32, 2>;

    public:

        /// \brief The lowest cutoff the section accepts, under which a source is inaudible rather than muffled.
        static constexpr Real32 kMinimumCutoff = 20.0f;

        /// \brief The resonance of the section, placing its two poles on a Butterworth circle.
        static constexpr Real32 kQuality       = 0.70710678f;

        /// \brief The magnitude a channel's memory is dropped under, far beneath anything heard.
        static constexpr Real32 kFloor         = 1.0e-15f;

    public:

        /// \brief Constructs the section for a cutoff at the mixer's sample rate.
        ///
        /// \param Cutoff The cutoff frequency in hertz.
        explicit Filter(Real32 Cutoff);

        /// \brief Filters a block of samples in place, advancing one channel's memory.
        ///
        /// \param State   The channel's memory, carried across blocks so a voice filters continuously.
        /// \param Samples The samples to filter.
        /// \param Count   The number of samples to filter.
        ZY_INLINE void Apply(Ref<State> State, Ptr<Real32> Samples, UInt32 Count) const
        {
            for (UInt32 Index = 0; Index < Count; ++Index)
            {
                const Real32 Input  = Samples[Index];
                const Real32 Output = mNumerator[0] * Input + State[0];

                State[0] = mNumerator[1] * Input - mDenominator[0] * Output + State[1];
                State[1] = mNumerator[2] * Input - mDenominator[1] * Output;

                Samples[Index] = Output;
            }

            // A memory this small is dropped, since left to decay it lingers as a denormal that slows every sample.
            if (Abs(State[0]) < kFloor && Abs(State[1]) < kFloor)
            {
                State[0] = 0.0f;
                State[1] = 0.0f;
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Real32, 3> mNumerator;
        Array<Real32, 2> mDenominator;
    };
}