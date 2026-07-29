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

#include "Tween.hpp"
#include "Zyphryon.Math/Bezier.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Determines how a track interpolates between adjacent samples.
    enum class Interpolation : UInt8
    {
        Step,    ///< Hold the previous sample's value (no interpolation).
        Linear,  ///< Linearly interpolate between adjacent samples.
        Cubic,   ///< Smoothly interpolate with a Catmull-Rom spline through the neighboring samples.
    };

    /// \brief Where a moment in time falls within a track: the samples bracketing it, and how far between them.
    struct Cursor final
    {
        /// The index of the sample at or before the time.
        UInt   Lower = 0;

        /// The index of the sample after it, held at \ref Lower once the time reaches the track's end.
        UInt   Upper = 0;

        /// How far the time sits between the two samples, in the range `[0, 1]`.
        Real32 Delta = 0.0f;
    };

    /// \brief A time-ordered sequence of values sampled to produce a value at any point in time.
    template<typename Type>
    class Track final
    {
    public:

        /// \brief Constructs an empty track with linear interpolation and no fixed cadence.
        ZY_INLINE Track()
            : mRate          { 0.0f },
              mInterpolation { Interpolation::Linear }
        {
        }

        /// \brief Appends a sample at an explicit time; samples must be added in ascending time order.
        ///
        /// \param Time  The sample's time, in seconds.
        /// \param Value The sample's value.
        ZY_INLINE void Add(Real64 Time, AnyRef<Type> Value)
        {
            ZY_ASSERT(!IsUniform(), "A track on a fixed cadence takes its times from the cadence");
            ZY_ASSERT(mTimes.IsEmpty() || Time >= mTimes[mTimes.GetSize() - 1],
                "Samples must be added in ascending time order");

            mTimes.Append(Time);
            mValues.Append(Forward<Type>(Value));
        }

        /// \brief Appends a sample at the next slot of the track's fixed cadence.
        ///
        /// \param Value The sample's value.
        ZY_INLINE void Add(AnyRef<Type> Value)
        {
            ZY_ASSERT(IsUniform(), "A track without a cadence needs a time for every sample");

            mValues.Append(Forward<Type>(Value));
        }

        /// \brief Puts the track on a fixed cadence, discarding any times it was carrying.
        ///
        /// \param Rate The number of samples per second, or `0` to return to explicit times.
        ZY_INLINE void SetRate(Real32 Rate)
        {
            ZY_ASSERT(Rate >= 0.0f, "A cadence cannot run backwards");

            mRate = Rate;
            mTimes.Clear();
        }

        /// \brief Sets the interpolation mode.
        ///
        /// \param Mode The interpolation mode to apply when sampling.
        ZY_INLINE void SetInterpolation(Interpolation Mode)
        {
            mInterpolation = Mode;
        }

        /// \brief Replaces the track's samples with a block already laid out in ascending order.
        ///
        /// \note One allocation and one copy, rather than an append per sample.
        ///
        /// \param Values The first of \p Count samples, contiguous.
        /// \param Count  The number of samples.
        ZY_INLINE void SetValues(ConstPtr<Type> Values, UInt Count)
        {
            ZY_ASSERT(IsUniform() || mTimes.GetSize() == Count,
                "A track carrying its own times needs one for every sample");

            mValues.Clear();
            mValues.Advance(Count);

            if (Count > 0)
            {
                Blit(mValues.GetData(), Count * sizeof(Type), Values);
            }
        }

        /// \brief Checks whether the track carries any sample at all.
        ///
        /// \return `true` when the track is empty, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mValues.IsEmpty();
        }

        /// \brief Checks whether the track's samples fall on a fixed cadence.
        ///
        /// \return `true` when the track stores a rate instead of per-sample times, otherwise `false`.
        ZY_INLINE Bool IsUniform() const
        {
            return mRate > 0.0f;
        }

        /// \brief Checks whether the track holds one value for the whole clip.
        ///
        /// \return `true` when the track carries exactly one sample, otherwise `false`.
        ZY_INLINE Bool IsConstant() const
        {
            return mValues.GetSize() == 1;
        }

        /// \brief Gets the number of samples the track carries.
        ///
        /// \return The sample count.
        ZY_INLINE UInt GetSize() const
        {
            return mValues.GetSize();
        }

        /// \brief Gets the track's cadence.
        ///
        /// \return The samples per second, or `0` when the track stores explicit times.
        ZY_INLINE Real32 GetRate() const
        {
            return mRate;
        }

        /// \brief Gets the track's values.
        ///
        /// \return A read-only view over the values, in ascending time order.
        ZY_INLINE ConstSpan<Type> GetValues() const
        {
            return mValues;
        }

        /// \brief Gets the track's sample times.
        ///
        /// \return A read-only view over the times, or an empty view when the track is on a cadence.
        ZY_INLINE ConstSpan<Real64> GetTimes() const
        {
            return mTimes;
        }

        /// \brief Gets the interpolation mode applied when sampling.
        ///
        /// \return The interpolation mode.
        ZY_INLINE Interpolation GetInterpolation() const
        {
            return mInterpolation;
        }

        /// \brief Gets the track's duration (the time of its last sample).
        ///
        /// \return The duration in seconds, or `0` when the track is empty.
        ZY_INLINE Real64 GetDuration() const
        {
            if (mValues.IsEmpty())
            {
                return 0.0;
            }

            if (IsUniform())
            {
                return static_cast<Real64>(mValues.GetSize() - 1) / static_cast<Real64>(mRate);
            }
            return mTimes[mTimes.GetSize() - 1];
        }

        /// \brief Finds where a moment in time falls within the track, clamping to the endpoints outside its range.
        ///
        /// \param Time The time to locate, in seconds.
        /// \return The bracketing samples and the fraction between them, or a zeroed cursor when empty.
        ZY_INLINE Cursor Locate(Real64 Time) const
        {
            Cursor Result;

            if (mValues.IsEmpty())
            {
                return Result;
            }

            const UInt Last = mValues.GetSize() - 1;

            UInt   Index;
            Real32 Delta;

            if (IsUniform())
            {
                const Real64 Scaled = Clamp(Time * static_cast<Real64>(mRate), 0.0, static_cast<Real64>(Last));

                Index = static_cast<UInt>(Scaled);
                Delta = static_cast<Real32>(Scaled - static_cast<Real64>(Index));
            }
            else
            {
                Search(Time, Index, Delta);
            }

            Result.Lower = Index;
            Result.Upper = Index < Last ? Index + 1 : Last;
            Result.Delta = Delta;
            return Result;
        }

        /// \brief Samples the track at a cursor obtained from this track or falling back to a value when is empty.
        ///
        /// \param Where    The position within the track to blend at.
        /// \param Fallback The value to return when the track carries no sample.
        /// \return The interpolated value, or \p Fallback when the track is empty.
        ZY_INLINE Type Sample(ConstRef<Cursor> Where, ConstRef<Type> Fallback) const
        {
            if (mValues.IsEmpty())
            {
                return Fallback;
            }

            if (IsConstant())
            {
                return mValues.GetFront();
            }

            ZY_ASSERT(Where.Upper < mValues.GetSize(), "Cursor does not belong to this track");

            if (mInterpolation == Interpolation::Step || Where.Lower == Where.Upper)
            {
                return mValues[Where.Lower];
            }

            ConstRef<Type> Previous = mValues[Where.Lower];
            ConstRef<Type> Next     = mValues[Where.Upper];

            if constexpr (IsSlerpable<Type>)
            {
                return Type::Slerp(Previous, Next, Where.Delta);
            }
            else
            {
                if constexpr (IsSplinable<Type>)
                {
                    if (mInterpolation == Interpolation::Cubic)
                    {
                        const UInt Last = mValues.GetSize() - 1;

                        ConstRef<Type> Before = mValues[Where.Lower     >= 1    ? Where.Lower - 1 : 0];
                        ConstRef<Type> After  = mValues[Where.Upper + 1 <= Last ? Where.Upper + 1 : Last];
                        return AnyBezier<Type>::FromCatmullRom(Before, Previous, Next, After).Point(Where.Delta);
                    }
                }

                if constexpr (IsLerpable<Type>)
                {
                    return Type::Lerp(Previous, Next, Where.Delta);
                }
                else
                {
                    return Lerp<Type>(Previous, Next, Where.Delta);
                }
            }
        }

        /// \brief Samples the track at the given time, falling back to a value when is empty.
        ///
        /// \param Time     The time to sample at, in seconds.
        /// \param Fallback The value to return when the track carries no sample.
        /// \return The interpolated value, or \p Fallback when the track is empty.
        ZY_INLINE Type Sample(Real64 Time, ConstRef<Type> Fallback) const
        {
            return Sample(Locate(Time), Fallback);
        }

    private:

        /// \brief The span below which the search stops halving and simply walks.
        static constexpr UInt kThreshold = 8;

        /// \brief Finds the sample preceding a time, for a track carrying its own times.
        ///
        /// \param Time  The time to search for, in seconds.
        /// \param Index Receives the index of the preceding sample.
        /// \param Delta Receives how far the time sits between that sample and the next.
        void Search(Real64 Time, Ref<UInt> Index, Ref<Real32> Delta) const
        {
            const UInt Last = mValues.GetSize() - 1;

            if (Time <= mTimes[0])
            {
                Index = 0;
                Delta = 0.0f;
                return;
            }

            if (Time >= mTimes[Last])
            {
                Index = Last;
                Delta = 0.0f;
                return;
            }

            // Halving narrows a long track in a handful of steps rather than one per sample, but each step
            // branches unpredictably and a mispredict costs more than several scanned entries.
            UInt Lower = 1;
            UInt Upper = Last;

            while (Upper - Lower > kThreshold)
            {
                const UInt Middle = Lower + (Upper - Lower) / 2;

                if (mTimes[Middle] < Time)
                {
                    Lower = Middle + 1;
                }
                else
                {
                    Upper = Middle;
                }
            }

            while (Lower < Upper && mTimes[Lower] < Time)
            {
                ++Lower;
            }

            Index = Lower - 1;
            Delta = static_cast<Real32>((Time - mTimes[Index]) / (mTimes[Lower] - mTimes[Index]));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Type>   mValues;
        Sequence<Real64> mTimes;
        Real32           mRate;
        Interpolation    mInterpolation;
    };
}