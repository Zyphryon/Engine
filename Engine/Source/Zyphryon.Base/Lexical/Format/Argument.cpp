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

#include "Argument.hpp"
#include "Zyphryon.Base/Lexical/String.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyFormat
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Argument::Write(Output Buffer, Placeholder Properties) const
    {
        switch (mKind)
        {
        case Kind::Signed:
            WriteInteger(Buffer, Properties, mInteger, static_cast<SInt64>(mInteger) < 0);
            break;
        case Kind::Unsigned:
            WriteInteger(Buffer, Properties, mInteger, false);
            break;
        case Kind::Real32:
            WriteReal(Buffer, Properties, static_cast<Real32>(mReal));
            break;
        case Kind::Real64:
            WriteReal(Buffer, Properties, mReal);
            break;
        case Kind::Flag:
            if (Properties.Type == 'b' || Properties.Type == 'B')
            {
                WritePadded(Buffer, Properties, '\0', mInteger ? Text("True") : Text("False"));
            }
            else
            {
                WritePadded(Buffer, Properties, '\0', mInteger ? Text("1") : Text("0"));
            }
            break;
        case Kind::Text:
            WritePadded(Buffer, Properties, '\0', Text(static_cast<ConstPtr<Char>>(mData), mSize));
            break;
        case Kind::Custom:
            mRelay(Buffer, Properties, mData);
            break;
        default:
            break;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Argument::WriteInteger(Output Buffer, Placeholder Properties, UInt64 Number, Bool Negative)
    {
        if (Properties.Type == 'c')
        {
            const Char Character = static_cast<Char>(Number);

            WritePadded(Buffer, Properties, '\0', Text(AddressOf(Character), 1));
            return;
        }

        const UInt64 Magnitude = Negative ? 0 - Number : Number;
        const Char   Prefix    = Negative ? '-' : (Properties.Sign == '-' ? '\0' : Properties.Sign);
        String<64>   Digits;

        switch (Properties.Type)
        {
        case 'x':
            Digits.AppendInteger(Magnitude, CountDigits<16>(Magnitude), 16, false);
            break;
        case 'X':
            Digits.AppendInteger(Magnitude, CountDigits<16>(Magnitude), 16, true);
            break;
        case 'b':
            Digits.AppendInteger(Magnitude, CountDigits<2>(Magnitude), 2, false);
            break;
        case 'B':
            Digits.AppendInteger(Magnitude, CountDigits<2>(Magnitude), 2, true);
            break;
        default:
            Digits.AppendInteger(Magnitude, CountDigits<10>(Magnitude), 10, true);
            break;
        }

        WritePadded(Buffer, Properties, Prefix, Digits);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    void Argument::WriteReal(Output Buffer, Placeholder Properties, Type Number)
    {
        const Char Prefix = Number < 0 ? '-' : (Properties.Sign == '-' ? '\0' : Properties.Sign);
        String<64> Digits;

        if (Properties.Type == 'f')
        {
            Digits.AppendReal(Abs(Number), Properties.Precision);
        }
        else
        {
            Digits.AppendReal(Abs(Number));
        }

        WritePadded(Buffer, Properties, Prefix, Digits);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Argument::WritePadded(Output Buffer, Placeholder Properties, Char Prefix, Text Content)
    {
        const UInt Required = (Prefix ? 1 : 0) + Content.GetSize();
        const UInt Count    = Properties.Width > Required ? Properties.Width - Required : 0;
        const Bool Zeros    = Properties.Padding == '0';
        UInt       Left     = Count;

        if (!Zeros && Properties.Alignment == '<')
        {
            Left = 0;
        }
        else if (!Zeros && Properties.Alignment == '^')
        {
            Left = Count / 2;
        }

        String<kMaximum<UInt8>> Padding;
        Padding.Append(Properties.Padding, Count);

        // Zeros stand between the sign and the digits, and any other padding before the sign.
        if (Zeros && Prefix)
        {
            Buffer.Append(Prefix);
        }

        if (Left > 0)
        {
            Buffer.Append(Padding.Slice(0, Left));
        }

        if (!Zeros && Prefix)
        {
            Buffer.Append(Prefix);
        }

        Buffer.Append(Content);

        if (Count > Left)
        {
            Buffer.Append(Padding.Slice(0, Count - Left));
        }
    }
}