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

#include "Package.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyContent
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Package::Package()
        : mCount { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Package::~Package()
    {
        Close();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Open(Text Path, Signer Key)
    {
        Close();

        const Retainer<Mapper> File = Retainer<Mapper>::Create();

        if (Filesystem::Map(Path, File->Memory) != Filesystem::Result::Success)
        {
            LOG_W("Content: Failed to open the package '{0}'", Path);
            return false;
        }
        mFile = File;

        if (!Load(Key))
        {
            LOG_W("Content: '{0}' is not a package this build reads", Path);

            Close();
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Package::Close()
    {
        // The file stays in memory until the last blob lent out of it is gone, which may be well after this.
        mFile = nullptr;

        mIndex.Clear();
        mCount = 0;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Package::Entry> Package::Find(Text Path) const
    {
        const UInt64           Key     = GetKey(Path);
        const ConstSpan<Entry> Entries = GetEntries();

        UInt32 Low  = 0;
        UInt32 High = mCount;

        while (Low < High)
        {
            const UInt32 Middle = Low + (High - Low) / 2;

            if (Entries[Middle].Key < Key)
            {
                Low = Middle + 1;
            }
            else
            {
                High = Middle;
            }
        }
        return (Low < mCount && Entries[Low].Key == Key) ? AddressOf(Entries[Low]) : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Package::Read(ConstRef<Entry> Entry, Ref<Blob> Output) const
    {
        const Ptr<Byte> Data = const_cast<Ptr<Byte>>(mFile->Memory.Data) + Entry.Offset;

        // Bytes other than the ones that were packed are never handed out, whatever it was that changed them.
        if (Hash(reinterpret_cast<ConstPtr<Char>>(Data), Entry.Size) != Entry.Checksum)
        {
            LOG_W("Content: '{0}' does not match its checksum", GetName(Entry));

            Output.Clear();
            return Filesystem::Result::Invalid;
        }

        // The blob lends the bytes where they lie, and holds the file in memory until it lets go of them.
        mFile->Retain();

        Output = Blob(Data, Entry.Size, [File = AddressOf(* mFile)](Ptr<Byte>)
        {
            File->Release();
        });
        return Filesystem::Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Load(Signer Key)
    {
        ConstRef<Filesystem::View> View = mFile->Memory;

        if (View.Size < sizeof(Header))
        {
            return false;
        }

        Reader       Input(View.Data, sizeof(Header));
        const Header Head = Input.Read<Header>();

        if (Head.Magic != kMagic || Head.Version != kVersion)
        {
            return false;
        }

        // The index is used as it lies in the file, so it has to fit both the file and a blob.
        const UInt64 Length = static_cast<UInt64>(Head.Count) * sizeof(Entry) + Head.Names;

        if (Length > View.Size - sizeof(Header) || Length > kMaximum<UInt32>)
        {
            return false;
        }

        mIndex = Blob::Borrow(View.Data + sizeof(Header), Length);
        mCount = Head.Count;

        // The seal speaks for every entry and every name, so an index changed without the key is refused here.
        if (Key.Sign(mIndex) != Head.Seal)
        {
            return false;
        }
        return Verify(View.Size, Head.Names);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Verify(UInt64 Size, UInt32 Names) const
    {
        const ConstSpan<Entry> Entries = GetEntries();

        if (mCount == 0)
        {
            return true;
        }

        // Every name is walked up to its zero, so the names have to end in one.
        if (Names == 0 || mIndex.GetData<Char>()[mIndex.GetSize() - 1] != '\0')
        {
            return false;
        }

        for (UInt32 Index = 0; Index < mCount; ++Index)
        {
            ConstRef<Entry> Each = Entries[Index];

            if (Each.Offset > Size || Each.Size > Size - Each.Offset || Each.Name >= Names)
            {
                return false;
            }

            // The search halves the entries by key, which only finds one when no key comes before a smaller one.
            if (Index > 0 && Each.Key <= Entries[Index - 1].Key)
            {
                return false;
            }
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt64 Package::GetKey(Text Path)
    {
        Filesystem::Path Name;

        for (UInt Index = 0; Index < Path.GetSize(); ++Index)
        {
            Name.Append(Path[Index] == '\\' ? '/' : StrLowercase(Path[Index]));
        }
        return Hash(Name.GetData(), Name.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Gather(Text Directory, Text Prefix, Ref<Sequence<Filesystem::Path>> Files)
    {
        Bool Complete = true;

        const Filesystem::Result Listed = Filesystem::Enumerate(Directory, [&](ConstRef<Filesystem::Record> Record)
        {
            return Collect(Directory, Prefix, Record, Files, Complete);
        });
        return Complete && Listed == Filesystem::Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Collect(
        Text                            Directory,
        Text                            Prefix,
        ConstRef<Filesystem::Record>    Record,
        Ref<Sequence<Filesystem::Path>> Files,
        Ref<Bool>                       Complete)
    {
        // A hidden file is left out, so a folder packs to the same bytes whichever system packed it.
        if (StrStartsWith(Record.Name, "."))
        {
            return true;
        }

        const Filesystem::Path Name = Filesystem::Path::Print("{0}{1}", Prefix, Record.Name);

        if (Record.Type == Filesystem::Type::Directory)
        {
            const Filesystem::Path Inner = Filesystem::Path::Join(Directory, '/', Record.Name);

            Complete = Gather(Inner, Filesystem::Path::Print("{0}/", Name), Files);
        }
        else if (Record.Type == Filesystem::Type::File)
        {
            if (Record.Size > kMaximum<UInt32>)
            {
                LOG_W("Content: '{0}' is too large to pack", Name);

                Complete = false;
            }
            else
            {
                Files.Append(Name);
            }
        }
        return Complete;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Repeats(Text Folder, Text Name, ConstRef<Blob> Data)
    {
        Blob Other;

        if (Filesystem::Read(Filesystem::Path::Join(Folder, '/', Name), Other) != Filesystem::Result::Success)
        {
            return false;
        }
        return Other.GetSize() == Data.GetSize() && Compare(Other.GetData(), Data.GetData(), Data.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Store(ConstRef<Filesystem::Handle> File, Text Folder, ConstSpan<Filesystem::Path> Files, Signer Key)
    {
        Sequence<Entry>       Entries;
        Sequence<Char>        Names;
        Table<UInt64, UInt32> Seen;

        for (ConstRef<Filesystem::Path> Name : Files)
        {
            Ref<Entry> Each = Entries.Append();
            Each.Key  = GetKey(Name);
            Each.Name = static_cast<UInt32>(Names.GetSize());

            Names.Append(Name);
            Names.Append('\0');
        }

        // The index goes first in the file but is written last, once the hash of every file is known.
        Header Head { };
        Head.Magic   = kMagic;
        Head.Version = kVersion;
        Head.Count   = static_cast<UInt32>(Entries.GetSize());
        Head.Names   = static_cast<UInt32>(Names.GetSize());

        UInt64 Cursor = Align(sizeof(Header) + Entries.GetSize() * sizeof(Entry) + Names.GetSize(), kAlignment);

        for (UInt Index = 0; Index < Files.GetSize(); ++Index)
        {
            Blob Data;

            if (Filesystem::Read(Filesystem::Path::Join(Folder, '/', Files[Index]), Data) != Filesystem::Result::Success)
            {
                LOG_W("Content: Failed to read '{0}' for packing", Files[Index]);
                return false;
            }

            Ref<Entry> Each = Entries[Index];
            Each.Offset   = Cursor;
            Each.Checksum = Hash(Data.GetData<Char>(), Data.GetSize());
            Each.Size     = Data.GetSize();

            // A file holding the very bytes of an earlier one is not written again, and is found where that one is.
            if (const ConstPtr<UInt32> Twin = Seen.Find(Each.Checksum); Twin && Repeats(Folder, Files[* Twin], Data))
            {
                Each.Offset = Entries[* Twin].Offset;
                continue;
            }
            Seen.Assign(Each.Checksum, static_cast<UInt32>(Index));

            if (Filesystem::Write(File, Cursor, ConstSpan(Data.GetData(), Data.GetSize())) != Filesystem::Result::Success)
            {
                return false;
            }
            Cursor = Align(Cursor + Data.GetSize(), kAlignment);
        }

        Entries.Sort([](ConstRef<Entry> Left, ConstRef<Entry> Right)
        {
            return Left.Key < Right.Key;
        });

        // Two paths hashing to one key would hide one file behind the other, so such a package is not written.
        for (UInt Index = 1; Index < Entries.GetSize(); ++Index)
        {
            if (Entries[Index].Key == Entries[Index - 1].Key)
            {
                return false;
            }
        }

        Writer Listing;
        Listing.Write(Entries.GetData(), static_cast<UInt32>(Entries.GetSize() * sizeof(Entry)));
        Listing.Write(Names.GetData(), static_cast<UInt32>(Names.GetSize()));

        // The seal is worked out last, since it speaks for the entries and the names as they are written.
        Head.Seal = Key.Sign(Listing);

        Writer Index;
        Index.Write(Head);
        Index.Write(Listing.GetData(), Listing.GetSize());

        return Filesystem::Write(File, 0, Index) == Filesystem::Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Package::Pack(Text Folder, Text Output, Signer Key)
    {
        Sequence<Filesystem::Path> Files;

        if (!Gather(Folder, Text(), Files))
        {
            LOG_W("Content: Failed to list '{0}' for packing", Folder);
            return false;
        }

        // The files are laid out by name, so the same folder packs to the same bytes however it was listed.
        Files.Sort([](ConstRef<Filesystem::Path> Left, ConstRef<Filesystem::Path> Right)
        {
            return StrCompareCaseInsensitive(Left, Right) < 0;
        });

        // A package finds a file whatever its case, so two names apart only by case cannot both be packed.
        for (UInt Index = 1; Index < Files.GetSize(); ++Index)
        {
            if (StrCompareCaseInsensitive(Files[Index], Files[Index - 1]) == 0)
            {
                LOG_W("Content: '{0}' and '{1}' differ only by case", Files[Index - 1], Files[Index]);
                return false;
            }
        }

        Filesystem::Handle File;

        if (Filesystem::Open(Output, Filesystem::Access::Write, File) != Filesystem::Result::Success)
        {
            LOG_W("Content: Failed to create the package '{0}'", Output);
            return false;
        }

        const Bool Stored = Store(File, Folder, Files, Key);

        Filesystem::Close(File);

        // A package cut short has no index to read it by, so nothing is left where one was asked for.
        if (!Stored)
        {
            LOG_W("Content: Failed to write the package '{0}'", Output);

            Filesystem::Delete(Output);
        }
        return Stored;
    }
}