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

#include "Processor.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyFormat
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Processor::Write(Output Buffer, Text Pattern, ConstSpan<Argument> Parameters)
    {
        UInt       Cursor = 0;
        UInt       Offset = 0;
        const UInt Size   = Pattern.GetSize();

        while (Cursor < Size)
        {
            const SInt Braces = StrFindAny(Pattern, "{}", Cursor);

            if (Braces < 0)
            {
                break;
            }

            Cursor = static_cast<UInt>(Braces);

            if (Cursor > Offset)
            {
                Buffer.Append(Pattern.Slice(Offset, Cursor - Offset));
            }

            if (Pattern[Cursor] == '{')
            {
                if (Cursor + 1 < Size && Pattern[Cursor + 1] == '{')
                {
                    Buffer.Append('{');
                    Cursor += 2;
                    Offset  = Cursor;
                    continue;
                }

                const Text Specification = StrExtractText(Pattern, ++Cursor, '}');

                if (!StrConsume(Pattern, Cursor, '}') || Specification.IsEmpty())
                {
                    Buffer.Append(Pattern[Cursor - 1]);
                }
                else if (const Placeholder Properties(Specification); Properties.Parameter < Parameters.GetSize())
                {
                    Parameters[Properties.Parameter].Write(Buffer, Properties);
                }

                Offset = Cursor;
            }
            else
            {
                Buffer.Append('}');

                if (Cursor + 1 < Size && Pattern[Cursor + 1] == '}')
                {
                    Cursor += 2;
                }
                else
                {
                    ++Cursor;
                }
                Offset = Cursor;
            }
        }

        if (Offset < Size)
        {
            Buffer.Append(Pattern.Slice(Offset, Size - Offset));
        }
    }
}