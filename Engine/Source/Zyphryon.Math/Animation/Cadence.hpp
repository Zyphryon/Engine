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

#include "Track.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief A cadence shared by several tracks, so one \ref Locate serves all of them.
    class Cadence final
    {
    public:

        /// \brief Constructs an empty cadence, carrying neither a rate nor a length.
        ZY_INLINE Cadence()
            : mRate { 0.0f },
              mSize { 0 }
        {
        }

        /// \brief Constructs a cadence over a fixed number of samples laid out at a fixed rate.
        ///
        /// \param Rate The number of samples per second the tracks are laid out on.
        /// \param Size The number of samples each of those tracks carries.
        ZY_INLINE Cadence(Real32 Rate, UInt32 Size)
            : mRate { Rate },
              mSize { Size }
        {
        }

        /// \brief Checks whether the cadence describes a timeline that can be sampled at all.
        ///
        /// \return `true` when the cadence carries a rate and a length, otherwise `false`.
        ZY_INLINE Bool IsValid() const
        {
            return mSize > 0 && mRate > 0.0f;
        }

        /// \brief Gets the rate the cadence is laid out on.
        ///
        /// \return The number of samples per second.
        ZY_INLINE Real32 GetRate() const
        {
            return mRate;
        }

        /// \brief Gets the number of samples the cadence spans.
        ///
        /// \return The sample count each track on the cadence carries.
        ZY_INLINE UInt32 GetSize() const
        {
            return mSize;
        }

        /// \brief Checks whether a track can be sampled with this cadence's cursor.
        ///
        /// \param Value The track to test.
        /// \return `true` when \ref Locate's cursor is valid for the track, otherwise `false`.
        template<typename Type>
        ZY_INLINE Bool Agrees(ConstRef<Track<Type>> Value) const
        {
            return Value.IsEmpty() || Value.IsConstant() || (Value.GetRate() == mRate && Value.GetSize() == mSize);
        }

        /// \brief Finds where a moment in time falls on the cadence, clamping to the endpoints outside its range.
        ///
        /// \param Time The time to locate, in seconds.
        /// \return The bracketing samples and the fraction between them.
        ZY_INLINE Cursor Locate(Real64 Time) const
        {
            const UInt   Last   = mSize > 0 ? mSize - 1 : 0;
            const Real64 Scaled = Clamp(Time * static_cast<Real64>(mRate), 0.0, static_cast<Real64>(Last));
            const UInt   Index  = static_cast<UInt>(Scaled);

            Cursor Result;
            Result.Lower = Index;
            Result.Upper = Index < Last ? Index + 1 : Last;
            Result.Delta = static_cast<Real32>(Scaled - static_cast<Real64>(Index));
            return Result;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32 mRate;
        UInt32 mSize;
    };
}
