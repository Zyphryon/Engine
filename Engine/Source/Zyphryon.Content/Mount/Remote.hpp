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
    /// \brief Virtual filesystem mount backed by a location this machine has to ask another one for.
    class Remote final : public Mount
    {
    public:

        /// \brief Constructs a mount rooted at the given address.
        ///
        /// \param Address The address every path is resolved against, with or without a trailing separator.
        explicit Remote(Text Address);

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

        /// \brief One request that has been sent and not yet answered.
        struct Request final
        {
            /// The callback that receives whatever comes back.
            OnRead   Callback;

            /// The mount that sent it, which the completion is handed back through.
            Ptr<Remote> Owner;
        };

        /// \brief Answers a request that came back with bytes, and releases it.
        ///
        /// \param Handle  The request being answered.
        /// \param Data    The bytes that came back, which the blob takes over.
        /// \param Size    The number of bytes that came back.
        static void OnSucceed(Ptr<Request> Handle, Ptr<Byte> Data, UInt32 Size);

        /// \brief Answers a request that came back with nothing, and releases it.
        ///
        /// \param Handle The request being answered.
        /// \param Status The status it came back with, which says why there is nothing.
        static void OnFail(Ptr<Request> Handle, UInt32 Status);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str mAddress;
    };
}