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

namespace ZyContent
{
    /// \brief Represents many files packed into one, which is laid into memory and read by name where it lies.
    class ZY_API Package final
    {
    public:

        /// \brief Represents one file of the package, as its index lays it out.
        struct Entry final
        {
            /// The hash of the path, lowercase and with forward slashes, which the index is sorted by.
            UInt64 Key;

            /// The offset of the bytes from the start of the package, shared by files holding the same bytes.
            UInt64 Offset;

            /// The hash of the bytes, which every read is checked against.
            UInt64 Checksum;

            /// The number of bytes.
            UInt32 Size;

            /// The offset of the path within the names, where it stands zero-ended in its original case.
            UInt32 Name;
        };

    public:

        /// \brief Constructs a package that is not open.
        Package();

        /// \brief Closes the package.
        ~Package();

        /// \brief Packages are not copied, since each holds its file in memory.
        Package(ConstRef<Package> Other) = delete;

        /// \brief Opens a package and reads its index, closing whichever one was open.
        ///
        /// \param Path The path of the package file.
        /// \param Key  The key the package was sealed under when it was packed.
        /// \return `true` if the package is open, `false` if the file is missing, is not a package or bears another seal.
        Bool Open(Text Path, Signer Key = Signer());

        /// \brief Closes the package and forgets its index, leaving the blobs it lent as good as they were.
        void Close();

        /// \brief Checks whether a package is open.
        ///
        /// \return `true` if a package is open, `false` otherwise.
        ZY_INLINE Bool IsOpen() const
        {
            return static_cast<Bool>(mFile);
        }

        /// \brief Gets every file of the package.
        ///
        /// \return The entries, sorted by key, or none while no package is open.
        ZY_INLINE ConstSpan<Entry> GetEntries() const
        {
            return ConstSpan(mIndex.GetData<Entry>(), mCount);
        }

        /// \brief Gets the path a file was packed under.
        ///
        /// \param Entry The entry of the file, taken from this package.
        /// \return The path, relative to the folder that was packed and in its original case.
        ZY_INLINE Text GetName(ConstRef<Entry> Entry) const
        {
            return StrConvert(mIndex.GetData<Char>() + mCount * sizeof(Package::Entry) + Entry.Name);
        }

        /// \brief Finds a file by its path, whatever its case and whichever way its slashes lean.
        ///
        /// \param Path The path of the file, relative to the folder that was packed.
        /// \return The entry of the file, or `nullptr` when the package holds none by that path.
        ConstPtr<Entry> Find(Text Path) const;

        /// \brief Lends the bytes of a file where they lie, once they check against the index, from any thread.
        ///
        /// \param Entry  The entry of the file, taken from this package.
        /// \param Output Receives the bytes, which must not be written to, or nothing when they are not the ones packed.
        /// \return The result of the read, \ref Filesystem::Result::Invalid when the bytes fail their checksum.
        Filesystem::Result Read(ConstRef<Entry> Entry, Ref<Blob> Output) const;

    private:

        /// \brief The four characters every package opens with, read as one little-endian value.
        static constexpr UInt32 kMagic     = 'Z' | ('P' << 8) | ('A' << 16) | ('K' << 24);

        /// \brief The layout revision this class reads and writes.
        static constexpr UInt16 kVersion   = 1;

        /// \brief The boundary the bytes of every file start on.
        static constexpr UInt32 kAlignment = 16;

        /// \brief Represents what a package opens with, ahead of its entries and their names.
        struct Header final
        {
            /// The four characters every package opens with.
            UInt32 Magic;

            /// The layout revision the package was written in.
            UInt16 Version;

            /// The bytes left unused, which keep the entries on a boundary of eight.
            UInt16 Unused;

            /// The number of entries that follow the header.
            UInt32 Count;

            /// The number of bytes the names take after the entries.
            UInt32 Names;

            /// The signature of the entries and the names, under the key the package was packed with.
            UInt64 Seal;
        };

