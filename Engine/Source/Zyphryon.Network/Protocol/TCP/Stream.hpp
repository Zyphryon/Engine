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

#include "Pipe.hpp"
#include "Zyphryon.Network/Channel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::TCP
{
    /// \brief Represents a connection over a stream, whether it was reached for or accepted.
    class Stream final : public Channel
    {
    public:

        /// \brief Constructs a stream that will reach for a peer.
        ///
        /// \param Link     The connection naming this stream.
        /// \param Address  The address it reaches for.
        /// \param Listener The handler told about it.
        Stream(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener);

        /// \brief Constructs a stream around a socket the platform has already connected.
        ///
        /// \param Link     The connection naming this stream.
        /// \param Address  The address the peer arrived from.
        /// \param Listener The handler told about it.
        /// \param Socket   The socket accepted for this peer.
        Stream(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener, ConstRef<Socket> Socket);

        /// \brief Destroys the stream, closing whatever socket it still holds.
        ~Stream() override;

        /// \see Channel::Verify(Connection)
        Bool Verify(Connection Link) const override;

        /// \see Channel::Open(Ref<Proactor>, ConstRef<Endpoint>)
        Bool Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address) override;

        /// \brief Puts the socket this stream was accepted on under the proactor and begins reading from it.
        ///
        /// \param Watcher The proactor the read is posted to.
        /// \return `true` when the stream is carrying traffic, otherwise `false`.
        Bool Adopt(Ref<Proactor> Watcher);

        /// \see Channel::Complete(Ref<Proactor>, Real64, ConstRef<Proactor::Event>, Ref<Mailbox>)
        void Complete(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event, Ref<Mailbox> Output) override;

        /// \see Channel::Tick(Ref<Proactor>, Real64, Ref<Mailbox>)
        void Tick(Ref<Proactor> Watcher, Real64 Time, Ref<Mailbox> Output) override;

        /// \see Channel::Send(Connection, Delivery, ConstSpan<Byte>)
        void Send(Connection Link, Delivery Delivery, ConstSpan<Byte> Message) override;

        /// \see Channel::Broadcast(Delivery, ConstSpan<Byte>)
        void Broadcast(Delivery Delivery, ConstSpan<Byte> Message) override;

        /// \see Channel::Retire(Ref<Proactor>, Connection, Reason, Ref<Mailbox>)
        void Retire(Ref<Proactor> Watcher, Connection Link, Reason Cause, Ref<Mailbox> Output) override;

        /// \see Channel::Probe(Connection, Ref<Statistics>, Ref<Address>)
        void Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const override;

    private:

        /// \brief Maximum time, in seconds, an attempt to reach a peer may take before it is taken as unreachable.
        static constexpr Real64 kMaxHandshake = 10.0;

        /// \brief Maximum time, in seconds, a queue may hold room it has stopped needing before it gives it back.
        static constexpr Real64 kMaxHoard     = 5.0;

        /// \brief Specifies what a frame carries, which the byte opening every one of them says.
        enum class Tag : UInt8
        {
            Message, ///< The rest of the frame is a payload for the application.
            Ping,    ///< Asks the peer to answer, so one gone quiet can be told from one that has stopped answering.
            Pong,    ///< Answers a ping.
        };

        /// \brief Posts the write that empties the outbound queue, if one is not out already.
        ///
        /// \param Watcher The proactor the write is posted to.
        void Drain(Ref<Proactor> Watcher);

        /// \brief Posts the read that fills the back of the inbound queue, if one is not out already.
        ///
        /// \param Watcher The proactor the read is posted to.
        void Fill(Ref<Proactor> Watcher);

        /// \brief Acts on the attempt that settled, opening the stream and posting its first traffic.
        ///
        /// \param Watcher The proactor the first read and write are posted to.
        /// \param Output  Receives the notice that the stream is open.
        void OnConnect(Ref<Proactor> Watcher, Ref<Mailbox> Output);

        /// \brief Acts on a read that finished, delivering what it completed and posting the next.
        ///
        /// \param Watcher The proactor the next read is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Size    The number of bytes the read moved, where none means the peer closed.
        /// \param Output  Receives every message completed, and the disconnect if the peer closed.
        void OnReceive(Ref<Proactor> Watcher, Real64 Time, UInt32 Size, Ref<Mailbox> Output);

        /// \brief Acts on a write that finished, taking it off the transmit queue and posting the next.
        ///
        /// \param Watcher The proactor the next write is posted to.
        /// \param Size    The number of bytes the write moved.
        void OnSend(Ref<Proactor> Watcher, UInt32 Size);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Socket     mSocket;
        Pipe       mInbound;
        Pipe       mOutbound;
        Pipe       mTransmit;
        Statistics mStats;
        Real64     mExpiry;
        Real64     mProbe;
        Real64     mCompact;
    };
}