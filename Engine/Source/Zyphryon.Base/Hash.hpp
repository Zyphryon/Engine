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

#include "Utility.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    namespace Detail
    {
        /// \brief Performs 64x64 -> 128-bit multiplication and stores the low and high halves of the result.
        ///
        /// \param Low  The first operand; receives the low 64 bits of the product.
        /// \param High The second operand; receives the high 64 bits of the product.
        constexpr void Mum(Ref<UInt64> Low, Ref<UInt64> High)
        {
            if consteval
            {
                const UInt64 L0 = (Low & 0xFFFFFFFFull) * (High & 0xFFFFFFFFull);
                const UInt64 M1 = (Low >> 32u)          * (High & 0xFFFFFFFFull);
                const UInt64 M2 = (Low & 0xFFFFFFFFull) * (High >> 32u);
                const UInt64 M0 = (L0 >> 32u) + (M1 & 0xFFFFFFFFull) + (M2 & 0xFFFFFFFFull);

                High = (Low >> 32u) * (High >> 32u) + (M1 >> 32u) + (M2 >> 32u) + (M0 >> 32u);
                Low  = (M0 << 32u) | (L0 & 0xFFFFFFFFull);
            }
            else
            {
#if   defined(ZY_COMPILER_MSVC) && defined(ZY_ARCH_X86_64)
                const UInt64 Result = _umul128(Low, High, &High);
                Low = Result;
#elif defined(ZY_COMPILER_MSVC) && defined(ZY_ARCH_ARM64)
                const UInt64 Result = __umulh(Low, High);
                Low  = Low * High;
                High = Result;
#elif defined(__SIZEOF_INT128__)
                const unsigned __int128 Product = static_cast<unsigned __int128>(Low) * static_cast<unsigned __int128>(High);
                Low  = static_cast<UInt64>(Product);
                High = static_cast<UInt64>(Product >> 64);
#else
                const UInt64 L0 = (Low & 0xFFFFFFFFull) * (High & 0xFFFFFFFFull);
                const UInt64 M1 = (Low >> 32u)          * (High & 0xFFFFFFFFull);
                const UInt64 M2 = (Low & 0xFFFFFFFFull) * (High >> 32u);
                const UInt64 M0 = (L0 >> 32u) + (M1 & 0xFFFFFFFFull) + (M2 & 0xFFFFFFFFull);

                High = (Low >> 32u) * (High >> 32u) + (M1 >> 32u) + (M2 >> 32u) + (M0 >> 32u);
                Low  = (M0 << 32u) | (L0 & 0xFFFFFFFFull);
#endif
            }
        }

        /// \brief Multiplies two 64-bit values and XORs the high and low halves of the 128-bit product.
        ///
        /// \param Left  The left operand; receives the low 64 bits of the product XOR'd with the high 64 bits.
        /// \param Right The right operand; receives the high 64 bits of the product.
        /// \return A 64-bit hash of the product.
        constexpr UInt64 Mix(UInt64 Left, UInt64 Right)
        {
            Mum(Left, Right);
            return Left ^ Right;
        }

        /// \brief Read a value of type \p Type from potentially unaligned memory.
        ///
        /// \param Source The pointer to the raw bytes to load from.
        /// \return A value of type \p Type read from the memory pointed to by \p Source.
        template<typename Output>
        constexpr Output Read(ConstPtr<Char> Source)
        {
            if consteval
            {
                Output Value = 0;

                for (UInt Index = 0; Index < sizeof(Output); ++Index)
                {
                    Value |= static_cast<Output>(static_cast<UInt8>(Source[Index])) << (Index * 8);
                }
                return Value;
            }
            else
            {
                Output Value;
                Blit(AddressOf(Value), sizeof(Output), Source);
                return Value;
            }
        }
    }

    /// \brief Concept for types that implement a custom `Hash()` method returning a value castable to `UInt64`.
    template<typename Type>
    concept Hashable = requires (ConstRef<Type> Value)
    {
        { Value.Hash() } -> IsCastable<UInt64>;
    };

    /// \brief Concept for trivially copyable types that should be hashed byte-by-byte.
    template<typename Type>
    concept IsTriviallyHashable = IsTriviallyCopyable<Type> && (!Hashable<Type>) && (!IsIntegral<Type>);

    /// \brief Computes a hash for the given data using the rapid-hash algorithm.
    ///
    /// \param Data The pointer to the raw bytes to hash.
    /// \param Size The number of bytes to hash.
    /// \return A 64-bit hash of the given byte range.
    constexpr UInt64 Hash(ConstPtr<Char> Data, UInt Size)
    {
        static constexpr UInt64 kSecret[] =
        {
            0x2D358DCCAA6C78A5ULL,
            0x8BB84B93962EACC9ULL,
            0x4B33A62ED433D4A3ULL,
            0x4D5A2DA51DE1AA47ULL,
            0xA0761D6478BD642FULL,
            0xE7037ED1A0B428DBULL,
            0x90ED1765281C388CULL,
            0xAAAAAAAAAAAAAAAAULL
        };

        // Handle empty input.
        if (Size == 0)
        {
            return Detail::Mix(kSecret[2], kSecret[1]);
        }

        // Initialize the seed.
        UInt64 Seed = Detail::Mix(kSecret[2], kSecret[1]);
        UInt64 A    = 0;
        UInt64 B    = 0;

        if (Size <= 16)
        {
            if (Size >= 4)
            {
                Seed ^= Size;

                if (Size >= 8)
                {
                    A = Detail::Read<UInt64>(Data);
                    B = Detail::Read<UInt64>(Data + Size - 8);
                }
                else
                {
                    A = Detail::Read<UInt32>(Data);
                    B = Detail::Read<UInt32>(Data + Size - 4);
                }
            }
            else
            {
                A = static_cast<UInt64>(Data[0]) << 45 | static_cast<UInt64>(Data[Size - 1]);
                B = static_cast<UInt64>(Data[Size >> 1]);
            }
        }
        else
        {
            if (Size > 112)
            {
                UInt64 S1 = Seed, S2 = Seed, S3 = Seed, S4 = Seed, S5 = Seed, S6 = Seed;
                do
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data)      ^ kSecret[0], Detail::Read<UInt64>(Data + 8)   ^ Seed);
                    S1   = Detail::Mix(Detail::Read<UInt64>(Data + 16) ^ kSecret[1], Detail::Read<UInt64>(Data + 24)  ^ S1);
                    S2   = Detail::Mix(Detail::Read<UInt64>(Data + 32) ^ kSecret[2], Detail::Read<UInt64>(Data + 40)  ^ S2);
                    S3   = Detail::Mix(Detail::Read<UInt64>(Data + 48) ^ kSecret[3], Detail::Read<UInt64>(Data + 56)  ^ S3);
                    S4   = Detail::Mix(Detail::Read<UInt64>(Data + 64) ^ kSecret[4], Detail::Read<UInt64>(Data + 72)  ^ S4);
                    S5   = Detail::Mix(Detail::Read<UInt64>(Data + 80) ^ kSecret[5], Detail::Read<UInt64>(Data + 88)  ^ S5);
                    S6   = Detail::Mix(Detail::Read<UInt64>(Data + 96) ^ kSecret[6], Detail::Read<UInt64>(Data + 104) ^ S6);
                    Data += 112;
                    Size -= 112;
                }
                while (Size > 112);

                Seed ^= S1 ^ S6 ^ (S2 ^ S3) ^ (S4 ^ S5);
            }

            if (Size > 16)
            {
                Seed = Detail::Mix(Detail::Read<UInt64>(Data) ^ kSecret[2], Detail::Read<UInt64>(Data + 8) ^ Seed);

                if (Size > 32)
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data + 16) ^ kSecret[2], Detail::Read<UInt64>(Data + 24) ^ Seed);
                }
                if (Size > 48)
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data + 32) ^ kSecret[1], Detail::Read<UInt64>(Data + 40) ^ Seed);
                }
                if (Size > 64)
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data + 48) ^ kSecret[1], Detail::Read<UInt64>(Data + 56) ^ Seed);
                }
                if (Size > 80)
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data + 64) ^ kSecret[2], Detail::Read<UInt64>(Data + 72) ^ Seed);
                }
                if (Size > 96)
                {
                    Seed = Detail::Mix(Detail::Read<UInt64>(Data + 80) ^ kSecret[1], Detail::Read<UInt64>(Data + 88) ^ Seed);
                }
            }

            A = Detail::Read<UInt64>(Data + Size - 16) ^ Size;
            B = Detail::Read<UInt64>(Data + Size - 8);
        }

        A ^= kSecret[1];
        B ^= Seed;
        Detail::Mum(A, B);

        return Detail::Mix(A ^ kSecret[7], B ^ kSecret[1] ^ Size);
    }

    /// \brief Computes a hash for types implementing the Hashable concept.
    ///
    /// \param Data The value to hash.
    /// \return A 64-bit hash of \p Data.
    template<Hashable Type>
    constexpr UInt64 Hash(ConstRef<Type> Data)
    {
        return Data.Hash();
    }

    /// \brief Computes a hash for integral types.
    ///
    /// \param Data The integral value to hash.
    /// \return A 64-bit hash of \p Data.
    template<IsIntegral Type>
    constexpr UInt64 Hash(Type Data)
    {
        return Detail::Mix(static_cast<UInt64>(Data), 0x9E3779B97F4A7C15ULL);
    }

    /// \brief Computes a hash for trivially copyable types.
    ///
    /// \param Data The value to hash.
    /// \return A 64-bit hash of the raw bytes of \p Data.
    template<IsTriviallyHashable Type>
    ZY_INLINE UInt64 Hash(ConstRef<Type> Data)
    {
        return Hash(reinterpret_cast<ConstPtr<Char>>(& Data), sizeof(Type));
    }

    /// \brief Combines the hash values of multiple parameters into a single hash value.
    ///
    /// Each parameter is hashed in order, with the previous hash used as the seed for the next.
    ///
    /// \param Parameters The parameters to hash and combine.
    /// \return A combined 64-bit hash of all parameters.
    template<typename... Arguments>
    constexpr UInt64 HashCombine(AnyRef<Arguments>... Parameters)
    {
        UInt64 Result = 0;
        (..., (Result ^= Hash(Parameters) + 0x9E3779B97F4A7C15ULL + (Result << 6) + (Result >> 2)));
        return Result;
    }

    /// \brief Computes a hash for the type \p Type using its name as a string.
    ///
    /// \tparam Type The type to hash.
    /// \return A 64-bit hash of the name of \p Type.
    template<typename Type>
    consteval UInt64 Hash()
    {
#if defined(ZY_COMPILER_MSVC)
        constexpr ConstPtr<Char> Name = __FUNCSIG__;
#else
        constexpr ConstPtr<Char> Name = __PRETTY_FUNCTION__;
#endif
        UInt Length = 0;

        while (Name[Length] != '\0')
        {
            ++Length;
        }
        return Hash(Name, Length);
    }
}