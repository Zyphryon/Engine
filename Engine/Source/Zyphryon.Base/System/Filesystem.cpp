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

inline namespace ZyBase
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

    Filesystem::Result Filesystem::Ensure(Text Path)
    {
        const Text Parent = StrBeforeLast(Path, '/');

        return Parent.IsEmpty() ? Result::Success : MakeAll(Parent);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CopyEntry(Text Source, Text Destination, ConstRef<Filesystem::Record> Entry,
        Ref<Filesystem::Result> Status)
    {
        const Filesystem::Path From = Filesystem::Path::Print<"{0}/{1}">(Source, Entry.Name);
        const Filesystem::Path To   = Filesystem::Path::Print<"{0}/{1}">(Destination, Entry.Name);

        if (Entry.Type == Filesystem::Type::Directory)
        {
            Status = Filesystem::CopyAll(From, To);
        }
        else if (Entry.Type == Filesystem::Type::File)
        {
            Status = Filesystem::Copy(From, To);
        }
        return Status == Filesystem::Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::CopyAll(Text Source, Text Destination)
    {
        if (Make(Destination) != Result::Success)
        {
            return Result::Incomplete;
        }

        Result Status = Result::Success;

        Enumerate(Source, [&](ConstRef<Record> Entry)
        {
            return CopyEntry(Source, Destination, Entry, Status);
        });
        return Status;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool DeleteEntry(Text Directory, ConstRef<Filesystem::Record> Entry, Ref<Filesystem::Result> Status)
    {
        const Filesystem::Path Current = Filesystem::Path::Print<"{0}/{1}">(Directory, Entry.Name);

        Status = (Entry.Type == Filesystem::Type::Directory) ? Filesystem::DeleteAll(Current)
                                                             : Filesystem::Delete(Current);

        return Status == Filesystem::Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::DeleteAll(Text Directory)
    {
        Result Status = Result::Success;

        Enumerate(Directory, [&](ConstRef<Record> Entry)
        {
            return DeleteEntry(Directory, Entry, Status);
        });

        if (Status == Result::Success)
        {
            return Delete(Directory);
        }
        return Status;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Read(Text Path, Ref<Blob> Output)
    {
        Handle File;

        if (const Result Opened = Open(Path, Access::Read, File); Opened != Result::Success)
        {
            return Opened;
        }

        UInt64 Size   = 0;
        Result Status = Tell(File, Size);

        if (Status == Result::Success)
        {
            Output = Blob::Allocate<Byte>(Size);
            Status = Read(File, 0, Span(Output.GetData(), Output.GetSize()));
        }

        Close(File);
        return Status;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Write(Text Path, ConstSpan<Byte> Data)
    {
        Handle File;

        if (const Result Opened = Open(Path, Access::Write, File); Opened != Result::Success)
        {
            return Opened;
        }

        const Result Status = Write(File, 0, Data);

        Close(File);
        return Status;
    }
}