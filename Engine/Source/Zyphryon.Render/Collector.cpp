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
        ZY_PROFILE_SCOPE("RadixSort64");

        constexpr UInt32 kRadixBits = 8;
        constexpr UInt32 kRadixSize = 1u << kRadixBits;
        constexpr UInt32 kRadixMask = kRadixSize - 1;
        constexpr UInt32 kPasses    = 64 / kRadixBits;

        if (Count < 2)
        {
            return Input;
        }

        UInt32 Histograms[kPasses][kRadixSize]{};

        for (UInt32 Element = 0; Element < Count; ++Element)
        {
            const UInt64 Key = Input[Element].Order;

            ++Histograms[0][(Key >>  0) & kRadixMask];
            ++Histograms[1][(Key >>  8) & kRadixMask];
            ++Histograms[2][(Key >> 16) & kRadixMask];
            ++Histograms[3][(Key >> 24) & kRadixMask];
            ++Histograms[4][(Key >> 32) & kRadixMask];
            ++Histograms[5][(Key >> 40) & kRadixMask];
            ++Histograms[6][(Key >> 48) & kRadixMask];
            ++Histograms[7][(Key >> 56) & kRadixMask];
        }

        UInt32 Executed = 0;

        for (UInt32 Pass = 0; Pass < kPasses; ++Pass)
        {
            const UInt32      Shift     = Pass * kRadixBits;
            const Ptr<UInt32> Histogram = Histograms[Pass];

            if (Histogram[(Input[0].Order >> Shift) & kRadixMask] == Count)
            {
                continue;
            }
            ++Executed;

            UInt32 Offset = 0;

            for (UInt32 Digit = 0; Digit < kRadixSize; ++Digit)
            {
                const UInt32 BucketCount = Histogram[Digit];
                Histogram[Digit] = Offset;

                Offset += BucketCount;
            }

            for (UInt32 Index = 0; Index < Count; ++Index)
            {
                const UInt32 Digit = (Input[Index].Order >> Shift) & kRadixMask;
                Output[Histogram[Digit]++] = Input[Index];
            }
            Swap(Input, Output);
        }

        ZY_PROFILE_VALUE("Render.Sort.Commands", static_cast<SInt64>(Count));
        ZY_PROFILE_VALUE("Render.Sort.Passes",   static_cast<SInt64>(Executed));

        return Input;
    }
}