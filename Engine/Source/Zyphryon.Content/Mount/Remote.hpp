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

struct emscripten_fetch_t;

namespace ZyContent
{
    /// \brief Represents a virtual filesystem mount backed by a location this machine has to ask another one for.
    class ZY_API Remote final : public Mount
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

        /// \brief The status a server answers with when the file is not there.
        static constexpr UInt32 kMissing     = 404;

        /// \brief Represents one request, either waiting its turn or waiting on an answer.
        struct Request final
        {
            /// The path being asked for, kept because a refused request is sent again.
            Str              Path;

            /// The callback that receives whatever comes back.
            OnRead           Callback;

            /// The mount that sent it, kept alive so an answer arriving after it was removed still has it.
            Retainer<Remote> Owner;

            /// The number of times this has been sent.
            UInt32           Attempt;
        };

        /// \brief Sends whatever is waiting its turn, for as long as there is room to.
        void Pump();

        /// \brief Sends one request.
        ///
        /// \param Handle The request to send.
        void Send(Ptr<Request> Handle);

        /// \brief Answers a request whose fetch came back with the file.
        ///
        /// \param Fetch The fetch that finished, which is closed here.
        static void OnSuccess(Ptr<emscripten_fetch_t> Fetch);

        /// \brief Answers a request whose fetch failed, or sends it again when the server asked to slow down.
        ///
        /// \param Fetch The fetch that failed, which is closed here.
        static void OnFailure(Ptr<emscripten_fetch_t> Fetch);

        /// \brief Answers a request and releases it, then sends whatever was waiting behind it.
        ///
        /// \param Handle The request being answered.
        /// \param Result The result to answer with.
        /// \param Data   The bytes that came back, or an empty blob.
        static void Close(Ptr<Request> Handle, Filesystem::Result Result, AnyRef<Blob> Data);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str                    mAddress;
        Sequence<Ptr<Request>> mQueue;
        UInt32                 mActive;
    };
}