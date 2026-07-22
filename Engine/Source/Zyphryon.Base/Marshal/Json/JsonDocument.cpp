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

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseValue(Text Content, Ref<UInt> Cursor);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static void WriteValue(Ref<Str> Output, ConstRef<JsonValue> Value, Text Indent, UInt Level);

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseString(Text Content, Ref<UInt> Cursor)
    {
        ++Cursor;

        Str Result;

        while (Cursor < Content.GetSize())
        {
            const Char Character = Content[Cursor++];

            if (Character == '"')
            {
                break;
            }
            else if (Character == '\\' && Cursor < Content.GetSize())
            {
                switch (const Char Escaped = Content[Cursor++])
                {
                case '"':
                    Result.Append('"');
                    break;
                case '\\':
                    Result.Append('\\');
                    break;
                case '/':
                    Result.Append('/');
                    break;
                case 'b':
                    Result.Append('\b');
                    break;
                case 'f':
                    Result.Append('\f');
                    break;
                case 'n':
                    Result.Append('\n');
                    break;
                case 'r':
                    Result.Append('\r');
                    break;
                case 't':
                    Result.Append('\t');
                    break;
                case 'u':
                {
                    UInt32 Codepoint = 0;

                    for (UInt Index = 0; Index < 4; ++Index)
                    {
                        if (Cursor >= Content.GetSize())
                        {
                            break;
                        }

                        const Char Hex = Content[Cursor++];
                        Codepoint <<= 4;

                        if (Hex >= '0' && Hex <= '9')
                        {
                            Codepoint |= (Hex - '0');
                        }
                        else if (Hex >= 'a' && Hex <= 'f')
                        {
                            Codepoint |= (Hex - 'a' + 10);
                        }
                        else if (Hex >= 'A' && Hex <= 'F')
                        {
                            Codepoint |= (Hex - 'A' + 10);
                        }
                    }
                    Result.AppendCodepoint(Codepoint);
                }
                break;
                default:
                    Result.Append(Escaped);
                    break;
                }
            }
            else
            {
                Result.Append(Character);
            }
        }
        return JsonValue(Result);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseObject(Text Content, Ref<UInt> Cursor)
    {
        JsonValue Result;
        Ref<JsonValue::Object> Object = Result.SetObject();

        ++Cursor;
        StrSkipWhitespace(Content, Cursor);

        if (Cursor < Content.GetSize() && Content[Cursor] == '}')
        {
            ++Cursor;
        }
        else
        {
            while (Cursor < Content.GetSize())
            {
                StrSkipWhitespace(Content, Cursor);

                if (Content[Cursor] != '"')
                {
                    break;
                }

                JsonValue Key = ParseString(Content, Cursor);

                StrSkipWhitespace(Content, Cursor);

                if (Cursor >= Content.GetSize() || Content[Cursor] != ':')
                {
                    break;
                }

                ++Cursor;

                JsonValue Value = ParseValue(Content, Cursor);

                Object.Assign(Key.GetString(), Move(Value));

                StrSkipWhitespace(Content, Cursor);

                if (Cursor >= Content.GetSize())
                {
                    break;
                }

                if (Content[Cursor] == '}')
                {
                    ++Cursor;
                    break;
                }

                if (Content[Cursor] == ',')
                {
                    ++Cursor;
                }
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseArray(Text Content, Ref<UInt> Cursor)
    {
        JsonValue Result;
        Ref<JsonValue::Array> Array = Result.SetArray();

        ++Cursor;
        StrSkipWhitespace(Content, Cursor);

        if (Cursor < Content.GetSize() && Content[Cursor] == ']')
        {
            ++Cursor;
        }
        else
        {
            while (Cursor < Content.GetSize())
            {
                JsonValue Element = ParseValue(Content, Cursor);
                Array.Append(Move(Element));

                StrSkipWhitespace(Content, Cursor);

                if (Cursor >= Content.GetSize())
                {
                    break;
                }

                if (Content[Cursor] == ']')
                {
                    ++Cursor;
                    break;
                }

                if (Content[Cursor] == ',')
                {
                    ++Cursor;
                }
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseBoolean(Text Content, Ref<UInt> Cursor)
    {
        return JsonValue(StrExtractBool(Content, Cursor));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseNull(Text Content, Ref<UInt> Cursor)
    {
        if (Cursor + 4 <= Content.GetSize()
            && Content[Cursor] == 'n'
            && Content[Cursor + 1] == 'u'
            && Content[Cursor + 2] == 'l'
            && Content[Cursor + 3] == 'l')
        {
            Cursor += 4;
        }
        return JsonValue();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseNumber(Text Content, Ref<UInt> Cursor)
    {
        return JsonValue(StrExtractNumber<Real64>(Content, Cursor));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static JsonValue ParseValue(Text Content, Ref<UInt> Cursor)
    {
        StrSkipWhitespace(Content, Cursor);

        if (Cursor >= Content.GetSize())
        {
            return JsonValue();
        }

        const Char Character = Content[Cursor];

        if (Character == '"')
        {
            return ParseString(Content, Cursor);
        }
        if (Character == '{')
        {
            return ParseObject(Content, Cursor);
        }
        if (Character == '[')
        {
            return ParseArray(Content, Cursor);
        }
        if (Character == 't' || Character == 'f')
        {
            return ParseBoolean(Content, Cursor);
        }
        if (Character == 'n')
        {
            return ParseNull(Content, Cursor);
        }
        if (Character == '-' || (Character >= '0' && Character <= '9'))
        {
            return ParseNumber(Content, Cursor);
        }
        return JsonValue();
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
        if (Value < 0.0)
        {
            Output.Append('-');
        }

        if (Value == static_cast<SInt64>(Value))
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

        for (ConstRef<JsonValue::Object::Pair> Entry: Value)
        {
            if (!First)
            {
                Output.Append(',');
                Output.Append('\n');
            }
            First = false;

            WriteIndent(Output, Indent, Level + 1);
            WriteString(Output, Entry.First);
            Output.Append(':');
            Output.Append(' ');
            WriteValue(Output, Entry.Second, Indent, Level + 1);
        }

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
        UInt Cursor = 0;
        StrSkipWhitespace(Content, Cursor);
        return ParseValue(Content, Cursor);
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