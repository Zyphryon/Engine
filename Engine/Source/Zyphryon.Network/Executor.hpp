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

#include "Channel.hpp"
#include "Zyphryon.Platform/Timer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Represents the thread every endpoint runs on, and the queues the application speaks to it through.
    class Executor final
    {
    public:

        /// \brief Represents one endpoint to open, as it stands once its address has been resolved.
        struct Request final
        {
            /// The connection naming the endpoint, taken before the address was resolved.
            Connection        Link;

            /// The transport the endpoint carries.
            Transport         Transport = Transport::TCP;

            /// Whether the endpoint accepts peers rather than reaching for one.
            Bool              Server    = false;

            /// The resolved address the endpoint is opened against.
            Endpoint          Remote;

            /// The address as the application wrote it down, kept so the endpoint can report where it is.
            Address           Origin;

            /// The handler the endpoint carries for whoever speaks for it.
            Retainer<Handler> Listener;
        };

    public:

        /// \brief Destroys the executor, stopping its thread and closing whatever it still holds.
        ~Executor();

        /// \brief Opens the platform's completion queue and starts the thread the endpoints run on.
        ///
        /// \return `true` if endpoints can now be opened, `false` otherwise.
        Bool Start();

        /// \brief Stops the thread and ends every endpoint still open.
        void Stop();

        /// \brief Writes down an endpoint for the next pass to open.
        ///
        /// \param Entry The endpoint to open.
        void Open(AnyRef<Request> Entry);

        /// \brief Writes down a message for a peer.
        ///
        /// \param Link     The peer to send to, or the endpoint itself.
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied and may be released as soon as this returns.
        void Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message);

        /// \brief Writes down a message for every peer of an endpoint.
        ///
        /// \param Link     The endpoint whose peers receive the message.
        /// \param Delivery The guarantee the message needs.
        /// \param Message  The payload, which is copied once per peer.
        void Broadcast(Connection Link, Delivery Delivery, ConstSpan<Byte> Message);

        /// \brief Writes down that a peer, or the endpoint it belongs to, is to end.
        ///
        /// \param Link The peer to end, or the endpoint.
        void Close(Connection Link);

        /// \brief Writes down how long a peer, or every peer of an endpoint, may say nothing before it is taken as gone.
        ///
        /// \param Link    The peer to set it on, or the endpoint and every peer it holds.
        /// \param Seconds The time silence is put up with, where zero never gives up on it at all.
        void SetTimeout(Connection Link, Real64 Seconds);

        /// \brief Hands over everything the endpoints have had to say, and wakes them for whatever was written down.
        ///
        /// \param Output Receives everything the endpoints had to say, traded for whatever it was holding.
        void Drain(Ref<Mailbox> Output);

    private:

        /// \brief Maximum time, in milliseconds, a pass waits before it gives the endpoints their turn regardless.
        static constexpr UInt32 kMaxIdle     = 100;

        /// \brief Number of passes a teardown waits before saying that the platform has stopped answering.
        static constexpr UInt32 kMaxPatience = 50;

        /// \brief Specifies what the application asked the endpoints to do.
        enum class Order : UInt8
        {
            Send,      ///< Hand a message to one peer.
            Broadcast, ///< Hand a message to every peer of an endpoint.
            Close,     ///< End a peer, or the endpoint it belongs to.
            Timeout,   ///< Set how long a peer, or every peer of an endpoint, may say nothing for.
        };

        /// \brief Represents one thing the application asked for, and where in the arena its payload sits.
        struct Command final
        {
            /// The connection the order concerns.
            Connection Link;

            /// What was asked for.
            Order      Kind   = Order::Close;

            /// The guarantee a message was sent under, meaningless for anything else.
            Delivery   Mode   = Delivery::Unreliable;

            /// The offset into the arena where the payload begins.
            UInt32     Offset = 0;

            /// The length of the payload, in bytes.
            UInt32     Size   = 0;
        };

        /// \brief Represents what the application has asked for and not yet had carried out.
        struct Outbox final
        {
            /// What the application asked for, in the order it asked.
            Sequence<Command> Entries;

            /// The payloads those orders carry, one behind the other.
            Sequence<Byte>    Arena;

            /// \brief Gets the payload one order carries.
            ///
            /// \param Entry The order to read.
            /// \return The payload, which is empty for anything that carries none.
            ZY_INLINE ConstSpan<Byte> GetPayload(ConstRef<Command> Entry) const
            {
                return ConstSpan(Arena.GetData() + Entry.Offset, Entry.Size);
            }

            /// \brief Drops everything recorded, keeping the room it was recorded into.
            ZY_INLINE void Clear()
            {
                Entries.Clear();
                Arena.Clear();
            }
        };

    private:

        /// \brief Takes what the platform finished, carries out what was asked for, and gives every endpoint its turn.
        void Run();

        /// \brief Writes down one thing for the endpoints to do on their next pass.
        ///
        /// \param Link    The connection the order concerns.
        /// \param Kind    The order being asked for.
        /// \param Mode    The guarantee a message needs, ignored for anything else.
        /// \param Message The payload, which is copied into the arena.
        void Post(Connection Link, Order Kind, Delivery Mode, ConstSpan<Byte> Message);

        /// \brief Hands one wait's worth of finished operations to whichever endpoints posted them.
        ///
        /// \param Time  The current time, in seconds.
        /// \param Ready The operations that finished.
        void Dispatch(Real64 Time, ConstSpan<Proactor::Event> Ready);

        /// \brief Opens every endpoint written down since the last pass.
        ///
        /// \param Batch The endpoints to open.
        void Admit(Ref<Sequence<Request>> Batch);

        /// \brief Carries out everything the application asked for since the last pass.
        ///
        /// \param Batch The orders to carry out.
        void Apply(Ref<Outbox> Batch);

        /// \brief Waits until the platform has given back every operation still out on the endpoints.
        void Settle();

        /// \brief Gives every endpoint its moment, and lets go of the ones that ended and have gone quiet.
        ///
        /// \param Time The current time, in seconds.
        void Sweep(Real64 Time);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Thread                                  mWorker;
        Platform::Timer                         mClock;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mutex                                   mMutex;
        Outbox                                  mPending;
        Sequence<Request>                       mRequests;
        Mailbox                                 mIncoming;

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Pool<Retainer<Channel>, kMaxEndpoints>  mChannels;
        Proactor                                mWatcher;
        Mailbox                                 mReport;
        Outbox                                  mScratch;
        Sequence<Request>                       mAdmit;
    };
}