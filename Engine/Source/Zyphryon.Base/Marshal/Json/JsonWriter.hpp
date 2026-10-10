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

#include "JsonValue.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    /// \brief Writes a value tree as JSON text, indented or on one line.
    class ZY_API JsonWriter final
    {
    public:

        /// \brief Constructs a writer that indents each level of nesting by the given text.
        ///
        /// \param Indent The text one level of nesting is indented by, where empty writes everything on one line,
        ///               with no line breaks and no space after a colon.
        explicit JsonWriter(Text Indent);

        /// \brief Writes a value and everything it holds.
        ///
        /// \param Value The value to write.
        /// \return The JSON text.
        Str Write(ConstRef<JsonValue> Value);

    private:

        /// \brief Writes a value of any kind at the current level of nesting.
        ///
        /// \param Value The value to write.
        void WriteValue(ConstRef<JsonValue> Value);

        /// \brief Writes a number in at most fifteen significant digits, or `null` if not finite.
        ///
        /// \param Value The number to write.
        void WriteNumber(Real64 Value);

        /// \brief Writes a string in quotes, escaping what JSON requires.
        ///
        /// \param Value The text to write.
        void WriteString(Text Value);

        /// \brief Writes an array with one element per line when indented.
        ///
        /// \param Value The array to write.
        void WriteArray(ConstRef<JsonValue::Array> Value);

        /// \brief Writes an object with one member per line when indented.
        ///
        /// \param Value The object to write.
        void WriteObject(ConstRef<JsonValue::Object> Value);

        /// \brief Starts a new line indented to the current level, unless the writer keeps everything on one line.
        void WriteBreak();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str  mOutput;
        Text mIndent;
        UInt mLevel;
    };
}