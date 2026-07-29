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

#include "Bit.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief An IEEE 754 binary16 value, used wherever half the storage matters more than the precision.
    class Half final
    {
    public:

        /// \brief Constructs a half holding positive zero.
        ZY_INLINE constexpr Half()
            : mBits { 0 }
        {
        }

        /// \brief Constructs a half from a single-precision value, rounding to nearest with ties to even.
        ///
        /// \param Value The value to narrow.
        ZY_INLINE explicit constexpr Half(Real32 Value)
            : mBits { Encode(Value) }
        {
        }

        /// \brief Gets the raw binary16 bit pattern.
        ///
        /// \return The encoded bits, suitable for writing straight into a buffer.
        ZY_INLINE constexpr UInt16 GetBits() const
        {
            return mBits;
        }

        /// \brief Widens this value to single precision.
        ///
        /// \return The exact single-precision equivalent.
        ZY_INLINE constexpr operator Real32() const
        {
            return Decode(mBits);
        }

    public:

        /// \brief Builds a half directly from a binary16 bit pattern.
        ///
        /// \param Bits The encoded bits, as read from a buffer.
        /// \return The half holding \p Bits.
        ZY_INLINE static constexpr Half FromBits(UInt16 Bits)
        {
            Half Result;
            Result.mBits = Bits;
            return Result;
        }

        /// \brief Gets the largest finite value a half can hold.
        ///
        /// \return 65504, the largest representable magnitude.
        ZY_INLINE static constexpr Half Largest()
        {
            return FromBits(0x7BFF);
        }

        /// \brief Gets the smallest positive normalized value a half can hold.
        ///
        /// \return The smallest normal, 2^-14.
        ZY_INLINE static constexpr Half Smallest()
        {
            return FromBits(0x0400);
        }

    private:

        /// \brief Narrows a single-precision value into a binary16 bit pattern.
        ///
        /// \param Value The value to encode.
        /// \return The encoded bits.
        ZY_INLINE static constexpr UInt16 Encode(Real32 Value)
        {
            const UInt32 Bits     = CastBit<UInt32>(Value);
            const UInt32 Sign     = (Bits >> 16) & 0x8000u;
            const UInt32 Biased   = (Bits >> 23) & 0x00FFu;
            const UInt32 Mantissa = Bits & 0x007FFFFFu;

            const SInt32 Exponent = static_cast<SInt32>(Biased) - 112;

            if (Exponent <= 0)
            {
                if (Exponent < -10)
                {
                    return static_cast<UInt16>(Sign);
                }

                const UInt32 Shift = static_cast<UInt32>(14 - Exponent);
                const UInt32 Whole = Mantissa | 0x00800000u;

                return static_cast<UInt16>(Sign | ((Whole >> Shift) + ((Whole >> (Shift - 1)) & 1u)));
            }

            if (Exponent >= 31)
            {
                if (Biased == 0x00FFu)
                {
                    return static_cast<UInt16>(Sign | 0x7C00u | (Mantissa ? 0x0200u : 0u));
                }
                return static_cast<UInt16>(Sign | 0x7BFFu);
            }

            const UInt32 Rounded = Mantissa + 0x00000FFFu + ((Mantissa >> 13) & 1u);
            return static_cast<UInt16>(Sign | ((static_cast<UInt32>(Exponent) << 10) + (Rounded >> 13)));
        }

        /// \brief Widens a binary16 bit pattern into a single-precision value.
        ///
        /// \param Bits The encoded bits.
        /// \return The exact single-precision equivalent.
        ZY_INLINE static constexpr Real32 Decode(UInt16 Bits)
        {
            const UInt32 Sign     = (Bits & 0x8000u) << 16;
            const UInt32 Exponent = (Bits >> 10) & 0x001Fu;
            const UInt32 Mantissa = Bits & 0x03FFu;

            if (Exponent == 0)
            {
                const Real32 Value = static_cast<Real32>(Mantissa) * 0x1p-24f;
                return (Bits & 0x8000u) ? -Value : Value;
            }

            if (Exponent == 31)
            {
                return CastBit<Real32>(Sign | 0x7F800000u | (Mantissa << 13));
            }
            return CastBit<Real32>(Sign | ((Exponent + 112u) << 23) | (Mantissa << 13));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt16 mBits;
    };
}