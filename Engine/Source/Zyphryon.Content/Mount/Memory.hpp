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

#include "Zyphryon.Content/Mount.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Virtual mount that serves read-only resources from in-memory, typically compiled-in, buffers.
    class Memory final : public Mount
    {
    public:

        /// \brief Associates a logical path with the bytes served for it.
        struct Entry final
        {
            /// The logical path within the mount.
            Text            Path;

            /// The bytes served for the path.
            ConstSpan<Byte> Data;
        };

    public:

        /// \brief Constructs a memory mount over a set of embedded entries.
        ///
        /// \param Entries The resource entries to serve; their buffers must outlive the mount.
        explicit Memory(ConstSpan<Entry> Entries);

        /// \see Mount::IsAsynchronous
        Bool IsAsynchronous() const override;

        /// \see Mount::Enumerate
        void Enumerate(Text Path, AnyRef<OnEnumerate> Callback) const override;

        /// \see Mount::Delete
        void Delete(Text Path, AnyRef<OnResult> Callback) override;

        /// \see Mount::Copy
        void Copy(Text Source, Text Destination, AnyRef<OnResult> Callback) override;

        /// \see Mount::Read
        void Read(Text Path, AnyRef<OnRead> Callback) override;

        /// \see Mount::Write
        void Write(Text Path, AnyRef<Blob> Bytes, AnyRef<OnResult> Callback) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<UInt64, Entry> mEntries;
    };
}
