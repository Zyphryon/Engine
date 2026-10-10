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

#include "JsonParser.hpp"
#include "Zyphryon.Base/Lexical/Algorithm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    JsonParser::JsonParser(Text Content)
        : mContent { Content },
          mCursor  { 0 },
          mDepth   { 0 }
    {
        // The byte order mark some editors save at the start is not part of the text, nor of its first line.
        UInt Mark = 0;
        StrConsume(Content, Mark, "\xEF\xBB\xBF");

        mContent = Content.Slice(Mark);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::Parse(Ref<JsonValue> Output, Ref<JsonError> Error)
    {
        JsonValue Result;

        if (ParseValue(Result))
        {
            StrSkipWhitespace(mContent, mCursor);

            if (mCursor == mContent.GetSize())
            {
                Output = Move(Result);
                return true;
            }
            Fail(mCursor, "Something follows the value, where the text should end");
        }

        Error = Move(mError);
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseValue(Ref<JsonValue> Output)
    {
        StrSkipWhitespace(mContent, mCursor);

        if (mCursor >= mContent.GetSize())
        {
            return Fail(mCursor, "A value is missing");
        }

        switch (const Char Character = mContent[mCursor])
        {
        case '"':
        {
            Text Result;

            if (!ParseString(Result))
            {
                return false;
            }
            Output.SetString(Result);
            return true;
        }
        case '{':
            return ParseObject(Output);
        case '[':
            return ParseArray(Output);
        case 't':
            return ParseWord("true", JsonValue(true), Output);
        case 'f':
            return ParseWord("false", JsonValue(false), Output);
        case 'n':
            return ParseWord("null", JsonValue(), Output);
        default:
            if (Character == '-' || StrIsDigit(Character))
            {
                return ParseNumber(Output);
            }
            return Fail(mCursor, "A value is not valid JSON");
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseObject(Ref<JsonValue> Output)
    {
        // Each array or object reads the next level down, so the nesting is bounded before the stack is.
        if (mDepth >= kMaxDepth)
        {
            return Fail(mCursor, "Arrays and objects nest too deeply");
        }

        ++mCursor;
        StrSkipWhitespace(mContent, mCursor);

        if (StrConsume(mContent, mCursor, '}'))
        {
            Output.SetObject();
            return true;
        }

        // The members gather at the top of a stack shared by every open object, which a nested one may grow.
        const UInt Base = mMembers.GetSize();

        ++mDepth;

        do
        {
            StrSkipWhitespace(mContent, mCursor);

            if (mCursor >= mContent.GetSize() || mContent[mCursor] != '"')
            {
                return Fail(mCursor, "An object needs a key in quotes");
            }

            Text Name;

            if (!ParseString(Name))
            {
                return false;
            }

            StrSkipWhitespace(mContent, mCursor);

            if (!StrConsume(mContent, mCursor, ':'))
            {
                return Fail(mCursor, "A key needs a ':' after it");
            }

            // The key is copied out before the value is read, as an escaped one lives in the scratch the value reuses.
            Str       Key(Name);
            JsonValue Value;

            if (!ParseValue(Value))
            {
                Prepend(Key);
                return false;
            }
            mMembers.Append(Move(Key), Move(Value));

            StrSkipWhitespace(mContent, mCursor);
        }
        while (StrConsume(mContent, mCursor, ','));

        if (!StrConsume(mContent, mCursor, '}'))
        {
            return Fail(mCursor, "An object needs a ',' or a '}' after a value");
        }

        --mDepth;

        // A key read twice keeps its first place and its last value.
        const UInt             Count  = mMembers.GetSize() - Base;
        Ref<JsonValue::Object> Object = Output.SetObject();
        Object.Reserve(Count);

        for (UInt Index = Base; Index < Base + Count; ++Index)
        {
            Object.Assign(Move(mMembers[Index].First), Move(mMembers[Index].Second));
        }
        mMembers.Remove(Base, Count);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseArray(Ref<JsonValue> Output)
    {
        if (mDepth >= kMaxDepth)
        {
            return Fail(mCursor, "Arrays and objects nest too deeply");
        }

        ++mCursor;
        StrSkipWhitespace(mContent, mCursor);

        if (StrConsume(mContent, mCursor, ']'))
        {
            Output.SetArray();
            return true;
        }

        // The elements gather at the top of a stack shared by every open array, which a nested one may grow.
        const UInt Base  = mElements.GetSize();
        UInt       Count = 0;

        ++mDepth;

        do
        {
            JsonValue Element;

            if (!ParseValue(Element))
            {
                Str Step;
                Step.Format<"[{0}]">(Count);
                Prepend(Step);
                return false;
            }
            mElements.Append(Move(Element));
            ++Count;

            StrSkipWhitespace(mContent, mCursor);
        }
        while (StrConsume(mContent, mCursor, ','));

        if (!StrConsume(mContent, mCursor, ']'))
        {
            return Fail(mCursor, "An array needs a ',' or a ']' after a value");
        }

        --mDepth;

        Ref<JsonValue::Array> Array = Output.SetArray();
        Array.Reserve(Count);

        for (UInt Index = Base; Index < Base + Count; ++Index)
        {
            Array.Append(Move(mElements[Index]));
        }
        mElements.Remove(Base, Count);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseString(Ref<Text> Output)
    {
        const UInt Start = ++mCursor;
        const UInt Size  = mContent.GetSize();

        // Most strings hold no escape, so their text is the slice between the quotes, found without copying.
        for (; mCursor < Size; ++mCursor)
        {
            const UInt8 Character = static_cast<UInt8>(mContent[mCursor]);

            if (Character == '"' || Character == '\\' || Character < 0x20)
            {
                break;
            }
        }

        if (mCursor < Size && mContent[mCursor] == '"')
        {
            Output = mContent.Slice(Start, mCursor - Start);
            ++mCursor;
            return true;
        }

        // Otherwise the rest is decoded a character at a time into the scratch, after what was already scanned.
        mEscaped = mContent.Slice(Start, mCursor - Start);

        while (mCursor < Size)
        {
            const Char Character = mContent[mCursor++];

            if (Character == '"')
            {
                Output = mEscaped;
                return true;
            }

            if (static_cast<UInt8>(Character) < 0x20)
            {
                return Fail(mCursor - 1, "A string holds a control character, which must be escaped");
            }

            if (Character != '\\')
            {
                mEscaped.Append(Character);
                continue;
            }

            if (mCursor >= Size)
            {
                break;
            }

            const Char Code  = mContent[mCursor++];
            const SInt Short = StrFind(kEscapeNames, Code);

            if (Short != -1)
            {
                mEscaped.Append(kEscapeChars[Short]);
                continue;
            }

            if (Code != 'u')
            {
                return Fail(mCursor - 1, "A string holds an escape JSON does not have");
            }

            UInt32 Codepoint = 0;

            if (!ParseHex(Codepoint))
            {
                return Fail(mCursor, "A \\u escape needs four hexadecimal digits");
            }

            // A character outside the basic plane is written as two escapes, high half first.
            if (Codepoint >= 0xD800 && Codepoint <= 0xDBFF)
            {
                UInt32     Low    = 0;
                const Bool Paired = StrConsume(mContent, mCursor, "\\u") && ParseHex(Low);

                if (!Paired || Low < 0xDC00 || Low > 0xDFFF)
                {
                    return Fail(mCursor, "A \\u escape holds half a surrogate pair");
                }
                Codepoint = 0x10000 + ((Codepoint - 0xD800) << 10) + (Low - 0xDC00);
            }
            else if (Codepoint >= 0xDC00 && Codepoint <= 0xDFFF)
            {
                return Fail(mCursor, "A \\u escape holds half a surrogate pair");
            }
            mEscaped.AppendCodepoint(Codepoint);
        }
        return Fail(Start - 1, "A string is never closed");
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseNumber(Ref<JsonValue> Output)
    {
        const UInt Start = mCursor;
        const UInt First = mContent[mCursor] == '-' ? mCursor + 1 : mCursor;

        if (First >= mContent.GetSize() || !StrIsDigit(mContent[First]))
        {
            return Fail(Start, "A number needs a digit");
        }

        Real64 Value = StrExtractNumber<Real64>(mContent, mCursor);

        // An exponent scales the number by a power of ten.
        if (mCursor < mContent.GetSize() && (mContent[mCursor] | 0x20) == 'e')
        {
            ++mCursor;

            const Bool Down = StrConsume(mContent, mCursor, '-');

            if (!Down)
            {
                StrConsume(mContent, mCursor, '+');
            }

            if (mCursor >= mContent.GetSize() || !StrIsDigit(mContent[mCursor]))
            {
                return Fail(mCursor, "A number needs a digit in its exponent");
            }

            const SInt64 Exponent = StrExtractNumber<10, SInt64>(mContent, mCursor);
            Value *= Pow(10.0, static_cast<Real64>(Down ? -Exponent : Exponent));
        }

        if (IsInf(Value))
        {
            return Fail(Start, "A number is too large to hold");
        }

        Output = JsonValue(Value);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseWord(Text Word, ConstRef<JsonValue> Value, Ref<JsonValue> Output)
    {
        if (!StrConsume(mContent, mCursor, Word))
        {
            return Fail(mCursor, "A value is not valid JSON");
        }

        Output = Value;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::ParseHex(Ref<UInt32> Output)
    {
        const Text Digits = mContent.Slice(mCursor, Min<UInt>(4, mContent.GetSize() - mCursor));
        UInt       Read   = 0;

        Output   = StrExtractNumber<16, UInt32>(Digits, Read);
        mCursor += Read;
        return Read == 4;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool JsonParser::Fail(UInt Position, Text Message)
    {
        const Text Before = mContent.Slice(0, Min(Position, mContent.GetSize()));

        mError.Message = Message;
        mError.Line    = static_cast<UInt32>(StrCount(Before, '\n') + 1);
        mError.Column  = static_cast<UInt32>(Before.GetSize() - (StrFindLast(Before, '\n') + 1) + 1);
        mError.Path.Clear();
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void JsonParser::Prepend(Text Step)
    {
        // A key is joined to the step after it by a dot, an index stands right against it.
        if (!mError.Path.IsEmpty() && mError.Path[0] != '[')
        {
            mError.Path.Insert(0, '.');
        }
        mError.Path.Insert(0, Step);
    }
}