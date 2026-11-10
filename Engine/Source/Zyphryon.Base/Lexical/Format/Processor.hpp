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

#include "Argument.hpp"
#include "Zyphryon.Base/Container/Array.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyFormat
{
    /// \brief Represents the static processor that formats arguments into an output according to a pattern.
    class Processor final
    {
    public:

        /// \brief Formats arguments into the output using the given pattern.
        ///
        /// \param Buffer     The output to write to.
        /// \param Pattern    The format pattern containing literal text and placeholders.
        /// \param Parameters The arguments to substitute into the pattern.
        template<typename... Arguments>
        ZY_INLINE static void Format(Output Buffer, Text Pattern, AnyRef<Arguments>... Parameters)
        {
            const Array<Argument, sizeof...(Arguments)> Values { Argument(Parameters)... };

            Write(Buffer, Pattern, Values);
        }

        /// \brief Writes the pattern's literal text into the output, dropping its placeholders.
        ///
        /// \param Buffer  The output to write to.
        /// \param Pattern The format pattern containing literal text and placeholders.
        ZY_INLINE static void Format(Output Buffer, Text Pattern)
        {
            Write(Buffer, Pattern, ConstSpan<Argument>());
        }

    private:

        /// \brief Writes a pattern into the output, each placeholder as the argument at its position.
        ///
        /// \param Buffer     The output to write to.
        /// \param Pattern    The format pattern containing literal text and placeholders.
        /// \param Parameters The arguments by position, where a placeholder past the last one writes nothing.
        ZY_API static void Write(Output Buffer, Text Pattern, ConstSpan<Argument> Parameters);
    };
}