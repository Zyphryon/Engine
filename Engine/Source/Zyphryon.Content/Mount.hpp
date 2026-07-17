// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Interface for a virtual mount point providing read, write, and delete access to asset files.
    ///
    /// A mount abstracts an underlying storage system (e.g., filesystem directory, archive file, or remote source).
    class Mount : public Retainable<Mount>
    {
    public:

        /// \brief A callback invoked when the contents of a directory are enumerated.
        using OnEnumerate = Delegate<void(Filesystem::Result, Sequence<Filesystem::Record>)>;

        /// \brief A callback invoked when a file is read from the mount.
        using OnRead      = Delegate<void(Filesystem::Result, Blob)>;

        /// \brief A callback invoked with the result of an operation.
        using OnResult    = Delegate<void(Filesystem::Result)>;

    public:

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Mount() = default;

        /// \brief Checks if the mount natively supports asynchronous operations.
        ///
        /// \return `true` if asynchronous operations are supported, otherwise `false`.
        virtual Bool IsAsynchronous() const = 0;

        /// \brief Enumerates the entries at the specified path within the mount.
        ///
        /// \param Path     The path to enumerate, relative to the mount's root.
        /// \param Callback The callback function that will be called for each record found.
        /// \return A sequence of items representing the entries at the specified path.
        virtual void Enumerate(Text Path, AnyRef<OnEnumerate> Callback) const = 0;

        /// \brief Deletes the file or directory at the specified path, if it exists.
        ///
        /// \param Path     The path to the file or directory to delete, relative to the mount's root.
        /// \param Callback The callback function that will be called with the result of the delete operation.
        virtual void Delete(Text Path, AnyRef<OnResult> Callback) = 0;

        /// \brief Copies a file from the source path to the destination path within the mount.
        ///
        /// \param Source      The path to the source file, relative to the mount's root.
        /// \param Destination The path to the destination file, relative to the mount's root.
        /// \param Callback    The callback function that will be called with the result of the copy operation
        virtual void Copy(Text Source, Text Destination, AnyRef<OnResult> Callback) = 0;

        /// \brief Reads the contents of the file at the specified path.
        ///
        /// \param Path     The path to the file to read, relative to the mount's root.
        /// \param Callback The callback function that will be called with the file's contents if succeeds.
        virtual void Read(Text Path, AnyRef<OnRead> Callback) = 0;

        /// \brief Writes the specified bytes to a file at the given path, creating or overwriting it as necessary.
        ///
        /// \param Path     The path to the file to write, relative to the mount's root.
        /// \param Bytes    The raw binary data to write to the file.
        /// \param Callback The callback function that will be called with the result of the write operation.
        virtual void Write(Text Path, AnyRef<Blob> Bytes, AnyRef<OnResult> Callback) = 0;
    };
}