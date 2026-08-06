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

#include "Zyphryon.Base/Container/Span.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Represents a key that signs a run of bytes, and tells its own signature from anybody else's.
    class Signer final
    {
    public:

        /// \brief Constructs a signer under a key of zero, whose signatures anybody can work out for themselves.
        ZY_INLINE constexpr Signer()
            : mKey { 0, 0 }
        {
        }

        /// \brief Constructs a signer under a key already chosen.
        ///
        /// \param First  The first half of the key.
        /// \param Second The second half of the key.
        ZY_INLINE constexpr Signer(UInt64 First, UInt64 Second)
            : mKey { First, Second }
        {
        }

        /// \brief Signs an array of bytes,.
        ///
        /// \param Data The bytes to sign, which are read but never kept.
        /// \return The signature those bytes carry under this key.
        constexpr UInt64 Sign(ConstSpan<Byte> Data) const
        {
            constexpr auto Revolve = [](UInt64 Value, UInt32 Count)
            {
                return (Value << Count) | (Value >> (64 - Count));
            };

            UInt64 V0 = 0x736F6D6570736575ull ^ mKey[0];
            UInt64 V1 = 0x646F72616E646F6Dull ^ mKey[1];
            UInt64 V2 = 0x6C7967656E657261ull ^ mKey[0];
            UInt64 V3 = 0x7465646279746573ull ^ mKey[1];

            const auto Round = [&]
            {
                V0 += V1;
                V1  = Revolve(V1, 13);
                V1 ^= V0;
                V0  = Revolve(V0, 32);

                V2 += V3;
                V3  = Revolve(V3, 16);
                V3 ^= V2;

                V0 += V3;
                V3  = Revolve(V3, 21);
                V3 ^= V0;

                V2 += V1;
                V1  = Revolve(V1, 17);
                V1 ^= V2;
                V2  = Revolve(V2, 32);
            };

            const UInt Size  = Data.GetSize();
            const UInt Whole = Size - (Size & 7);

            for (UInt Step = 0; Step < Whole; Step += 8)
            {
                UInt64 Block = 0;

                for (UInt Part = 0; Part < 8; ++Part)
                {
                    Block |= static_cast<UInt64>(Data[Step + Part]) << (Part * 8);
                }

                V3 ^= Block;

                Round();
                Round();

                V0 ^= Block;
            }

            // The last block carries the length in its top byte, so two runs alike but for how much followed them
            // cannot come out the same.
            UInt64 Tail = static_cast<UInt64>(Size) << 56;

            for (UInt Step = Whole; Step < Size; ++Step)
            {
                Tail |= static_cast<UInt64>(Data[Step]) << ((Step - Whole) * 8);
            }

            V3 ^= Tail;

            Round();
            Round();

            V0 ^= Tail;
            V2 ^= 0xFF;

            Round();
            Round();
            Round();
            Round();

            return V0 ^ V1 ^ V2 ^ V3;
        }

        /// \brief Signs whatever a value is laid out as in memory.
        ///
        /// \param Value The value to sign, taken as the bytes it occupies.
        /// \return The signature that value carries under this key.
        template<typename Type>
        ZY_INLINE constexpr UInt64 Sign(ConstRef<Type> Value) const
            requires(!IsContiguous<Type>)
        {
            return Sign(ConstSpan(reinterpret_cast<ConstPtr<Byte>>(AddressOf(Value)), sizeof(Type)));
        }

        /// \brief Checks a signature against what this key would have made of the same bytes.
        ///
        /// \param Data  The bytes, or the value, the signature speaks for.
        /// \param Value The signature to weigh.
        /// \return `true` when the signature is this key's own, `false` otherwise.
        template<typename Type>
        ZY_INLINE constexpr Bool Verify(ConstRef<Type> Data, UInt64 Value) const
        {
            return Sign(Data) == Value;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mKey[2];
    };
}