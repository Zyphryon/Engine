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

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#if   defined(ZY_PLATFORM_LINUX)
#include <sys/sendfile.h>
#elif defined(ZY_PLATFORM_MACOS)
#include <copyfile.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result GetResult(SInt32 Error)
    {
        switch (Error)
        {
        case 0:
            return Filesystem::Result::Success;
        case ENOENT:
        case ENOTDIR:
        case ENAMETOOLONG:
        case ENODEV:
            return Filesystem::Result::Inexistent;
        case EACCES:
        case EPERM:
        case EROFS:
            return Filesystem::Result::Denied;
        case EINVAL:
        case EFAULT:
        case EBADF:
        case EISDIR:
        case ENOTTY:
        case ELOOP:
        case EEXIST:
            return Filesystem::Result::Invalid;
        case EIO:
            return Filesystem::Result::Incomplete;
        default:
            return Filesystem::Result::Unknown;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Path Filesystem::GetRootFolder()
    {
        Char Buffer[kMaxPathLength];

        if (getcwd(Buffer, sizeof(Buffer)))
        {
            return StrConvert(Buffer);
        }
        return "/";
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Path Filesystem::GetDataFolder(Text Organization, Text Application)
    {
        Path Buffer;

        if (const ConstPtr<Char> XDGDataHome = getenv("XDG_DATA_HOME"))
        {
            Buffer.Append(StrConvert(XDGDataHome));
        }
        else
        {
            if (const ConstPtr<Char> Home = getenv("HOME"))
            {
                Buffer.Append(StrConvert(Home));
#if defined(ZY_PLATFORM_MACOS)
                Buffer.Append("/Library/Application Support/");
#else
                Buffer.Append("/.local/share/");
#endif
            }
        }

        if (!Buffer.IsEmpty())
        {
            Buffer.Append(Organization);
            Buffer.Append('/');
            Buffer.Append(Application);
        }
        return Buffer;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Enumerate(Text Path, AnyRef<OnEnumerate> Callback)
    {
        if (const Ptr<DIR> Directory = opendir(Path.GetData()))
        {
            const SInt32 Handle = dirfd(Directory);

            while (const ConstPtr<dirent> Entry = readdir(Directory))
            {
                const Text Name = StrConvert(Entry->d_name);

                if (!StrStartsWith(Entry->d_name, ".") && !StrStartsWith(Entry->d_name, ".."))
                {
                    struct stat Statistics { };

                    if (fstatat(Handle, Entry->d_name, & Statistics, 0) == 0)
                    {
                        Record Record;
                        Record.Name = Name;
                        Record.Time = static_cast<UInt64>(Statistics.st_mtime);

                        if (S_ISDIR(Statistics.st_mode))
                        {
                            Record.Type = Type::Directory;
                            Record.Size = 0;
                        }
                        else
                        {
                            Record.Type = Type::File;
                            Record.Size = static_cast<UInt64>(Statistics.st_size);
                        }

                        if (!Callback(Record))
                        {
                            break;
                        }
                    }
                }
            }

            closedir(Directory);
            return Result::Success;
        }
        return GetResult(errno);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Make(Text Path)
    {
        if (mkdir(Path.GetData(), 0755) == 0 || errno == EEXIST)
        {
            return Result::Success;
        }
        return GetResult(errno);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Copy(Text Source, Text Destination)
    {
#if defined(ZY_PLATFORM_MACOS)

        if (copyfile(Source.GetData(), Destination.GetData(), nullptr, COPYFILE_ALL) == 0)
        {
            return Result::Success;
        }
        return GetResult(errno);

#else

        Blob Buffer;

        if (const Result Result = Read(Source, Buffer); Result == Result::Success)
        {
            return Write(Destination, Buffer);
        }
        else
        {
            return Result;
        }

#endif
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Delete(Text Path)
    {
        if (remove(Path.GetData()) == 0 || errno == ENOENT)
        {
            return Result::Success;
        }
        return GetResult(errno);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Rename(Text Source, Text Destination)
    {
        if (rename(Source.GetData(), Destination.GetData()) == 0)
        {
            return Result::Success;
        }
        return GetResult(errno);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Filesystem::Result Filesystem::Open(Text Path, Access Access, Ref<Handle> Output)
    {
        Close(Output);

        const SInt32 Native = (Access == Access::Read)
            ? open(Path.GetData(), O_RDONLY)
            : open(Path.GetData(), O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (Native < 0)
        {
            return GetResult(errno);
        }

        Output.Value = static_cast<SInt>(Native);
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

        struct stat Statistics { };

        if (fstat(static_cast<SInt32>(Handle.Value), AddressOf(Statistics)) != 0)
        {
            return GetResult(errno);
        }

        Output = static_cast<UInt64>(Statistics.st_size);
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

        const SInt32 Native    = static_cast<SInt32>(Handle.Value);
        Ptr<Byte>    Buffer    = Destination.GetData();
        UInt         Remaining = Destination.GetSize();
        SInt32       Error     = 0;

        while (Remaining > 0)
        {
            const ssize_t Consumed = pread(Native, Buffer, Remaining, static_cast<off_t>(Offset));

            if (Consumed > 0)
            {
                Remaining -= Consumed;
                Buffer    += Consumed;
                Offset    += Consumed;
            }
            else
            {
                if (Consumed == 0)
                {
                    Error = EIO;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    Error = errno;
                }
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

        const SInt32   Native    = static_cast<SInt32>(Handle.Value);
        ConstPtr<Byte> Buffer    = Source.GetData();
        UInt           Remaining = Source.GetSize();
        SInt32         Error     = 0;

        while (Remaining > 0)
        {
            const ssize_t Written = pwrite(Native, Buffer, Remaining, static_cast<off_t>(Offset));

            if (Written > 0)
            {
                Remaining -= Written;
                Buffer    += Written;
                Offset    += Written;
            }
            else
            {
                if (Written == 0)
                {
                    Error = EIO;
                }
                else if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    Error = errno;
                }
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
            close(static_cast<SInt32>(Handle.Value));

            Handle.Value = Handle::kInvalid;
        }
    }
}