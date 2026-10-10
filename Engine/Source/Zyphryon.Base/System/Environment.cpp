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

inline namespace ZyBase
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Environment::Parse(UInt Count, ConstPtr<ConstPtr<Char>> Arguments)
    {
        static constexpr Text kPrefix   = "--";
        static constexpr Text kNegation = "no-";

        mStorage.Clear();
        mStorage.Reserve(Count);

        for (UInt Index = 1; Index < Count; ++Index)
        {
            mStorage.Append(StrConvert(Arguments[Index]));
        }

        for (UInt Index = 0; Index < mStorage.GetSize(); ++Index)
        {
            const Text Argument = mStorage[Index];

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
            else if (Index + 1 < mStorage.GetSize())
            {
                if (const Text Next = mStorage[Index + 1]; !StrStartsWith(Next, kPrefix))
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
        const SInt Index = mSwitches.FindLast([Name](ConstRef<Switch> Entry)
        {
            return StrEqualCaseInsensitive(Entry.Name, Name);
        });
        return Index >= 0 ? AddressOf(mSwitches[static_cast<UInt>(Index)]) : nullptr;
    }
}