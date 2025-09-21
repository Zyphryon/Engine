// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Log.hpp"
#include "Trait.hpp"
#include "SDL3/SDL_timer.h"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a pseudo-random number generator (PRNG).
    ///
    /// It uses the xoshiro256** algorithm for generating high-quality random numbers.
    class Random final
    {
    public:

        /// \brief Constructs a new random number generator with the given seed.
        ///
        /// \param Seed The initial seed value (default is 0).
        ZYPHRYON_INLINE constexpr explicit Random(UInt64 Seed = 0)
        {
            UInt64 Value = Seed;

            mState[0] = SplitMix64(Value);
            mState[1] = SplitMix64(Value);
            mState[2] = SplitMix64(Value);
            mState[3] = SplitMix64(Value);
        }

        /// \brief Generates the next random integer of the specified type.
        ///
        /// \tparam Type The type of the integer to generate.
        /// \return A random integer of the specified type.
        template<typename Type>
        ZYPHRYON_INLINE constexpr Type NextInteger()
            requires(IsInteger<Type>)
        {
            return static_cast<Type>(NextUnsignedInteger64());
        }

        /// \brief Generates a random integer within the specified range.
        ///
        /// \tparam Type   The type of the integer to generate.
        /// \param Minimum The lower bound of the range, inclusive.
        /// \param Maximum The upper bound of the range, inclusive.
        /// \return A random integer within the specified range.
        template<typename Type>
        ZYPHRYON_INLINE constexpr Type NextInteger(Type Minimum, Type Maximum)
            requires(IsInteger<Type>)
        {
            LOG_ASSERT(Minimum < Maximum, "Minimum must be less than Maximum");

            using Unsigned = std::make_unsigned_t<Type>;

            const Unsigned Range = static_cast<Unsigned>(Maximum) - static_cast<Unsigned>(Minimum) + 1;
            const UInt64   Limit = UINT64_MAX - (UINT64_MAX % Range);

            UInt64 Value;
            do
            {
                Value = NextUnsignedInteger64();
            }
            while (Value > Limit);

            return Minimum + static_cast<Type>(Value % Range);
        }

        /// \brief Generates the next random real number of the specified type.
        ///
        /// \tparam Type The type of the real number to generate.
        /// \return A random real number of the specified type.
        template<typename Type>
        ZYPHRYON_INLINE constexpr Type NextReal()
            requires(IsReal<Type>)
        {
            const UInt64 Value = NextUnsignedInteger64();

            if constexpr (IsEqual<Type, Real32>)
            {
                return (Value >> 40) * (1.0f / 16777216.0f);
            }
            else
            {
                return (Value >> 11) * (1.0 / 9007199254740992.0);
            }
        }

        /// \brief Generates a random real number within the specified range.
        ///
        /// \tparam Type The type of the real number to generate.
        /// \param Minimum The lower bound of the range, inclusive.
        /// \param Maximum The upper bound of the range, inclusive.
        /// \return A random real number within the specified range.
        template<typename Type>
        ZYPHRYON_INLINE constexpr Type NextReal(Type Minimum, Type Maximum)
            requires(IsReal<Type>)
        {
            LOG_ASSERT(Minimum < Maximum, "Minimum must be less than Maximum");

            return Minimum + (Maximum - Minimum) * NextReal<Type>();
        }

    public:

        /// \brief Creates a default random number generator seeded with the current time.
        ///
        /// \return A default random number generator.
        ZYPHRYON_INLINE static Ref<Random> Default()
        {
            thread_local Random kDefaultInstance = Random(SDL_GetTicksNS());
            return kDefaultInstance;
        }

    private:

        /// \brief Generates the next random 64-bit unsigned integer.
        ///
        /// \return A random 64-bit unsigned integer.
        ZYPHRYON_INLINE constexpr UInt64 NextUnsignedInteger64()
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
        /// \param Bits  The number of bits to rotate.
        /// \return The rotated value.
        ZYPHRYON_INLINE static constexpr UInt64 RotateLeft(UInt64 Value, UInt32 Bits)
        {
            return (Value << Bits) | (Value >> (64 - Bits));
        }

        /// \brief Generates a 16-bit value using the SplitMix64 algorithm.
        ///
        /// \param State The current state of the generator.
        /// \return A 16-bit value generated by the algorithm.
        ZYPHRYON_INLINE static constexpr UInt16 SplitMix64(Ref<UInt64> State)
        {
            UInt64 Result = (State += 0x9e3779b97f4a7c15);
            Result = (Result ^ (Result >> 30)) * 0xbf58476d1ce4e5b9;
            Result = (Result ^ (Result >> 27)) * 0x94d049bb133111eb;
            return Result ^ (Result >> 31);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mState[4];
    };
}