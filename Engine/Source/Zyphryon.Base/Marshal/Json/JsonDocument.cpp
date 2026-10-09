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

#include "JsonDocument.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseValue(Text Content, Ref<UInt> Cursor, UInt Depth, Ref<JsonValue> Output, Ref<JsonError> Error);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteValue(Ref<Str> Output, ConstRef<JsonValue> Value, Text Indent, UInt Level);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Fail(Text Content, UInt Cursor, Text Message, Ref<JsonError> Error)
    {
        const Text Before = Content.Slice(0, Min(Cursor, Content.GetSize()));

        Error.Message = Message;
        Error.Line    = static_cast<UInt32>(StrCount(Before, '\n') + 1);
        Error.Column  = static_cast<UInt32>(Before.GetSize() - (StrFindLast(Before, '\n') + 1) + 1);
        Error.Path.Clear();
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void Prepend(Ref<JsonError> Error, Text Step)
    {
        // A key is joined to the step after it by a dot, an index stands right against it.
        if (!Error.Path.IsEmpty() && Error.Path[0] != '[')
        {
            Error.Path.Insert(0, '.');
        }
        Error.Path.Insert(0, Step);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseHex(Text Content, Ref<UInt> Cursor, Ref<UInt32> Output)
    {
        // Exactly four digits follow, so only they are read.
        const Text Digits = Content.Slice(Cursor, Min<UInt>(4, Content.GetSize() - Cursor));
        UInt       Read   = 0;

        Output  = StrExtractNumber<16, UInt32>(Digits, Read);
        Cursor += Read;
        return Read == 4;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseString(Text Content, Ref<UInt> Cursor, Ref<Str> Output, Ref<JsonError> Error)
    {
        const UInt Start = Cursor++;

        while (Cursor < Content.GetSize())
        {
            const Char Character = Content[Cursor++];

            if (Character == '"')
            {
                return true;
            }

            if (static_cast<UInt8>(Character) < 0x20)
            {
                return Fail(Content, Cursor - 1, "A string holds a control character, which must be escaped", Error);
            }

            if (Character != '\\')
            {
                Output.Append(Character);
                continue;
            }

            if (Cursor >= Content.GetSize())
            {
                break;
            }

            switch (Content[Cursor++])
            {
            case '"':
                Output.Append('"');
                break;
            case '\\':
                Output.Append('\\');
                break;
            case '/':
                Output.Append('/');
                break;
            case 'b':
                Output.Append('\b');
                break;
            case 'f':
                Output.Append('\f');
                break;
            case 'n':
                Output.Append('\n');
                break;
            case 'r':
                Output.Append('\r');
                break;
            case 't':
                Output.Append('\t');
                break;
            case 'u':
            {
                UInt32 Codepoint = 0;

                if (!ParseHex(Content, Cursor, Codepoint))
                {
                    return Fail(Content, Cursor, "A \\u escape needs four hexadecimal digits", Error);
                }

                // A character outside the basic plane is written as two escapes, high half first.
                if (Codepoint >= 0xD800 && Codepoint <= 0xDBFF)
                {
                    const Bool Paired = Cursor + 1 < Content.GetSize() && Content[Cursor] == '\\' && Content[Cursor + 1] == 'u';
                    UInt32     Low    = 0;

                    if (Paired)
                    {
                        Cursor += 2;
                    }

                    if (!Paired || !ParseHex(Content, Cursor, Low) || Low < 0xDC00 || Low > 0xDFFF)
                    {
                        return Fail(Content, Cursor, "A \\u escape holds half a surrogate pair", Error);
                    }
                    Codepoint = 0x10000 + ((Codepoint - 0xD800) << 10) + (Low - 0xDC00);
                }
                else if (Codepoint >= 0xDC00 && Codepoint <= 0xDFFF)
                {
                    return Fail(Content, Cursor, "A \\u escape holds half a surrogate pair", Error);
                }
                Output.AppendCodepoint(Codepoint);
                break;
            }
            default:
                return Fail(Content, Cursor - 1, "A string holds an escape JSON does not have", Error);
            }
        }
        return Fail(Content, Start, "A string is never closed", Error);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseNumber(Text Content, Ref<UInt> Cursor, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        const UInt Start = Cursor;
        const UInt First = Content[Cursor] == '-' ? Cursor + 1 : Cursor;

        if (First >= Content.GetSize() || !StrIsDigit(Content[First]))
        {
            return Fail(Content, Start, "A number needs a digit", Error);
        }

        Real64 Value = StrExtractNumber<Real64>(Content, Cursor);

        // An exponent scales the number by a power of ten.
        if (Cursor < Content.GetSize() && (Content[Cursor] | 0x20) == 'e')
        {
            ++Cursor;

            const Bool Down = StrConsume(Content, Cursor, '-');

            if (!Down)
            {
                StrConsume(Content, Cursor, '+');
            }

            if (Cursor >= Content.GetSize() || !StrIsDigit(Content[Cursor]))
            {
                return Fail(Content, Cursor, "A number needs a digit in its exponent", Error);
            }

            const SInt64 Exponent = StrExtractNumber<10, SInt64>(Content, Cursor);
            Value *= Pow(10.0, static_cast<Real64>(Down ? -Exponent : Exponent));
        }

        if (IsInf(Value))
        {
            return Fail(Content, Start, "A number is too large to hold", Error);
        }

        Output = JsonValue(Value);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseWord(Text Content, Ref<UInt> Cursor, Text Word, ConstRef<JsonValue> Value, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        if (!StrConsume(Content, Cursor, Word))
        {
            return Fail(Content, Cursor, "A value is not valid JSON", Error);
        }

        Output = Value;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseObject(Text Content, Ref<UInt> Cursor, UInt Depth, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        Ref<JsonValue::Object> Object = Output.SetObject();

        ++Cursor;
        StrSkipWhitespace(Content, Cursor);

        if (StrConsume(Content, Cursor, '}'))
        {
            return true;
        }

        while (true)
        {
            if (Cursor >= Content.GetSize() || Content[Cursor] != '"')
            {
                return Fail(Content, Cursor, "An object needs a key in quotes", Error);
            }

            Str Key;

            if (!ParseString(Content, Cursor, Key, Error))
            {
                return false;
            }

            StrSkipWhitespace(Content, Cursor);

            if (!StrConsume(Content, Cursor, ':'))
            {
                return Fail(Content, Cursor, "A key needs a ':' after it", Error);
            }

            JsonValue Value;

            if (!ParseValue(Content, Cursor, Depth + 1, Value, Error))
            {
                Prepend(Error, Key);
                return false;
            }
            Object.Assign(Move(Key), Move(Value));

            StrSkipWhitespace(Content, Cursor);

            if (StrConsume(Content, Cursor, '}'))
            {
                return true;
            }

            if (!StrConsume(Content, Cursor, ','))
            {
                return Fail(Content, Cursor, "An object needs a ',' or a '}' after a value", Error);
            }
            StrSkipWhitespace(Content, Cursor);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseArray(Text Content, Ref<UInt> Cursor, UInt Depth, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        Ref<JsonValue::Array> Array = Output.SetArray();

        ++Cursor;
        StrSkipWhitespace(Content, Cursor);

        if (StrConsume(Content, Cursor, ']'))
        {
            return true;
        }

        while (true)
        {
            JsonValue Element;

            if (!ParseValue(Content, Cursor, Depth + 1, Element, Error))
            {
                Str Step;
                Step.Format<"[{0}]">(Array.GetSize());
                Prepend(Error, Step);
                return false;
            }
            Array.Append(Move(Element));

            StrSkipWhitespace(Content, Cursor);

            if (StrConsume(Content, Cursor, ']'))
            {
                return true;
            }

            if (!StrConsume(Content, Cursor, ','))
            {
                return Fail(Content, Cursor, "An array needs a ',' or a ']' after a value", Error);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool ParseValue(Text Content, Ref<UInt> Cursor, UInt Depth, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        StrSkipWhitespace(Content, Cursor);

        if (Cursor >= Content.GetSize())
        {
            return Fail(Content, Cursor, "A value is missing", Error);
        }

        const Char Character = Content[Cursor];

        // Each array or object reads the next level down, so the nesting is bounded before the stack is.
        if ((Character == '{' || Character == '[') && Depth >= JsonDocument::kMaxDepth)
        {
            return Fail(Content, Cursor, "Arrays and objects nest too deeply", Error);
        }

        switch (Character)
        {
        case '"':
        {
            Str Result;

            if (!ParseString(Content, Cursor, Result, Error))
            {
                return false;
            }
            Output = JsonValue(Result);
            return true;
        }
        case '{':
            return ParseObject(Content, Cursor, Depth, Output, Error);
        case '[':
            return ParseArray(Content, Cursor, Depth, Output, Error);
        case 't':
            return ParseWord(Content, Cursor, "true", JsonValue(true), Output, Error);
        case 'f':
            return ParseWord(Content, Cursor, "false", JsonValue(false), Output, Error);
        case 'n':
            return ParseWord(Content, Cursor, "null", JsonValue(), Output, Error);
        default:
            if (Character == '-' || StrIsDigit(Character))
            {
                return ParseNumber(Content, Cursor, Output, Error);
            }
            return Fail(Content, Cursor, "A value is not valid JSON", Error);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteIndent(Ref<Str> Output, Text Indent, UInt Level)
    {
        for (UInt I = 0; I < Level; ++I)
        {
            Output.Append(Indent);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteNumber(Ref<Str> Output, Real64 Value)
    {
        // JSON has no word for a number that is not finite, so it reads back as null rather than breaking the text.
        if (Value != Value || Value > kMaximum<Real64> || Value < -kMaximum<Real64>)
        {
            Output.Append("null");
            return;
        }

        if (Value < 0.0)
        {
            Output.Append('-');
            Value = -Value;
        }

        // Past 2^53 a double cannot hold a fraction anyway, and past the integer range the cast is undefined.
        if (Value < 9.0e15 && Value == static_cast<SInt64>(Value))
        {
            const SInt64 IntPart = Value;
            Output.AppendInteger(IntPart, CountDigits<10>(IntPart), 10, true);
        }
        else
        {
            Output.AppendReal(Value, 12);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteString(Ref<Str> Output, Text Value)
    {
        Output.Append('"');

        for (UInt I = 0; I < Value.GetSize(); ++I)
        {
            switch (const Char Character = Value[I])
            {
            case '"':
                Output.Append('\\');
                Output.Append('"');
                break;
            case '\\':
                Output.Append('\\');
                Output.Append('\\');
                break;
            case '\b':
                Output.Append('\\');
                Output.Append('b');
                break;
            case '\f':
                Output.Append('\\');
                Output.Append('f');
                break;
            case '\n':
                Output.Append('\\');
                Output.Append('n');
                break;
            case '\r':
                Output.Append('\\');
                Output.Append('r');
                break;
            case '\t':
                Output.Append('\\');
                Output.Append('t');
                break;
            default:
                if (static_cast<UInt8>(Character) < 0x20)
                {
                    const Char Hi = (static_cast<UInt8>(Character) >> 4);
                    const Char Lo = (static_cast<UInt8>(Character) & 0x0F);

                    Output.Append("\\u00");
                    Output.Append(Hi < 10 ? ('0' + Hi) : ('a' + Hi - 10));
                    Output.Append(Lo < 10 ? ('0' + Lo) : ('a' + Lo - 10));
                }
                else
                {
                    Output.Append(Character);
                }
                break;
            }
        }

        Output.Append('"');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteArray(Ref<Str> Output, ConstRef<JsonValue::Array> Value, Text Indent, UInt Level)
    {
        if (Value.IsEmpty())
        {
            Output.Append("[]");
            return;
        }

        Output.Append('[');
        Output.Append('\n');

        Bool First = true;

        for (ConstRef<JsonValue> Element: Value)
        {
            if (!First)
            {
                Output.Append(',');
                Output.Append('\n');
            }
            First = false;

            WriteIndent(Output, Indent, Level + 1);
            WriteValue(Output, Element, Indent, Level + 1);
        }

        Output.Append('\n');
        WriteIndent(Output, Indent, Level);
        Output.Append(']');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteObject(Ref<Str> Output, ConstRef<JsonValue::Object> Value, Text Indent, UInt Level)
    {
        if (Value.IsEmpty())
        {
            Output.Append("{}");
            return;
        }

        Output.Append('{');
        Output.Append('\n');

        Bool First = true;

        Value.ForEach([&](ConstRef<Str> Key, ConstRef<JsonValue> Member)
        {
            if (!First)
            {
                Output.Append(',');
                Output.Append('\n');
            }
            First = false;

            WriteIndent(Output, Indent, Level + 1);
            WriteString(Output, Key);
            Output.Append(':');
            Output.Append(' ');
            WriteValue(Output, Member, Indent, Level + 1);
        });

        Output.Append('\n');
        WriteIndent(Output, Indent, Level);
        Output.Append('}');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteValue(Ref<Str> Output, ConstRef<JsonValue> Value, Text Indent, UInt Level)
    {
        if (Value.IsNull())
        {
            Output.Append("null");
        }
        else if (Value.IsBool())
        {
            Output.Append(Value.GetBool() ? "true"_Text : "false"_Text);
        }
        else if (Value.IsNumber())
        {
            WriteNumber(Output, Value.GetNumber<Real64>());
        }
        else if (Value.IsString())
        {
            WriteString(Output, Value.GetString());
        }
        else if (Value.IsArray())
        {
            WriteArray(Output, Value.GetArray(), Indent, Level);
        }
        else if (Value.IsObject())
        {
            WriteObject(Output, Value.GetObject(), Indent, Level);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    JsonValue JsonDocument::Parse(Text Content)
    {
        JsonValue Result;
        JsonError Error;
        Parse(Content, Result, Error);
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonDocument::Parse(Text Content, Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        // The byte order mark some editors save at the start is not part of the text, nor of its first line.
        UInt Mark = 0;
        StrConsume(Content, Mark, "\xEF\xBB\xBF");

        const Text Source = Content.Slice(Mark);
        UInt       Cursor = 0;
        JsonValue  Result;

        if (!ParseValue(Source, Cursor, 0, Result, Error))
        {
            return false;
        }

        StrSkipWhitespace(Source, Cursor);

        if (Cursor < Source.GetSize())
        {
            return Fail(Source, Cursor, "Something follows the value, where the text should end", Error);
        }

        Output = Move(Result);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Str JsonDocument::Dump(ConstRef<JsonValue> Value, Text Indent)
    {
        Str Output;
        WriteValue(Output, Value, Indent, 0);
        return Output;
    }
}