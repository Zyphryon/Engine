// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Hash.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    Type Load(ConstPtr<Byte> Pointer)
    {
        Type Value;
        std::memcpy(& Value, Pointer, sizeof(Type));

        return Value;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Mum(Ref<UInt64> Lower, Ref<UInt64> Upper)
    {
#if defined(ZYPHRYON_COMPILER_MSVC)
        Lower = _umul128(Lower, Upper, &Upper);
#else
        __uint128_t Return = static_cast<__uint128_t>(Lower) * Upper;

        Lower = static_cast<UInt64>(Return);
        Upper = static_cast<UInt64>(Return >> 64);
#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Mix(UInt64 Left, UInt64 Right)
    {
        Mum(Left, Right);
        return Left ^ Right;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Hash(ConstPtr<Byte> Data, UInt32 Size, UInt64 Seed)
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

        // Initialize the seed.
        Seed ^= Mix(Seed ^ kSecret[2], kSecret[1]);

        UInt64 A = 0;
        UInt64 B = 0;

        if (Size <= 16)
        {
            if (Size >= 4)
            {
                Seed ^= Size;

                if (Size >= 8)
                {
                    A = Load<UInt64>(Data);
                    B = Load<UInt64>(Data + Size - 8);
                }
                else
                {
                    A = Load<UInt32>(Data);
                    B = Load<UInt32>(Data + Size - 4);
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
                    Seed = Mix(Load<UInt64>(Data) ^ kSecret[0], Load<UInt64>(Data + 8) ^ Seed);
                    S1   = Mix(Load<UInt64>(Data + 16) ^ kSecret[1], Load<UInt64>(Data + 24) ^ S1);
                    S2   = Mix(Load<UInt64>(Data + 32) ^ kSecret[2], Load<UInt64>(Data + 40) ^ S2);
                    S3   = Mix(Load<UInt64>(Data + 48) ^ kSecret[3], Load<UInt64>(Data + 56) ^ S3);
                    S4   = Mix(Load<UInt64>(Data + 64) ^ kSecret[4], Load<UInt64>(Data + 72) ^ S4);
                    S5   = Mix(Load<UInt64>(Data + 80) ^ kSecret[5], Load<UInt64>(Data + 88) ^ S5);
                    S6   = Mix(Load<UInt64>(Data + 96) ^ kSecret[6], Load<UInt64>(Data + 104) ^ S6);
                    Data += 112;
                    Size -= 112;
                }
                while (Size > 112);

                Seed ^= S1 ^ S6 ^ (S2 ^ S3) ^ (S4 ^ S5);
            }

            if (Size > 16)
            {
                Seed = Mix(Load<UInt64>(Data) ^ kSecret[2], Load<UInt64>(Data + 8) ^ Seed);

                if (Size > 32)
                {
                    Seed = Mix(Load<UInt64>(Data + 16) ^ kSecret[2], Load<UInt64>(Data + 24) ^ Seed);
                }
                if (Size > 48)
                {
                    Seed = Mix(Load<UInt64>(Data + 32) ^ kSecret[1], Load<UInt64>(Data + 40) ^ Seed);
                }
                if (Size > 64)
                {
                    Seed = Mix(Load<UInt64>(Data + 48) ^ kSecret[1], Load<UInt64>(Data + 56) ^ Seed);
                }
                if (Size > 80)
                {
                    Seed = Mix(Load<UInt64>(Data + 64) ^ kSecret[2], Load<UInt64>(Data + 72) ^ Seed);
                }
                if (Size > 96)
                {
                    Seed = Mix(Load<UInt64>(Data + 80) ^ kSecret[1], Load<UInt64>(Data + 88) ^ Seed);
                }
            }

            A = Load<UInt64>(Data + Size - 16) ^ Size;
            B = Load<UInt64>(Data + Size - 8);
        }

        A ^= kSecret[1];
        B ^= Seed;
        Mum(A, B);
        return Mix(A ^ kSecret[7], B ^ kSecret[1] ^ Size);
    }
}