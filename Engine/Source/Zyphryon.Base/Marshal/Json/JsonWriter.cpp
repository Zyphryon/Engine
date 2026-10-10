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

#include "JsonWriter.hpp"
#include "JsonParser.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    JsonWriter::JsonWriter(Text Indent)
        : mIndent { Indent },
          mLevel  { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Str JsonWriter::Write(ConstRef<JsonValue> Value)
    {
        mOutput.Clear();
        mLevel = 0;

        WriteValue(Value);
        return Move(mOutput);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteValue(ConstRef<JsonValue> Value)
    {
        if (Value.IsNull())
        {
            mOutput.Append("null");
        }
        else if (Value.IsBool())
        {
            mOutput.Append(Value.GetBool() ? "true"_Text : "false"_Text);
        }
        else if (Value.IsNumber())
        {
            WriteNumber(Value.GetNumber<Real64>());
        }
        else if (Value.IsString())
        {
            WriteString(Value.GetString());
        }
        else if (Value.IsArray())
        {
            WriteArray(Value.GetArray());
        }
        else if (Value.IsObject())
        {
            WriteObject(Value.GetObject());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteNumber(Real64 Value)
    {
        if (IsNaN(Value) || IsInf(Value))
        {
            mOutput.Append("null");
        }
        else
        {
            mOutput.AppendReal(Value);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteString(Text Value)
    {
        UInt Run = 0;

        mOutput.Append('"');

        for (UInt Index = 0; Index < Value.GetSize(); ++Index)
        {
            const Char Character = Value[Index];

            // What needs no escape is copied in runs, between the characters that do.
            if (static_cast<UInt8>(Character) >= 0x20 && Character != '"' && Character != '\\')
            {
                continue;
            }

            mOutput.Append(Value.Slice(Run, Index - Run));
            Run = Index + 1;

            if (const SInt Short = StrFind(JsonParser::kEscapeChars, Character); Short != -1)
            {
                mOutput.Append('\\');
                mOutput.Append(JsonParser::kEscapeNames[Short]);
            }
            else
            {
                const UInt8 Code = static_cast<UInt8>(Character);

                mOutput.Append("\\u00");
                mOutput.Append("0123456789abcdef"[Code >> 4]);
                mOutput.Append("0123456789abcdef"[Code & 0x0F]);
            }
        }

        mOutput.Append(Value.Slice(Run));
        mOutput.Append('"');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteArray(ConstRef<JsonValue::Array> Value)
    {
        if (Value.IsEmpty())
        {
            mOutput.Append("[]");
            return;
        }

        mOutput.Append('[');
        ++mLevel;

        for (UInt Index = 0; Index < Value.GetSize(); ++Index)
        {
            if (Index > 0)
            {
                mOutput.Append(',');
            }
            WriteBreak();
            WriteValue(Value[Index]);
        }

        --mLevel;
        WriteBreak();
        mOutput.Append(']');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteObject(ConstRef<JsonValue::Object> Value)
    {
        if (Value.IsEmpty())
        {
            mOutput.Append("{}");
            return;
        }

        // Indented text puts a space after each key's colon, text on one line does not.
        const Text Colon = mIndent.IsEmpty() ? ":"_Text : ": "_Text;
        Bool       First = true;

        mOutput.Append('{');
        ++mLevel;

        Value.ForEach([&](ConstRef<Str> Key, ConstRef<JsonValue> Member)
        {
            if (!First)
            {
                mOutput.Append(',');
            }
            First = false;

            WriteBreak();
            WriteString(Key);
            mOutput.Append(Colon);
            WriteValue(Member);
        });

        --mLevel;
        WriteBreak();
        mOutput.Append('}');
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonWriter::WriteBreak()
    {
        if (mIndent.IsEmpty())
        {
            return;
        }

        mOutput.Append('\n');

        for (UInt Level = 0; Level < mLevel; ++Level)
        {
            mOutput.Append(mIndent);
        }
    }
}