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

#include "Environment.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Environment::Parse(UInt Count, ConstPtr<ConstPtr<Char>> Arguments)
    {
        static constexpr Text kPrefix   = "--";
        static constexpr Text kNegation = "no-";

        for (UInt Index = 1; Index < Count; ++Index)
        {
            const Text Argument = StrConvert(Arguments[Index]);

            if (!StrStartsWith(Argument, kPrefix))
            {
                mOperands.Append(Argument);
                continue;
            }

            Text Name  = Argument.Slice(kPrefix.GetSize());
            Text Value = Text::Empty();

            if (const SInt Separator = StrFind(Name, '='); Separator >= 0)
            {
                Value = Name.Slice(Separator + 1);
                Name  = Name.Slice(0, Separator);
            }
            else if (StrStartsWith(Name, kNegation))
            {
                Name  = Name.Slice(kNegation.GetSize());
                Value = "false";
            }
            else if (Index + 1 < Count)
            {
                if (const Text Next = StrConvert(Arguments[Index + 1]); !StrStartsWith(Next, kPrefix))
                {
                    Value = Next;
                    ++Index;
                }
            }

            if (!Name.IsEmpty())
            {
                mSwitches.Append(Name, Value);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Environment::Switch> Environment::Find(Text Name) const
    {
        for (UInt Index = mSwitches.GetSize(); Index > 0; --Index)
        {
            ConstRef<Switch> Entry = mSwitches[Index - 1];

            if (StrEqualCaseInsensitive(Entry.Name, Name))
            {
                return AddressOf(Entry);
            }
        }
        return nullptr;
    }
}