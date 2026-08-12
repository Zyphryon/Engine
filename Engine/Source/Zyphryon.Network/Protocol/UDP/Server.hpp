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

#include "Sentry.hpp"
#include "Session.hpp"
#include "Zyphryon.Network/Channel.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::UDP
{
    /// \brief Represents an endpoint that accepts peers over datagrams, and owns every peer it accepted.
    class Server final : public Channel
    {
    public:

        /// \brief Constructs an endpoint that will accept peers.
        ///
        /// \param Link     The connection naming this endpoint.
        /// \param Address  The address it accepts on.
        /// \param Listener The handler told about its peers.
        Server(Connection Link, ConstRef<Address> Address, ConstRetainer<Handler> Listener);

        /// \brief Destroys the endpoint, closing its socket and every peer it accepted.
        ~Server() override;

        /// \see Channel::IsQuiet()
        Bool IsQuiet() const override;

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

        /// \brief Maximum number of packets the endpoint may have on their way at once.
        static constexpr UInt kMaxFlight = 32;

        /// \brief Represents one packet on its way out, which outlives the peer that asked for it.
        struct Flight final
        {
            /// The bytes the platform is sending, which stay put until it says it is finished with them.
            Sequence<Byte, kMaxDatagram> Data;

            /// The peer it went to, kept so what it cost is put against the right one when it lands.
            Connection::Peer             Peer;
        };

        /// \brief Represents one peer this endpoint speaks to, which owns no socket of its own.
        struct Peer final
        {
            /// The connection naming this peer.
            Connection Link;

            /// Where the peer is, which is what a datagram is matched against to find whose it is.
            Endpoint   Remote;

            /// Where the peer is, as it would be written down.
            Address    Origin;

            /// What it is owed and what it has been told arrived.
            Session    Session;

            /// When anything last arrived from it, which is what a lapse is measured from.
            Real64     Freshness = 0.0;
        };

        /// \brief Finds the peer an address belongs to, without taking on one it does not.
        ///
        /// \param Origin The address a datagram arrived from.
        /// \return The place the peer sits, or zero when no peer is known at that address.
        Connection::Peer Locate(ConstRef<Endpoint> Origin) const;

        /// \brief Takes on the peer at an address, which is only ever reached once the address has been shown.
        ///
        /// \param Origin The address to take a peer on at.
        /// \param Output Receives a notice for the peer if it was taken on.
        /// \return The place the peer sits, or zero when there was no room for another.
        Connection::Peer Admit(ConstRef<Endpoint> Origin, Ref<Mailbox> Output);

        /// \brief Asks an address to show that it reaches whoever sent from it, writing nothing down either way.
        ///
        /// \param Watcher The proactor the write is posted to.
        /// \param Origin  The address to ask.
        void Question(Ref<Proactor> Watcher, ConstRef<Endpoint> Origin);

        /// \brief Reads a datagram into the peer it came from, handing over whatever it carried.
        ///
        /// \param Watcher The proactor the peer is given up on through, should it have to be.
        /// \param Time    The current time, in seconds.
        /// \param Slot    The place the peer sits.
        /// \param Size    The number of bytes the datagram carried.
        /// \param Output  Receives the messages, and a notice should the peer be let go of.
        void Absorb(Ref<Proactor> Watcher, Real64 Time, Connection::Peer Slot, UInt32 Size, Ref<Mailbox> Output);

        /// \brief Takes a peer out of the table it is found by, leaving its slot to whoever frees it.
        ///
        /// \param Entry The peer to forget.
        void Forget(ConstRef<Peer> Entry);

        /// \brief Posts a write for every peer with something waiting, up to what the endpoint may have out at once.
        ///
        /// \param Watcher The proactor the writes are posted to.
        /// \param Time    The current time, in seconds.
        void Drain(Ref<Proactor> Watcher, Real64 Time);

        /// \brief Posts the write that puts one peer's next packet out, if it has one and there is room for it.
        ///
        /// \param Watcher The proactor the write is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Slot    The place the peer sits.
        /// \param Entry   The peer to take the packet from.
        /// \return `true` if a packet went out, `false` if the peer had none or there was no room left.
        Bool Push(Ref<Proactor> Watcher, Real64 Time, Connection::Peer Slot, Ref<Peer> Entry);

        /// \brief Posts the read that takes the next datagram, if one is not out already.
        ///
        /// \param Watcher The proactor the read is posted to.
        void Fill(Ref<Proactor> Watcher);

        /// \brief Ends the endpoint itself, giving up on its socket and announcing every peer it was carrying.
        ///
        /// \param Watcher The proactor whatever is still outstanding is given up on through.
        /// \param Cause   The reason it is retired.
        /// \param Output  Receives a notice for every peer the endpoint was carrying.
        void Fold(Ref<Proactor> Watcher, Reason Cause, Ref<Mailbox> Output);

        /// \brief Acts on a read that finished, delivering what arrived to its peer and posting the next.
        ///
        /// \param Watcher The proactor the next read is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Origin  The address the datagram arrived from.
        /// \param Size    The number of bytes the read moved, where none is a datagram that said nothing.
        /// \param Output  Receives the message, and a notice for the peer if it was taken on for it.
        void OnReceive(Ref<Proactor> Watcher, Real64 Time, ConstRef<Endpoint> Origin, UInt32 Size, Ref<Mailbox> Output);

        /// \brief Acts on a write that finished, taking its room back and posting whatever waits behind it.
        ///
        /// \param Watcher The proactor the next write is posted to.
        /// \param Time    The current time, in seconds.
        /// \param Event   The write that finished.
        void OnSend(Ref<Proactor> Watcher, Real64 Time, ConstRef<Proactor::Event> Event);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Socket                          mSocket;
        Sentry                          mSentry;
        Pool<Peer, kMaxPeers>           mPeers;
        Pool<Flight, kMaxFlight>        mFlights;
        Table<Digest, Connection::Peer> mRegistry;
        Sequence<Byte, kMaxRead>        mInbound;
        Real64                          mClock;
        UInt                            mCursor;
    };
}