        /// \brief Represents the package file laid into memory, kept there until the last blob lent out of it is gone.
        struct Mapper final : Retainable<Mapper>
        {
            /// The file as it lies in memory.
            Filesystem::View Memory;

            /// \brief Takes the file out of memory.
            ZY_INLINE ~Mapper()
            {
                Filesystem::Unmap(Memory);
            }
        };

        /// \brief Reads and checks the header and the index of the file that was just opened.
        ///
        /// \param Key The key the index has to be sealed under.
        /// \return `true` if the index was read and holds, `false` if the file is not a package this class reads.
        Bool Load(Signer Key);

        /// \brief Checks that the index names nothing outside the package, so it can be read without further checks.
        ///
        /// \param Size  The size of the package file in bytes.
        /// \param Names The number of bytes the names take.
        /// \return `true` if every entry stays inside the file and the keys ascend, `false` otherwise.
        Bool Verify(UInt64 Size, UInt32 Names) const;

        /// \brief Gets the key a path is found by.
        ///
        /// \param Path The path of a file, in any case and with either slash.
        /// \return The hash of the path, lowercase and with forward slashes.
        static UInt64 GetKey(Text Path);

        /// \brief Lists every file under a folder, its subfolders included and hidden files left out.
        ///
        /// \param Directory The folder to list.
        /// \param Prefix    The path of the folder relative to the one being packed, empty or ending in a slash.
        /// \param Files     Receives the path of each file, relative to the folder being packed.
        /// \return `true` if every folder was listed, `false` if one could not be or a file is too large to pack.
        static Bool Gather(Text Directory, Text Prefix, Ref<Sequence<Filesystem::Path>> Files);

        /// \brief Takes one record of a folder being listed, descending into it when it is a folder itself.
        ///
        /// \param Directory The folder the record was found in.
        /// \param Prefix    The path of that folder relative to the one being packed, empty or ending in a slash.
        /// \param Record    The record to take.
        /// \param Files     Receives the path of each file, relative to the folder being packed.
        /// \param Complete  Receives `false` when the record could not be taken, and is left alone otherwise.
        /// \return `true` to go on listing, `false` to stop.
        static Bool Collect(
            Text                            Directory,
            Text                            Prefix,
            ConstRef<Filesystem::Record>    Record,
            Ref<Sequence<Filesystem::Path>> Files,
            Ref<Bool>                       Complete);

        /// \brief Checks whether a file of the folder being packed holds the very bytes of another already read.
        ///
        /// \param Folder The folder the files are read from.
        /// \param Name   The path of the file relative to the folder.
        /// \param Data   The bytes to compare the file against.
        /// \return `true` if the file holds the same bytes, `false` if it differs or cannot be read.
        static Bool Repeats(Text Folder, Text Name, ConstRef<Blob> Data);

        /// \brief Writes the files of a folder and then the index that names them into an open file.
        ///
        /// \param File   The file to write into, open for writing.
        /// \param Folder The folder the files are read from.
        /// \param Files  The path of each file relative to the folder, in the order they are laid out.
        /// \param Key    The key the index is sealed under.
        /// \return `true` if the package was written whole, `false` if a file could not be read or written.
        static Bool Store(ConstRef<Filesystem::Handle> File, Text Folder, ConstSpan<Filesystem::Path> Files, Signer Key);

    public:

        /// \brief Packs every file under a folder into a package, which is the same bytes for the same folder.
        ///
        /// \param Folder The folder to pack, whose hidden files are left out.
        /// \param Output The path of the package file to write, replaced if it exists.
        /// \param Key    The key to seal the package under, which whoever opens it has to hold.
        /// \return `true` if the package was written, `false` if it could not be, in which case none is left behind.
        static Bool Pack(Text Folder, Text Output, Signer Key = Signer());

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Mapper> mFile;
        Blob             mIndex;
        UInt32           mCount;
    };
}