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

#include "Collector.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Collector::Command> Collector::RadixSort64(Ptr<Command> Input, Ptr<Command> Output, UInt32 Count)
    {
        constexpr UInt32 kRadixBits = 8;
        constexpr UInt32 kRadixSize = 1u << kRadixBits;
        constexpr UInt32 kRadixMask = kRadixSize - 1;
        constexpr UInt32 kPasses = 64 / kRadixBits;

        if (Count < 2)
        {
            return Input;
        }

        UInt32 Histograms[kPasses][kRadixSize]{};
        UInt32 Buckets[kPasses]{};

        for (UInt32 Element = 0; Element < Count; ++Element)
        {
            const UInt64 Key = Input[Element].Order;

            const UInt32 D0 = (Key >> 0) & 0xFF;
            const UInt32 D1 = (Key >> 8) & 0xFF;
            const UInt32 D2 = (Key >> 16) & 0xFF;
            const UInt32 D3 = (Key >> 24) & 0xFF;
            const UInt32 D4 = (Key >> 32) & 0xFF;
            const UInt32 D5 = (Key >> 40) & 0xFF;
            const UInt32 D6 = (Key >> 48) & 0xFF;
            const UInt32 D7 = (Key >> 56) & 0xFF;

            if (Histograms[0][D0]++ == 0)
            {
                ++Buckets[0];
            }
            if (Histograms[1][D1]++ == 0)
            {
                ++Buckets[1];
            }
            if (Histograms[2][D2]++ == 0)
            {
                ++Buckets[2];
            }
            if (Histograms[3][D3]++ == 0)
            {
                ++Buckets[3];
            }
            if (Histograms[4][D4]++ == 0)
            {
                ++Buckets[4];
            }
            if (Histograms[5][D5]++ == 0)
            {
                ++Buckets[5];
            }
            if (Histograms[6][D6]++ == 0)
            {
                ++Buckets[6];
            }
            if (Histograms[7][D7]++ == 0)
            {
                ++Buckets[7];
            }
        }

        for (UInt32 Pass = 0; Pass < kPasses; ++Pass)
        {
            if (Buckets[Pass] == 1)
            {
                continue;
            }

            const Ptr<UInt32> Histogram = Histograms[Pass];

            UInt32 Offset = 0;

            for (UInt32 Digit = 0; Digit < kRadixSize; ++Digit)
            {
                const UInt32 BucketCount = Histogram[Digit];
                Histogram[Digit] = Offset;

                Offset += BucketCount;
            }

            for (UInt32 Index = 0, Shift = Pass * kRadixBits; Index < Count; ++Index)
            {
                const UInt32 Digit = (Input[Index].Order >> Shift) & kRadixMask;
                Output[Histogram[Digit]++] = Input[Index];
            }
            Swap(Input, Output);
        }
        return Input;
    }
}