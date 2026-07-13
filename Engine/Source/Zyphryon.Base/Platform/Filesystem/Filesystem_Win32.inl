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

#include "Zyphryon.Base/Lexical/Algorithm.hpp"
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
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);
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

    Filesystem::Result Filesystem::Read(Text Path, Ref<Blob> Output)
    {
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);

        const HANDLE Handle = CreateFileW(
            InPath.GetData(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (Handle != INVALID_HANDLE_VALUE)
        {
            LARGE_INTEGER Size;

            if (GetFileSizeEx(Handle, AddressOf(Size)))
            {
                Output = Blob::Allocate<Byte>(static_cast<UInt>(Size.QuadPart));

                Ptr<Byte> Buffer    = Output.GetData();
                DWORD     Remaining = Output.GetSize();
                DWORD     Error     = ERROR_SUCCESS;

                while (Remaining > 0)
                {
                    DWORD Consumed = 0;

                    if (ReadFile(Handle, Buffer, Remaining, AddressOf(Consumed), nullptr))
                    {
                        if (Consumed == 0)
                        {
                            Error = ERROR_HANDLE_EOF;
                            break;
                        }
                        Remaining -= Consumed;
                        Buffer    += Consumed;
                    }
                    else
                    {
                        Error = GetLastError();
                        break;
                    }
                }

                CloseHandle(Handle);
                return GetResult(Error);
            }
            CloseHandle(Handle);
        }
        return GetResult(GetLastError());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Write(Text Path, ConstSpan<Byte> Data)
    {
        Sequence<Wide, kMaxPathLength> InPath = StrConvertUTF16<kMaxPathLength>(Path);

        const HANDLE Handle = CreateFileW(
            InPath.GetData(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (Handle != INVALID_HANDLE_VALUE)
        {
            ConstPtr<Byte> Buffer    = Data.GetData();
            DWORD          Remaining = static_cast<DWORD>(Data.GetSize());
            DWORD          Written   = 0;
            DWORD          Error     = ERROR_SUCCESS;

            while (Remaining > 0)
            {
                if (WriteFile(Handle, Buffer, Remaining, AddressOf(Written), nullptr))
                {
                    if (Written == 0)
                    {
                        Error = ERROR_HANDLE_EOF;
                        break;
                    }
                    Remaining -= Written;
                    Buffer    += Written;
                }
                else
                {
                    Error = GetLastError();
                    break;
                }
            }

            CloseHandle(Handle);
            return GetResult(Error);
        }
        return GetResult(GetLastError());
    }
}