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

#include "Session.hpp"
#include "Zyphryon.Network/Channel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    /// \brief Represents a connection over datagrams, pointed at the one peer it speaks to.
    class Client final : public Channel
    {
    public:

        /// \brief Constructs a datagram that will reach for a peer.
        ///
        /// \param Link     The connection naming this datagram.
        /// \param Address  The address it reaches for.
        /// \param Listener The handler told about it.
        Client(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener);

        /// \brief Destroys the datagram, closing whatever socket it still holds.
        ~Client() override;

        /// \see Channel::Verify(Connection)
        Bool Verify(Connection Link) const override;

        /// \see Channel::Open(Ref<Proactor>, ConstRef<Endpoint>)
        Bool Open(Ref<Proactor> Watcher, ConstRef<Endpoint> Address) override;

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

        /// \brief Posts the write that puts the next packet out, if one is not out already.
        ///
        /// \param Watcher The proactor the write is posted to.
        /// \param Time    The current time, in seconds.
        void Drain(Ref<Proactor> Watcher, Real64 Time);

        /// \brief Posts the read that takes the next datagram, if one is not out already.
        ///
        /// \param Watcher The proactor the read is posted to.
        void Fill(Ref<Proactor> Watcher);

        /// \brief Acts on a read that finished, delivering what arrived and posting the next.
        ///
        /// \param Watcher The proactor the next read is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Size    The number of bytes the read moved, where none is a datagram that said nothing.
        /// \param Output  Receives the message, unless the datagram carried something this build cannot read.
        void OnReceive(Ref<Proactor> Watcher, Real64 Time, UInt32 Size, Ref<Mailbox> Output);

        /// \brief Acts on a write that finished, posting whatever datagram waits behind it.
        ///
        /// \param Watcher The proactor the next write is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Size    The number of bytes the write moved.
        void OnSend(Ref<Proactor> Watcher, Real64 Time, UInt32 Size);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Socket                       mSocket;
        Sequence<Byte, kMaxRead>     mInbound;
        Sequence<Byte, kMaxDatagram> mTransmit;
        Session                      mSession;
        UInt64                       mCookie;
        Bool                         mAnswering;
    };
}