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

#include "Zyphryon.Base/Lexical/Algorithm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Format
{
    /// \brief Parsed format placeholder specification controlling output formatting.
    struct Placeholder final
    {
        /// The parameter index for positional formatting.
        UInt8 Parameter = 0;

        /// The minimum field width for the formatted output.
        UInt8 Width     = 0;

        /// The precision for floating-point or maximum width for strings.
        UInt8 Precision = 6;

        /// The padding character used when the output is shorter than the width.
        Char  Padding   = ' ';

        /// The alignment character: '<' for left, '>' for right, '^' for center.
        Char  Alignment = '<';

        /// The sign character: '+' for always show, ' ' for space on positive, '-' for only negative.
        Char  Sign      = '-';

        /// The type character specifying the format type (e.g., 'd', 'x', 'f', 's').
        Char  Type      = 0;

        /// \brief Default constructs an inactive placeholder.
        ZY_INLINE constexpr Placeholder() = default;

        /// \brief Parses a format specification string into a placeholder.
        ///
        /// \param Specification The format specification string (e.g., "0:>+010.2f").
        ZY_INLINE constexpr Placeholder(Text Specification)
        {
            UInt Cursor = 0;

            Parameter = StrExtractNumber<10, UInt8>(Specification, Cursor);

            if (Cursor >= Specification.GetSize())
            {
                return;
            }

            StrConsume(Specification, Cursor, ':');

            if (Cursor >= Specification.GetSize())
            {
                return;
            }

            if (StrConsume(Specification, Cursor, '<')
             || StrConsume(Specification, Cursor, '>')
             || StrConsume(Specification, Cursor, '^'))
            {
                Alignment = Specification[Cursor - 1];
            }

            if (StrConsume(Specification, Cursor, '+')
             || StrConsume(Specification, Cursor, ' '))
            {
                Sign = Specification[Cursor - 1];
            }

            if (StrConsume(Specification, Cursor, '0'))
            {
                Padding = '0';
            }

            Width = StrExtractNumber<10, UInt8>(Specification, Cursor);

            if (StrConsume(Specification, Cursor, '.'))
            {
                Precision = StrExtractNumber<10, UInt8>(Specification, Cursor);
            }

            if (Cursor < Specification.GetSize())
            {
                Type = Specification[Cursor];
            }
        }
    };
}