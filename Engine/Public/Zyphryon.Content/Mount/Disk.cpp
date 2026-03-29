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

#include "Disk.hpp"
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_filesystem.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Disk::Disk(ConstStr8 Path)
        : mPath { CreatePath(Path) }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector<Mount::Item> Disk::Enumerate(ConstStr8 Path) const
    {
        Vector<Item> Entries;

        constexpr auto OnEnumerate = [](Ptr<void> User, ConstPtr<Char> Directory, ConstPtr<Char> Entry)
        {
            const Ptr<Vector<Item>> Context = static_cast<Ptr<Vector<Item>>>(User);

            SDL_PathInfo Info;

            if (SDL_GetPathInfo(Format("{}{}", Directory, Entry).data(), &Info))
            {
                const Mount::Entry Type = (Info.type == SDL_PATHTYPE_DIRECTORY ? Entry::Directory : Entry::File);
                Context->emplace_back(Entry, Type, Info.size, Info.modify_time);
            }
            return SDL_ENUM_CONTINUE;
        };

        if (const Bool Result = SDL_EnumerateDirectory(Format("{}{}", mPath, Path).data(), OnEnumerate, &Entries); !Result)
        {
            LOG_ERROR("Failed to enumerate directory: {}", SDL_GetError());
        }

        return Entries;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob Disk::Read(ConstStr8 Path)
    {
        Blob Result;

        if (const Ptr<SDL_IOStream> Stream = SDL_IOFromFile(Format("{}{}", mPath, Path).data(), "r"); Stream)
        {
            if (const SInt64 Size = SDL_GetIOSize(Stream); Size > 0)
            {
                Result = Blob::Bytes(SDL_GetIOSize(Stream));
                SDL_ReadIO(Stream, Result.GetData(), Result.GetSize());
            }
            else
            {
                LOG_ERROR("Invalid file size: {} ({} bytes)", Path, Size);
            }

            SDL_CloseIO(Stream);
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Disk::Write(ConstStr8 Path, ConstSpan<Byte> Bytes)
    {
        if (const Ptr<SDL_IOStream> Stream = SDL_IOFromFile(Format("{}{}", mPath, Path).data(), "w"); Stream)
        {
            SDL_WriteIO(Stream, Bytes.data(), Bytes.size());
            SDL_CloseIO(Stream);
        }
        else
        {
            LOG_ERROR("Failed to write a file: {}", SDL_GetError());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Disk::Delete(ConstStr8 Path)
    {
        if (const Bool Result = SDL_RemovePath(Format("{}{}", mPath, Path).data()); !Result)
        {
            LOG_ERROR("Failed to delete a file: {}", SDL_GetError());
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Str8 Disk::CreatePath(ConstStr8 Path)
    {
        if (Path.empty())
        {
            return Str8(SDL_GetCurrentDirectory());
        }
        return (Path.ends_with("/") ? Str8(Path) : Str8(Path).append("/"));
    }
}