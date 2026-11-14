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

#include "SHA256.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::SHA256()
        : mState   { 0x6A09E667u, 0xBB67AE85u, 0x3C6EF372u, 0xA54FF53Au, 
                     0x510E527Fu, 0x9B05688Cu, 0x1F83D9ABu, 0x5BE0CD19u },
          mPending { },
          mHeld    { 0 },
          mLength  { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SHA256::Update(ConstSpan<Byte> Data)
    {
        mLength += Data.GetSize();

        // What an earlier call left short of a block is filled up first.
        if (mHeld > 0)
        {
            const UInt Room = kBlock - mHeld;
            const UInt Took = (Data.GetSize() < Room ? Data.GetSize() : Room);

            Copy(mPending.GetData() + mHeld, Took, Data.GetData());
            mHeld += static_cast<UInt32>(Took);
            Data   = Data.Slice(Took);

            if (mHeld < kBlock)
            {
                return;
            }
            Compress(mPending.GetData());
        }

        // Whole blocks are worked in from where they lie, and what is left over waits for the next call.
        for (; Data.GetSize() >= kBlock; Data = Data.Slice(kBlock))
        {
            Compress(Data.GetData());
        }
        Copy(mPending.GetData(), Data.GetSize(), Data.GetData());
        mHeld = static_cast<UInt32>(Data.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::Sum SHA256::Finish()
    {
        const UInt64 Bits = mLength * 8;

        // A set bit closes the message, then zeros up to the last eight bytes of a block, which carry its length.
        mPending[mHeld++] = 0x80;

        if (mHeld > kBlock - 8)
        {
            Fill(mPending.GetData() + mHeld, kBlock - mHeld, 0);
            Compress(mPending.GetData());
            mHeld = 0;
        }
        Fill(mPending.GetData() + mHeld, kBlock - 8 - mHeld, 0);

        for (UInt32 Index = 0; Index < 8; ++Index)
        {
            mPending[kBlock - 8 + Index] = static_cast<Byte>(Bits >> (56 - Index * 8));
        }
        Compress(mPending.GetData());

        Sum Output;

        for (UInt32 Index = 0; Index < kSize; ++Index)
        {
            Output[Index] = static_cast<Byte>(mState[Index / 4] >> (24 - (Index % 4) * 8));
        }
        return Output;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SHA256::Compress(ConstPtr<Byte> Block)
    {
        static constexpr Array<UInt32, 64> kRound =
        {
            0x428A2F98u, 0x71374491u, 0xB5C0FBCFu, 0xE9B5DBA5u, 0x3956C25Bu, 0x59F111F1u, 0x923F82A4u, 0xAB1C5ED5u,
            0xD807AA98u, 0x12835B01u, 0x243185BEu, 0x550C7DC3u, 0x72BE5D74u, 0x80DEB1FEu, 0x9BDC06A7u, 0xC19BF174u,
            0xE49B69C1u, 0xEFBE4786u, 0x0FC19DC6u, 0x240CA1CCu, 0x2DE92C6Fu, 0x4A7484AAu, 0x5CB0A9DCu, 0x76F988DAu,
            0x983E5152u, 0xA831C66Du, 0xB00327C8u, 0xBF597FC7u, 0xC6E00BF3u, 0xD5A79147u, 0x06CA6351u, 0x14292967u,
            0x27B70A85u, 0x2E1B2138u, 0x4D2C6DFCu, 0x53380D13u, 0x650A7354u, 0x766A0ABBu, 0x81C2C92Eu, 0x92722C85u,
            0xA2BFE8A1u, 0xA81A664Bu, 0xC24B8B70u, 0xC76C51A3u, 0xD192E819u, 0xD6990624u, 0xF40E3585u, 0x106AA070u,
            0x19A4C116u, 0x1E376C08u, 0x2748774Cu, 0x34B0BCB5u, 0x391C0CB3u, 0x4ED8AA4Au, 0x5B9CCA4Fu, 0x682E6FF3u,
            0x748F82EEu, 0x78A5636Fu, 0x84C87814u, 0x8CC70208u, 0x90BEFFFAu, 0xA4506CEBu, 0xBEF9A3F7u, 0xC67178F2u,
        };

        Array<UInt32, 64> Words;

        // The block is read as sixteen words, highest byte first, and the rest are spun out of those.
        for (UInt32 Index = 0; Index < 16; ++Index)
        {
            const ConstPtr<Byte> Word = Block + Index * 4;

            Words[Index] = (static_cast<UInt32>(Word[0]) << 24) | (static_cast<UInt32>(Word[1]) << 16)
                         | (static_cast<UInt32>(Word[2]) << 8)  |  static_cast<UInt32>(Word[3]);
        }
        for (UInt32 Index = 16; Index < 64; ++Index)
        {
            const UInt32 Near = Words[Index - 15];
            const UInt32 Far  = Words[Index - 2];
            const UInt32 Low  = Revolve(Near, 7) ^ Revolve(Near, 18) ^ (Near >> 3);
            const UInt32 High = Revolve(Far, 17) ^ Revolve(Far, 19) ^ (Far >> 10);

            Words[Index] = Words[Index - 16] + Low + Words[Index - 7] + High;
        }

        Array<UInt32, 8> Work = mState;

        for (UInt32 Index = 0; Index < 64; ++Index)
        {
            const UInt32 Upper  = Revolve(Work[4], 6) ^ Revolve(Work[4], 11) ^ Revolve(Work[4], 25);
            const UInt32 Choice = (Work[4] & Work[5]) ^ (~Work[4] & Work[6]);
            const UInt32 First  = Work[7] + Upper + Choice + kRound[Index] + Words[Index];
            const UInt32 Lower  = Revolve(Work[0], 2) ^ Revolve(Work[0], 13) ^ Revolve(Work[0], 22);
            const UInt32 Most   = (Work[0] & Work[1]) ^ (Work[0] & Work[2]) ^ (Work[1] & Work[2]);
            const UInt32 Second = Lower + Most;

            Work[7] = Work[6];
            Work[6] = Work[5];
            Work[5] = Work[4];
            Work[4] = Work[3] + First;
            Work[3] = Work[2];
            Work[2] = Work[1];
            Work[1] = Work[0];
            Work[0] = First + Second;
        }

        for (UInt32 Index = 0; Index < 8; ++Index)
        {
            mState[Index] += Work[Index];
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void SHA256::Prime(ConstSpan<Byte> Key, Ref<SHA256> Inner, Ref<SHA256> Outer)
    {
        // A key longer than a block is digested down first, and a shorter one is padded with zeros.
        Array<Byte, kBlock> Padded { };

        if (Key.GetSize() > kBlock)
        {
            const Sum Short = Hash(Key);

            Copy(Padded.GetData(), kSize, Short.GetData());
        }
        else
        {
            Copy(Padded.GetData(), Key.GetSize(), Key.GetData());
        }

        Array<Byte, kBlock> Mixed;

        for (UInt32 Index = 0; Index < kBlock; ++Index)
        {
            Mixed[Index] = static_cast<Byte>(Padded[Index] ^ 0x36);
        }
        Inner.Update(Mixed);

        for (UInt32 Index = 0; Index < kBlock; ++Index)
        {
            Mixed[Index] = static_cast<Byte>(Padded[Index] ^ 0x5C);
        }
        Outer.Update(Mixed);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::Sum SHA256::Seal(Ref<SHA256> Inner, SHA256 Outer)
    {
        Outer.Update(Inner.Finish());
        return Outer.Finish();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::Sum SHA256::Hash(ConstSpan<Byte> Data)
    {
        SHA256 Whole;
        Whole.Update(Data);
        return Whole.Finish();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::Sum SHA256::Sign(ConstSpan<Byte> Key, ConstSpan<Byte> Data)
    {
        SHA256 Inner;
        SHA256 Outer;
        Prime(Key, Inner, Outer);

        Inner.Update(Data);
        return Seal(Inner, Outer);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    SHA256::Sum SHA256::Derive(ConstSpan<Byte> Password, ConstSpan<Byte> Salt, UInt32 Rounds)
    {
        static constexpr Array<Byte, 4> kFirst = { 0, 0, 0, 1 };

        // Every round signs under the same password, so the key is taken in once and each round starts from it.
        SHA256 Inner;
        SHA256 Outer;
        Prime(Password, Inner, Outer);

        // The first round signs the salt followed by the number of the block, 1, as four bytes.
        SHA256 Opening = Inner;
        Opening.Update(Salt);
        Opening.Update(kFirst);

        Sum Round  = Seal(Opening, Outer);
        Sum Output = Round;

        for (UInt32 Count = 1; Count < Rounds; ++Count)
        {
            SHA256 Next = Inner;
            Next.Update(Round);

            Round = Seal(Next, Outer);

            for (UInt32 Index = 0; Index < kSize; ++Index)
            {
                Output[Index] ^= Round[Index];
            }
        }
        return Output;
    }
}