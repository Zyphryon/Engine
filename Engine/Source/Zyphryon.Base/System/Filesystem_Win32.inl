// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include <shlobj.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Type GetType(DWORD Attributes)
    {
        if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            return Filesystem::Type::Directory;
        }
        if (Attributes & FILE_ATTRIBUTE_ARCHIVE)
        {
            return Filesystem::Type::File;
        }
        if (Attributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
            return Filesystem::Type::Link;
        }
        return Filesystem::Type::Unknown;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result GetResult(DWORD Error)
    {
        switch (Error)
        {
        case ERROR_SUCCESS:
            return Filesystem::Result::Success;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case ERROR_INVALID_DRIVE:
            return Filesystem::Result::Inexistent;
        case ERROR_ACCESS_DENIED:
        case ERROR_WRITE_PROTECT:
        case ERROR_SHARING_VIOLATION:
        case ERROR_LOCK_VIOLATION:
        case ERROR_CANNOT_MAKE:
            return Filesystem::Result::Denied;
        case ERROR_INVALID_NAME:
        case ERROR_BAD_PATHNAME:
        case ERROR_FILENAME_EXCED_RANGE:
        case ERROR_DIRECTORY:
        case ERROR_BAD_DEVICE:
            return Filesystem::Result::Invalid;
        case ERROR_READ_FAULT:
        case ERROR_WRITE_FAULT:
        case ERROR_GEN_FAILURE:
        case ERROR_CRC:
        case ERROR_DISK_OPERATION_FAILED:
        case ERROR_DEVICE_NOT_CONNECTED:
        case ERROR_IO_DEVICE:
        case ERROR_IO_PENDING:
        case ERROR_HANDLE_EOF:
            return Filesystem::Result::Incomplete;
        default:
            return Filesystem::Result::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Path Filesystem::GetRootFolder()
    {
        Wide Buffer[kMaxPathLength];

        const DWORD Size = GetModuleFileNameW(nullptr, Buffer, kMaxPathLength);
        ZY_ASSERT(Size != 0, "Failed to get module filename.");

        Path Result = Path::ConvertFromUTF16(ConstSpan(Buffer, Size));

        if (const SInt Slash = StrFindLast(Result, '\\'))
        {
            Result.RemoveLast(Result.GetSize() - Slash - 1);
        }
        Result.Replace('\\', '/');
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Path Filesystem::GetDataFolder(Text Organization, Text Application)
    {
        PWSTR Path = nullptr;

        if (FAILED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE, nullptr, AddressOf(Path))))
        {
            return Text::Empty();
        }

        String<kMaxPathLength> Buffer = Path::ConvertFromUTF16(StrConvert(Path));
        CoTaskMemFree(Path);

        if (!Organization.IsEmpty())
        {
            Buffer.Append('/');
            Buffer.Append(Organization);
        }

        if (!Application.IsEmpty())
        {
            Buffer.Append('/');
            Buffer.Append(Application);
        }

        Buffer.Replace('\\', '/');
        return Buffer;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Enumerate(Text Path, AnyRef<OnEnumerate> Callback)
    {
        Sequence<Wide, kMaxPathLength + 3> InPath = StrConvertUTF16<kMaxPathLength + 3>(Path);
        InPath.Append(L'/');
        InPath.Append(L'*');
        InPath.Append(L'\0');

        WIN32_FIND_DATAW Entry;

        if (const HANDLE Handle = FindFirstFileW(InPath.GetData(), AddressOf(Entry)); Handle != INVALID_HANDLE_VALUE)
        {
            do
            {
                const ConstSpan<Wide> Name = StrConvert(Entry.cFileName);

                if (!(Name[0] == L'.' && (Name.GetSize() == 1 || (Name[1] == L'.' && Name.GetSize() == 2))))
                {
                    Record Data;
                    Data.Name = Path::ConvertFromUTF16(Name);
                    Data.Type = GetType(Entry.dwFileAttributes);
                    Data.Size = (static_cast<UInt64>(Entry.nFileSizeHigh) << 32) | Entry.nFileSizeLow;
                    Data.Time = (static_cast<UInt64>(Entry.ftLastWriteTime.dwHighDateTime) << 32) | Entry.ftLastWriteTime.dwLowDateTime;

                    if (!Callback(Data))
                    {
                        break;
                    }
                }
            }
            while (FindNextFileW(Handle, AddressOf(Entry)));

            FindClose(Handle);
            return Result::Success;
        }
        return GetResult(GetLastError());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Make(Text Path)
    {
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);

        if (CreateDirectoryW(InPath.GetData(), nullptr))
        {
            return Result::Success;
        }
        return (GetLastError() == ERROR_ALREADY_EXISTS ? Result::Success : GetResult(GetLastError()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Copy(Text Source, Text Destination)
    {
        Sequence<Wide, kMaxPathLength> InSource      = StrConvertUTF16<kMaxPathLength>(Source);
        Sequence<Wide, kMaxPathLength> InDestination = StrConvertUTF16<kMaxPathLength>(Destination);

        if (CopyFileExW(InSource.GetData(), InDestination.GetData(), nullptr, nullptr, nullptr, COPY_FILE_NO_BUFFERING))
        {
            return Result::Success;
        }
        return GetResult(GetLastError());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Delete(Text Path)
    {
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);

        if (const DWORD Attributes = GetFileAttributesW(InPath.GetData()); Attributes != INVALID_FILE_ATTRIBUTES)
        {
            if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (RemoveDirectoryW(InPath.GetData()))
                {
                    return Result::Success;
                }
            }
            else
            {
                if (DeleteFileW(InPath.GetData()))
                {
                    return Result::Success;
                }
            }
        }
        return GetResult(GetLastError());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Rename(Text Source, Text Destination)
    {
        Sequence<Wide, kMaxPathLength> InSource      = StrConvertUTF16<kMaxPathLength>(Source);
        Sequence<Wide, kMaxPathLength> InDestination = StrConvertUTF16<kMaxPathLength>(Destination);

        if (MoveFileExW(InSource.GetData(), InDestination.GetData(), MOVEFILE_REPLACE_EXISTING))
        {
            return Result::Success;
        }
        return GetResult(GetLastError());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Open(Text Path, Access Access, Ref<Handle> Output)
    {
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);

        Close(Output);

        const Bool   Reading = (Access == Access::Read);
        const HANDLE Native  = CreateFileW(
            InPath.GetData(),
            Reading ? GENERIC_READ : GENERIC_WRITE,
            Reading ? FILE_SHARE_READ : 0,
            nullptr,
            Reading ? OPEN_EXISTING : CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (Native == INVALID_HANDLE_VALUE)
        {
            return GetResult(GetLastError());
        }

        Output.Value = static_cast<SInt>(reinterpret_cast<UInt>(Native));
        return Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Tell(ConstRef<Handle> Handle, Ref<UInt64> Output)
    {
        Output = 0;

        if (!Handle)
        {
            return Result::Invalid;
        }

        LARGE_INTEGER Size;

        if (!GetFileSizeEx(reinterpret_cast<HANDLE>(Handle.Value), AddressOf(Size)))
        {
            return GetResult(GetLastError());
        }

        Output = static_cast<UInt64>(Size.QuadPart);
        return Result::Success;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Read(ConstRef<Handle> Handle, UInt64 Offset, Span<Byte> Destination)
    {
        if (!Handle)
        {
            return Result::Invalid;
        }

        const HANDLE Native    = reinterpret_cast<HANDLE>(Handle.Value);
        Ptr<Byte>    Buffer    = Destination.GetData();
        DWORD        Remaining = static_cast<DWORD>(Destination.GetSize());
        DWORD        Error     = ERROR_SUCCESS;

        while (Remaining > 0)
        {
            OVERLAPPED Position { };
            Position.Offset     = static_cast<DWORD>(Offset & 0xFFFFFFFF);
            Position.OffsetHigh = static_cast<DWORD>(Offset >> 32);

            DWORD Consumed = 0;

            if (ReadFile(Native, Buffer, Remaining, AddressOf(Consumed), AddressOf(Position)))
            {
                if (Consumed == 0)
                {
                    Error = ERROR_HANDLE_EOF;
                    break;
                }
                Remaining -= Consumed;
                Buffer    += Consumed;
                Offset    += Consumed;
            }
            else
            {
                Error = GetLastError();
                break;
            }
        }
        return GetResult(Error);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Write(ConstRef<Handle> Handle, UInt64 Offset, ConstSpan<Byte> Source)
    {
        if (!Handle)
        {
            return Result::Invalid;
        }

        const HANDLE   Native    = reinterpret_cast<HANDLE>(Handle.Value);
        ConstPtr<Byte> Buffer    = Source.GetData();
        DWORD          Remaining = static_cast<DWORD>(Source.GetSize());
        DWORD          Error     = ERROR_SUCCESS;

        while (Remaining > 0)
        {
            OVERLAPPED Position { };
            Position.Offset     = static_cast<DWORD>(Offset & 0xFFFFFFFF);
            Position.OffsetHigh = static_cast<DWORD>(Offset >> 32);

            DWORD Written = 0;

            if (WriteFile(Native, Buffer, Remaining, AddressOf(Written), AddressOf(Position)))
            {
                if (Written == 0)
                {
                    Error = ERROR_HANDLE_EOF;
                    break;
                }
                Remaining -= Written;
                Buffer    += Written;
                Offset    += Written;
            }
            else
            {
                Error = GetLastError();
                break;
            }
        }
        return GetResult(Error);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Filesystem::Close(Ref<Handle> Handle)
    {
        if (Handle)
        {
            CloseHandle(reinterpret_cast<HANDLE>(Handle.Value));

            Handle.Value = Handle::kInvalid;
        }
    }
}