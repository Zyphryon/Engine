// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Base/Memory/Trackable.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Interface for a virtual mount point providing read, write, and delete access to asset files.
    ///
    /// A mount abstracts an underlying storage system (e.g., filesystem directory, archive file, or remote source)
    /// and exposes uniform file access methods for the content system.
    class Mount : public Trackable<Mount>
    {
    public:

        /// \brief Enumeration of file system entry types.
        enum class Entry : UInt8
        {
            File,
            Directory,
        };

        /// \brief Metadata information for a file system entry.
        struct Item
        {
            /// \brief The name of the entry including extension (for files).
            Str8   Name;

            /// \brief The type of the entry (file or directory).
            Entry  Type;

            /// \brief The size of the entry in bytes.
            UInt64 Size;

            /// \brief The last modification time in milliseconds since epoch.
            UInt64 Time;
        };

    public:

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Mount() = default;

        /// \brief Enumerates all file system entries (files and folders) at the specified path.
        ///
        /// \param Path The directory path to enumerate. Empty string indicates the mount's root directory.
        /// \return A vector of \ref Item objects containing metadata for each file and directory.
        virtual Vector<Item> Enumerate(ConstStr8 Path) const = 0;

        /// \brief Reads all bytes from a file located at the specified path.
        ///
        /// \param Path Path to the file within this mount.
        /// \return A \ref Blob containing the file's raw binary data.
        virtual Blob Read(ConstStr8 Path) = 0;

        /// \brief Writes raw bytes to a file at the specified path.
        ///
        /// \param Path  Path to the file within this mount.
        /// \param Bytes Contiguous buffer of bytes to write.
        virtual void Write(ConstStr8 Path, ConstSpan<Byte> Bytes) = 0;

        /// \brief Deletes a file located at the specified path.
        ///
        /// \param Path Path to the file within this mount.
        virtual void Delete(ConstStr8 Path) = 0;
    };
}