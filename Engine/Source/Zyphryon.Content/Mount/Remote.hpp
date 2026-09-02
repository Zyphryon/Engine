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

        /// \brief How many requests are allowed to be waiting on an answer at once.
        static constexpr UInt32 kMaxInFlight = 6;

        /// \brief How many times a request that was refused for arriving too fast is sent again.
        static constexpr UInt32 kMaxAttempts = 4;

        /// \brief The status a server answers with when it is being asked for too much at once.
        static constexpr UInt32 kTooMany     = 429;

        /// \brief One request, either waiting its turn or waiting on an answer.
        struct Request final
        {
            /// The path being asked for, kept because a refused request is sent again.
            Str         Path;

            /// The callback that receives whatever comes back.
            OnRead      Callback;

            /// The mount that sent it, which every answer is handed back through.
            Ptr<Remote> Owner;

            /// How many times this has been sent.
            UInt32      Attempt;
        };

        /// \brief Sends whatever is waiting its turn, for as long as there is room to.
        void Pump();

        /// \brief Sends one request.
        ///
        /// \param Handle The request to send.
        void Send(Ptr<Request> Handle);

        /// \brief Answers a request and releases it, then sends whatever was waiting behind it.
        ///
        /// \param Handle The request being answered.
        /// \param Result What to answer with.
        /// \param Data   The bytes that came back, which the blob takes over, or nothing.
        /// \param Size   The number of bytes that came back.
        static void Close(Ptr<Request> Handle, Filesystem::Result Result, Ptr<Byte> Data, UInt32 Size);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str                    mAddress;
        Sequence<Ptr<Request>> mQueue;
        UInt32                 mActive;
    };
}