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
    /// \brief Represents a parser that reads one JSON text into a value tree, reusing its scratch from value to value.
    class ZY_API JsonParser final
    {
    public:

        /// The deepest arrays and objects may nest, so a hostile file cannot exhaust the stack.
        static constexpr UInt kMaxDepth    = 128;

        /// The characters a short escape names after its backslash.
        static constexpr Text kEscapeNames = "\"\\/bfnrt";

        /// The characters each short escape stands for, in the same order as the names.
        static constexpr Text kEscapeChars = "\"\\/\b\f\n\r\t";

    public:

        /// \brief Constructs a parser over a JSON text, skipping the UTF-8 byte order mark it may open with.
        ///
        /// \param Content The JSON text to read, which must outlive the parser.
        explicit JsonParser(Text Content);

        /// \brief Reads the whole text as one value.
        ///
        /// \param Output Receives the parsed tree, left untouched when parsing fails.
        /// \param Error  Receives why and where parsing failed, left untouched when it succeeds.
        /// \return `true` if the whole text was one valid value, `false` otherwise.
        Bool Parse(Ref<JsonValue> Output, Ref<JsonError> Error);

    private:

        /// \brief Reads the value at the cursor, after any whitespace before it.
        ///
        /// \param Output Receives the value, left untouched when it is not valid.
        /// \return `true` if a valid value was read, `false` otherwise.
        Bool ParseValue(Ref<JsonValue> Output);

        /// \brief Reads the object that opens at the cursor into a table of its exact size.
        ///
        /// \param Output Receives the object, left untouched when it is not valid.
        /// \return `true` if a valid object was read, `false` otherwise.
        Bool ParseObject(Ref<JsonValue> Output);

        /// \brief Reads the array that opens at the cursor into a sequence of its exact size.
        ///
        /// \param Output Receives the array, left untouched when it is not valid.
        /// \return `true` if a valid array was read, `false` otherwise.
        Bool ParseArray(Ref<JsonValue> Output);

        /// \brief Reads the string that opens at the cursor.
        ///
        /// \param Output Receives the text, valid until the next string is read.
        /// \return `true` if a valid string was read, `false` otherwise.
        Bool ParseString(Ref<Text> Output);

        /// \brief Reads the number that starts at the cursor.
        ///
        /// \param Output Receives the number, left untouched when it is not valid.
        /// \return `true` if a valid number was read, `false` otherwise.
        Bool ParseNumber(Ref<JsonValue> Output);

        /// \brief Reads one of the literal words `true`, `false` or `null` at the cursor.
        ///
        /// \param Word   The word expected at the cursor.
        /// \param Value  The value the word stands for.
        /// \param Output Receives \p Value.
        /// \return `true` if the word was there, `false` otherwise.
        Bool ParseWord(Text Word, ConstRef<JsonValue> Value, Ref<JsonValue> Output);

        /// \brief Reads the four hexadecimal digits of a `\u` escape at the cursor.
        ///
        /// \param Output Receives the code unit the digits spell.
        /// \return `true` if four digits were read, `false` otherwise.
        Bool ParseHex(Ref<UInt32> Output);

        /// \brief Records why the parse stopped, with the line and column of the given position.
        ///
        /// \param Position The offset into the content where the problem is.
        /// \param Message  The reason, as one sentence.
        /// \return Always `false`, so a failing read can return it directly.
        Bool Fail(UInt Position, Text Message);

        /// \brief Adds the key or index being read to the front of the error's path, as the failure unwinds.
        ///
        /// \param Step The key of a member, or the bracketed index of an element.
        void Prepend(Text Step);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Text                              mContent;
        UInt                              mCursor;
        UInt                              mDepth;
        JsonError                         mError;
        Sequence<JsonValue::Object::Pair> mMembers;
        Sequence<JsonValue>               mElements;
        Str                               mEscaped;
    };
}