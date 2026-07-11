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

inline namespace Math
{
    /// \brief Represents a pseudo-random number generator (PRNG).
    ///
    /// It uses the xoshiro256++ algorithm for generating high-quality random numbers.
    class Random final
    {
    public:

        /// \brief Constructs a new random number generator with the given seed.
        ///
        /// \param Seed The initial seed value (default is 0).
        ZY_INLINE constexpr explicit Random(UInt64 Seed = 0)
            : mState { }
        {
            UInt64 Value = Seed;

            mState[0] = SplitMix64(Value);
            mState[1] = SplitMix64(Value);
            mState[2] = SplitMix64(Value);
            mState[3] = SplitMix64(Value);
        }

        /// \brief Generates the next random integer of the specified type.
        ///
        /// \return A random integer of the specified type.
        template<typename Type>
        ZY_INLINE constexpr Type NextInteger()
            requires(IsIntegral<Type>)
        {
            return static_cast<Type>(NextUnsignedInteger64());
        }

        /// \brief Generates a random integer within the specified range.
        ///
        /// \param Minimum The lower bound of the range, inclusive.
        /// \param Maximum The upper bound of the range, inclusive.
        /// \return A random integer within the specified range.
        template<typename Type>
        ZY_INLINE constexpr Type NextInteger(Type Minimum, Type Maximum)
            requires(IsIntegral<Type>)
        {
            ZY_ASSERT(Minimum <= Maximum, "Minimum must be less or equal than Maximum");

            using Unsigned = Unsigned<Type>;

            const Unsigned Range = static_cast<Unsigned>(Maximum) - static_cast<Unsigned>(Minimum) + 1;
            const UInt64   Limit = kMaximum<UInt64> - (kMaximum<UInt64> % Range);

            UInt64 Value;
            do
            {
                Value = NextUnsignedInteger64();
            }
            while (Value > Limit);

            return Minimum + static_cast<Type>(Value % Range);
        }

        /// \brief Generates the next random real number of the specified type in the range [0, 1).
        ///
        /// \return A random real number in the range [0, 1).
        template<typename Type>
        ZY_INLINE constexpr Type NextReal()
            requires(IsReal<Type>)
        {
            const UInt64 Value = NextUnsignedInteger64();

            if constexpr (IsAnyOf<Type, Real32>)
            {
                return static_cast<Real32>(Value >> 40) * (1.0f / 16777216.0f);
            }
            else
            {
                return static_cast<Real64>(Value >> 11) * (1.0 / 9007199254740992.0);
            }
        }

        /// \brief Generates a random real number within the specified range.
        ///
        /// \param Minimum The lower bound of the range, inclusive.
        /// \param Maximum The upper bound of the range, exclusive.
        /// \return A random real number in the range [Minimum, Maximum).
        template<typename Type>
        ZY_INLINE constexpr Type NextReal(Type Minimum, Type Maximum)
            requires(IsReal<Type>)
        {
            ZY_ASSERT(Minimum <= Maximum, "Minimum must be less or equal than Maximum");

            return Minimum + (Maximum - Minimum) * NextReal<Type>();
        }

    private:

        /// \brief Generates the next raw 64-bit unsigned integer using the xoshiro256++ state.
        ///
        /// \return The next 64-bit unsigned integer from the generator state.
        ZY_INLINE constexpr UInt64 NextUnsignedInteger64()
        {
            const UInt64 Result = RotateLeft(mState[0] + mState[3], 23) + mState[0];
            const UInt64 Temp = mState[1] << 17;

            mState[2] ^= mState[0];
            mState[3] ^= mState[1];
            mState[1] ^= mState[2];
            mState[0] ^= mState[3];
            mState[2] ^= Temp;
            mState[3]  = RotateLeft(mState[3], 45);
            return Result;
        }

        /// \brief Rotates the bits of a 64-bit unsigned integer to the left.
        ///
        /// \param Value The value to rotate.
        /// \param Bits  The number of bit positions to rotate left.
        /// \return The rotated value.
        ZY_INLINE static constexpr UInt64 RotateLeft(UInt64 Value, UInt32 Bits)
        {
            return (Value << Bits) | (Value >> (64 - Bits));
        }

        /// \brief Advances a seed state using the SplitMix64 algorithm and returns a mixed output.
        ///
        /// \param State The seed state to advance, modified in-place.
        /// \return The 64-bit pseudo-random output derived from the updated state.
        ZY_INLINE static constexpr UInt64 SplitMix64(Ref<UInt64> State)
        {
            UInt64 Result = (State += 0x9E3779B97F4A7C15);
            Result = (Result ^ (Result >> 30)) * 0xBF58476D1CE4E5B9;
            Result = (Result ^ (Result >> 27)) * 0x94D049BB133111EB;
            return Result ^ (Result >> 31);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mState[4];
    };
}