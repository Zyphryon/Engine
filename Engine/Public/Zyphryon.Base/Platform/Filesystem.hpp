// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Container/Blob.hpp"
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Filesystem
{
    /// \brief Gets the path to the application's executable directory.
    ///
    /// \return A string containing the path to the application's executable directory.
    ZYPHRYON_INLINE static Str8 GetBase()
    {
        return  SDL_GetBasePath();
    }

    /// \brief Gets the path to the user's home directory.
    ///
    /// \param Organization The name of the organization or company.
    /// \param Application  The name of the application.
    /// \return A string containing the path to the user's home directory for the specified application.
    ZYPHRYON_INLINE static Str8 GetHome(ConstStr8 Organization, ConstStr8 Application)
    {
        return SDL_GetPrefPath(Organization.data(), Application.data());
    }

    /// \brief Creates a directory at the specified path.
    ///
    /// \param Path The path where the directory should be created.
    /// \return `true` if the directory was successfully created, `false` otherwise.
    ZYPHRYON_INLINE static Bool Make(ConstStr8 Path)
    {
        return SDL_CreateDirectory(Path.data());
    }

    /// \brief Copies a file from the source path to the destination path.
    ///
    /// \param Source      The path of the file to be copied.
    /// \param Destination The path where the file should be copied to.
    ZYPHRYON_INLINE static Bool Copy(ConstStr8 Source, ConstStr8 Destination)
    {
        return SDL_CopyFile(Source.data(), Destination.data());
    }

    /// \brief Recursively copies all files and directories from the source path to the destination path.
    ///
    /// \param Source      The path of the directory to be copied.
    /// \param Destination The path where the directory should be copied to.
    /// \return `true` if all files and directories were successfully copied, `false` otherwise.
    Bool CopyAll(ConstStr8 Source, ConstStr8 Destination);

    /// \brief Deletes the file or directory at the specified path.
    ///
    /// \param Path The path of the file or directory to be deleted.
    /// \return `true` if the file or directory was successfully deleted, `false` otherwise.
    ZYPHRYON_INLINE static Bool Delete(ConstStr8 Path)
    {
        return SDL_RemovePath(Path.data());
    }

    /// \brief Renames a file or directory from the old path to the new name.
    ///
    /// \param Path The current path of the file or directory to be renamed.
    /// \param Name The new name for the file or directory.
    /// \return `true` if the file or directory was successfully renamed, `false`
    ZYPHRYON_INLINE static Bool Rename(ConstStr8 Path, ConstStr8 Name)
    {
        return SDL_RenamePath(Path.data(), Name.data());
    }

    /// \brief Loads the contents of a file into a binary blob.
    ///
    /// \param Path The path of the file to be loaded.
    /// return A Blob containing the contents of the loaded file.
    ZYPHRYON_INLINE static Blob Load(ConstStr8 Path)
    {
        size_t Size = 0;
        return Blob(SDL_LoadFile(Path.data(), std::addressof(Size)), Size, SDL_free);
    }

    /// \brief Saves binary data to a file at the specified path.
    ///
    /// \param Path  The path where the file should be saved.
    /// \param Data  A Blob containing the binary data to be saved to the file.
    /// \param Count The number of elements of the specified type to be saved to the file.
    /// \return `true` if the file was successfully saved, `false` otherwise.
    template<typename Type>
    ZYPHRYON_INLINE static Bool Save(ConstStr8 Path, ConstPtr<Type> Data, UInt32 Count)
    {
        return SDL_SaveFile(Path.data(), Data, Count * sizeof(Type));
    }

    /// \brief Saves text data to a file at the specified path.
    ///
    /// \param Path The path where the file should be saved.
    /// \param Data A string containing the text data to be saved to the file.
    ZYPHRYON_INLINE static Bool Save(ConstStr8 Path, ConstStr8 Data)
    {
        return Save(Path, Data.data(), static_cast<UInt32>(Data.size()));
    }
}