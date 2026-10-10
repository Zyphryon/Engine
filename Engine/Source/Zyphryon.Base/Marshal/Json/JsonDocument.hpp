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

#include "JsonError.hpp"
#include "JsonValue.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Provides JSON parsing and serialization utilities.
    class ZY_API JsonDocument final
    {
    public:

        /// \brief Parses JSON text into a value structure.
        ///
        /// \param Content The JSON text to parse, which may open with a UTF-8 byte order mark.
        /// \return The parsed root value, or null if parsing fails.
        static JsonValue Parse(Text Content);

        /// \brief Parses JSON text into a value structure, saying why and where when it fails.
        ///
        /// \param Content The JSON text to parse, which may open with a UTF-8 byte order mark.
        /// \param Output  Receives the parsed root value, left untouched when parsing fails.
        /// \param Error   Receives why and where parsing failed, left untouched when it succeeds.
        /// \return `true` if the whole text was one valid value, `false` otherwise.
        static Bool Parse(Text Content, Ref<JsonValue> Output, Ref<JsonError> Error);

        /// \brief Serializes a JSON value to text.
        ///
        /// \param Value  The value to serialize.
        /// \param Indent The string used for indentation (default is two spaces).
        /// \return The serialized JSON text with formatting.
        static Str Dump(ConstRef<JsonValue> Value, Text Indent = "  ");
    };
}