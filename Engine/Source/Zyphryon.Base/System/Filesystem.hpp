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

#include "Zyphryon.Base/Functional/Delegate.hpp"
#include "Zyphryon.Base/Lexical/String.hpp"
#include "Zyphryon.Base/Memory/Blob.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Provides platform-independent interface for file system operations.
    class Filesystem final
    {
    public:

        /// \brief The maximum number of characters allowed in a file system path.
        static constexpr UInt kMaxPathLength = 320;

        /// \brief The maximum number of characters allowed in a file or directory name.
        static constexpr UInt kMaxNameLength = 128;

        /// \brief A fixed-capacity string type used to represent file system paths.
        using Path = String<kMaxPathLength>;

        /// \brief A fixed-capacity string type used to represent file system names.
        using Name = String<kMaxNameLength>;

        /// \brief Describes the result of a file system operation.
        enum class Result : UInt8
        {
            Success,        ///< The operation completed successfully.
            Invalid,        ///< The provided path was invalid.
            Inexistent,     ///< The specified file or directory does not exist.
            Denied,         ///< The operation was denied due to insufficient permissions.
            Incomplete,     ///< The operation could not be completed due to an incomplete state.
            Unknown,        ///< An unknown error occurred.
        };

        /// \brief Describes what an open file may be used for.
        enum class Access : UInt8
        {
            Read,           ///< Open an existing file for reading.
            Write,          ///< Create the file, or truncate it if it exists, and open it for writing.
        };

        /// \brief Describes the type of a file system entry.
        enum class Type : UInt8
        {
            File,           ///< A regular file.
            Directory,      ///< A directory.
            Link,           ///< A symbolic link.
            Unknown         ///< An entry of unknown or unsupported type.
        };

        /// \brief Represents a file system entry returned during directory enumeration.
        struct Record final
        {
            /// The name of the file or directory.
            Name   Name;

            /// The type of the entry.
            Type   Type;

            /// The size of the file in bytes, or zero for directories.
            UInt64 Size;

            /// The last write time of the file or directory, in platform-native ticks.
            UInt64 Time;
        };

        /// \brief A callback invoked for each entry found during directory enumeration.
        ///
        /// \return `true` to continue enumeration, `false` to stop early.
        using OnEnumerate = Delegate<Bool(ConstRef<Record>)>;

        /// \brief Identifies an open file, transferred at explicit offsets rather than through a cursor.
        struct Handle final
        {
            /// The value standing for a handle that refers to no file, shared by both backends.
            static constexpr SInt kInvalid = -1;

            /// The native descriptor the platform layer transfers through.
            SInt Value = kInvalid;

            /// \brief Checks whether the handle refers to an open file.
            ///
            /// \return `true` if a file is open, otherwise `false`.
            ZY_INLINE explicit operator Bool() const
            {
                return Value != kInvalid;
            }
        };

    public:

        /// \brief Gets the path to the user's root directory.
        ///
        /// \return A path to the user's root directory.
        static Path GetRootFolder();

        /// \brief Gets the path to the user's home directory.
        ///
        /// \param Organization The name of the organization or company.
        /// \param Application  The name of the application.
        /// \return A path to the user's home directory for the specified application.
        static Path GetDataFolder(Text Organization, Text Application);

        /// \brief Enumerates the entries in a directory.
        ///
        /// \param Path     The path to the directory to enumerate.
        /// \param Callback The callback function that will be called for each record found.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Enumerate(Text Path, AnyRef<OnEnumerate> Callback);

        /// \brief Creates a directory at the specified path.
        ///
        /// \param Path The path where the directory should be created.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Make(Text Path);

        /// \brief Creates a directory at the specified path, including all missing parent directories.
        ///
        /// \param Path The path where the directory hierarchy should be created.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result MakeAll(Text Path);

        /// \brief Ensures the parent directory of the given path exists, creating it if necessary.
        ///
        /// \param Path The file path whose parent directory should exist.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Ensure(Text Path);

        /// \brief Copies a file from the source path to the destination path.
        ///
        /// \param Source      The path of the file to be copied.
        /// \param Destination The path where the file should be copied to.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Copy(Text Source, Text Destination);

        /// \brief Recursively copies all files and directories from the source path to the destination path.
        ///
        /// \param Source      The path of the directory to be copied.
        /// \param Destination The path where the directory should be copied to.
        /// \return `true` if all files and directories were successfully copied, `false` otherwise.
        static Result CopyAll(Text Source, Text Destination);

        /// \brief Renames a file or directory at the specified path.
        ///
        /// \param Source      The path of the file or directory to be renamed.
        /// \param Destination The new path for the file or directory.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Rename(Text Source, Text Destination);

        /// \brief Deletes the file or directory at the specified path.
        ///
        /// \param Path The path of the file or directory to be deleted.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Delete(Text Path);

        /// \brief Recursively deletes all files and directories at the specified path.
        ///
        /// \param Directory The path of the directory to be deleted.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result DeleteAll(Text Directory);

        /// \brief Reads the contents of a file into a binary blob.
        ///
        /// \param Path   The path of the file to be read.
        /// \param Output The blob where the file contents will be stored.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Read(Text Path, Ref<Blob> Output);

        /// \brief Writes binary data to a file at the specified path.
        ///
        /// \param Path The path of the file to be written.
        /// \param Data The binary data to write to the file.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Write(Text Path, ConstSpan<Byte> Data);

        /// \brief Writes text to a file at the specified path.
        ///
        /// \param Path The path of the file to be written.
        /// \param Data The text to write to the file, whose bytes are written as they are.
        /// \return A \p Result indicating the success or failure of the operation.
        ZY_INLINE static Result Write(Text Path, Text Data)
        {
            return Write(Path, ConstSpan<Byte>(reinterpret_cast<ConstPtr<Byte>>(Data.GetData()), Data.GetSize()));
        }

        /// \brief Opens a file and keeps it open for repeated transfers at explicit offsets.
        ///
        /// \param Path   The path of the file to be opened.
        /// \param Access What the file will be used for.
        /// \param Output The handle receiving the open file, left closed when the file could not be opened.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Open(Text Path, Access Access, Ref<Handle> Output);

        /// \brief Measures how large the file behind a handle currently is.
        ///
        /// \param Handle The handle to measure.
        /// \param Output The size in bytes.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Tell(ConstRef<Handle> Handle, Ref<UInt64> Output);

        /// \brief Reads a range of an open file into the given buffer.
        ///
        /// \param Handle      The handle to read from.
        /// \param Offset      The byte offset within the file at which the read begins.
        /// \param Destination The buffer receiving the bytes; its size names how many are read.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Read(ConstRef<Handle> Handle, UInt64 Offset, Span<Byte> Destination);

        /// \brief Writes a buffer into an open file at the given offset, extending it as needed.
        ///
        /// \param Handle The handle to write to.
        /// \param Offset The byte offset within the file at which the write begins.
        /// \param Source The bytes to write.
        /// \return A \p Result indicating the success or failure of the operation.
        static Result Write(ConstRef<Handle> Handle, UInt64 Offset, ConstSpan<Byte> Source);

        /// \brief Closes an open file and resets the handle.
        ///
        /// \param Handle The handle to close.
        static void Close(Ref<Handle> Handle);

    };
}