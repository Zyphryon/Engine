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

#include "Primitive.hpp"
#include <format>
#include <string>
#include <SDL3/SDL_stdinc.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief An owning UTF-8 encoded string used for textual storage and manipulation.
    using Str8       = std::string;

    /// \brief A non-owning, read-only view of a UTF-8 encoded string.
    using ConstStr8  = std::string_view;

    /// \brief An owning UTF-16 encoded string used for textual storage and manipulation.
    using Str16      = std::wstring;

    /// \brief A non-owning, read-only view of a UTF-16 encoded string.
    using ConstStr16 = std::wstring_view;

    /// \brief Converts a UTF-16 encoded string to UTF-8.
    ///
    /// \param Value The UTF-16 encoded string to convert.
    /// \return A UTF-8 encoded string containing the converted text.
    Str8 ConvertUTF16ToUTF8(ConstStr16 Value);

    /// \brief Formats a UTF-8 string using the specified arguments.
    ///
    /// \param Format     The format string, following std::format syntax.
    /// \param Parameters The parameters to substitute into the format string.
    /// \return A formatted UTF-8 string.
    template<typename... Arguments>
    static ConstStr8 Format(ConstStr8 Format, AnyRef<Arguments>... Parameters)
    {
        thread_local Char Buffer[4096];

        const auto Result = std::vformat_to(Buffer, Format, std::make_format_args(Parameters...));
        (* Result) = '\0';

        return ConstStr8(Buffer, std::distance(Buffer, Result));
    }

    /// \brief Iterates over each UTF-8 codepoint in the given text, invoking a callback for each.
    ///
    /// \param Text     The UTF-8 encoded text to iterate over.
    /// \param Callback The callback function to invoke for each codepoint.
    template<typename Function>
    static void IterateUTF8(ConstStr8 Text, AnyRef<Function> Callback)
    {
        ConstPtr<Char> Data      = Text.data();
        UInt           Size      = Text.size();
        UInt32         Codepoint = SDL_StepUTF8(&Data, &Size);

        while (Codepoint != 0)
        {
            Callback(Codepoint);

            Codepoint = SDL_StepUTF8(&Data, &Size);
        }
    }
}