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

#include "Filesystem.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include "Filesystem_Win32.inl"
#elif defined(ZY_PLATFORM_POSIX)
#include "Filesystem_Posix.inl"
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::MakeAll(Text Path)
    {
        for (UInt Position = 1, Limit = Path.GetSize(); Position < Limit; ++Position)
        {
            if (const Char Character = Path[Position]; Character == '/' || Character == '\\')
            {
                if (const Char Previous = Path[Position - 1]; Previous == '/' || Previous == '\\' || Previous == ':')
                {
                    continue;
                }
                Make(Path.Slice(0, Position));
            }
        }
        return Make(Path);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::CopyAll(Text Source, Text Destination)
    {
        Result Result = Result::Success;

        if (Result = Make(Destination); Result != Result::Success)
        {
            return Result::Incomplete;
        }

        Enumerate(Source, [&](ConstRef<Record> Entry) -> Bool
        {
            const Path CurrentSource      = Path::Print<"{0}/{1}">(Source, Entry.Name);
            const Path CurrentDestination = Path::Print<"{0}/{1}">(Destination, Entry.Name);

            if (Entry.Type == Type::Directory)
            {
                if (Result = CopyAll(CurrentSource, CurrentDestination); Result != Result::Success)
                {
                    return false;
                }
            }
            else if (Entry.Type == Type::File)
            {
                if (Result = Copy(CurrentSource, CurrentDestination); Result != Result::Success)
                {
                    return false;
                }
            }
            return true;
        });
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::DeleteAll(Text Directory)
    {
        Result Result = Result::Success;

        Enumerate(Directory, [&](ConstRef<Record> Entry) -> Bool
        {
            const Path Current = Path::Print<"{0}/{1}">(Directory, Entry.Name);

            Result = (Entry.Type == Type::Directory) ? DeleteAll(Current) : Delete(Current);

            return (Result == Result::Success);
        });

        if (Result == Result::Success)
        {
            return Delete(Directory);
        }
        return Result;
    }
}