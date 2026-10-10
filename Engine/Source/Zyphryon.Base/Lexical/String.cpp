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

#include "String.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void StrWriteReal(Ref<Str32> Output, Real64 Number, UInt Significant)
    {
        ZY_ASSERT(Significant >= 1 && Significant <= 15, "A real is written in one to fifteen significant digits");

        if (IsNaN(Number))
        {
            Output.Append("NaN");
            return;
        }

        if (Number < 0.0)
        {
            Output.Append('-');
            Number = -Number;
        }

        if (IsInf(Number))
        {
            Output.Append("Inf");
            return;
        }

        const SInt32 Limit = static_cast<SInt32>(Significant);

        // A whole number is written in digits.
        if (Number < Pow10<Real64>(Limit + 1) && Number == static_cast<UInt64>(Number))
        {
            const UInt64 Integer = static_cast<UInt64>(Number);
            Output.AppendInteger(Integer, CountDigits<10>(Integer), 10, true);
            return;
        }

        const SInt32 Lift     = Number < 1.0e-290 ? 100 : 0;
        const Real64 Lifted   = Lift ? Number * 1.0e100 : Number;
        SInt32       Exponent = Log10(Lifted);
        const SInt32 Shift    = Limit - 1 - Exponent;

        // Dividing by a power of ten rather than multiplying by its inverse keeps an exact power exact.
        const Real64 Scaled = Shift >= 0 ? Lifted * Pow10<Real64>(Shift) : Lifted / Pow10<Real64>(-Shift);

        UInt64 Mantissa = static_cast<UInt64>(Scaled + 0.5);
        SInt32 Digits   = Limit;

        // Rounding up the last digit may carry into one more.
        if (Mantissa == Pow10(Limit))
        {
            Mantissa /= 10;
            ++Exponent;
        }
        Exponent -= Lift;

        // Rounded up, the largest doubles would read back as infinity.
        if (Exponent == 308)
        {
            Mantissa = Min<UInt64>(Mantissa, 179769313486231 / Pow10(15 - Limit));
        }

        while (Mantissa % 10 == 0)
        {
            Mantissa /= 10;
            --Digits;
        }

        // Where fixed digits would run long, an exponent keeps the number within what StrExtractNumber reads.
        const Bool   Scientific = Exponent < -5 || Exponent >= Limit;
        const SInt32 Point      = Scientific ? 1 : Exponent + 1;

        if (Point <= 0)
        {
            Output.Append("0.");
            Output.AppendInteger(Mantissa, Digits - Point, 10, true);
        }
        else if (Point < Digits)
        {
            const UInt64 Scale = Pow10(Digits - Point);

            Output.AppendInteger(Mantissa / Scale, Point, 10, true);
            Output.Append('.');
            Output.AppendInteger(Mantissa % Scale, Digits - Point, 10, true);
        }
        else
        {
            Output.AppendInteger(Mantissa * Pow10(Point - Digits), Point, 10, true);
        }

        if (Scientific)
        {
            const SInt32 Power = Abs(Exponent);

            Output.Append('e');

            if (Exponent < 0)
            {
                Output.Append('-');
            }
            Output.AppendInteger(Power, CountDigits<10>(Power), 10, true);
        }
    }
}