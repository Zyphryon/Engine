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

#include "Manifest.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Extract(Text Value, Ref<UInt32> Output)
    {
        UInt Cursor = 0;
        Output = StrExtractNumber<10, UInt32>(Value, Cursor);

        return Cursor > 0 && Cursor == Value.GetSize() && Output <= 0xFFFF;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Sequence<Manifest::Entry> Manifest::Read(Text Path)
    {
        Blob Input;

        if (Filesystem::Read(Path, Input) != Filesystem::Result::Success || Input == nullptr)
        {
            LOG_E("Texture: failed to read the manifest '{0}'", Path);

            return Sequence<Entry>();
        }

        Sequence<Entry> Entries;

        const Text Content(Input.GetData<Char>(), Input.GetSize());

        UInt Cursor = 0;

        while (Cursor < Content.GetSize())
        {
            const SInt           Break  = StrFind(Content.Slice(Cursor), '\n');
            const ConstPtr<Char> Anchor = Content.GetData() + Cursor;

            UInt Length = (Break < 0) ? Content.GetSize() - Cursor : static_cast<UInt>(Break);

            Cursor += (Break < 0) ? Length : Length + 1;

            if (Length > 0 && Anchor[Length - 1] == '\r')
            {
                --Length;
            }

            const Text Line = StrTrim(Text(Anchor, Length));

            if (Line.IsEmpty() || Line[0] == '#')
            {
                continue;
            }

            // The source may hold spaces, so the four numbers are taken from the end of the line inward.
            UInt32 Field[4] = { };
            Text   Head     = Line;
            Bool   Parsed   = true;

            for (SInt32 Index = 3; Index >= 0 && Parsed; --Index)
            {
                const SInt Space = StrFindLast(Head, ' ');

                Parsed = (Space > 0) && Extract(Head.Slice(static_cast<UInt>(Space) + 1), Field[Index]);
                Head   = Parsed ? StrTrimRight(Head.Slice(0, static_cast<UInt>(Space))) : Head;
            }

            if (!Parsed || Head.IsEmpty())
            {
                LOG_E("Texture: '{0}' is not a manifest entry", Line);

                return Sequence<Entry>();
            }

            Ref<Entry> Value = Entries.Append();
            Value.Source = Str(Head);
            Value.X      = static_cast<UInt16>(Field[0]);
            Value.Y      = static_cast<UInt16>(Field[1]);
            Value.Width  = static_cast<UInt16>(Field[2]);
            Value.Height = static_cast<UInt16>(Field[3]);
        }

        if (Entries.IsEmpty())
        {
            LOG_E("Texture: the manifest '{0}' names no frames", Path);
        }
        return Entries;
    }
}