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

#include "Stream.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network::TCP
{
    /// \brief Represents an endpoint that accepts peers over a stream, and owns every peer it accepted.
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

        /// \see Channel::SetTimeout(Connection, Real64)
        void SetTimeout(Connection Link, Real64 Seconds) override;

        /// \see Channel::Probe(Connection, Ref<Statistics>, Ref<Address>)
        void Probe(Connection Link, Ref<Statistics> Metrics, Ref<Address> Origin) const override;

    private:

        /// \brief Gets the peer a connection names.
        ///
        /// \param Link The connection naming the peer to find, or this endpoint.
        /// \return A pointer to the peer, or `nullptr` when the connection names none of them.
        Ptr<Stream> Locate(Connection Link) const;

        /// \brief Posts the accept that invites the next peer in, if one is not out already.
        ///
        /// \param Watcher The proactor the accept is posted to.
        void Invite(Ref<Proactor> Watcher);

        /// \brief Takes on the peer an accept produced, giving it a slot and a connection of its own.
        ///
        /// \param Watcher The proactor the peer's own operations are posted to.
        /// \param Taken   The socket the peer arrived on.
        /// \param Origin  The address it arrived from.
        /// \param Output  Receives a notice for the peer if it was taken on.
        void Admit(Ref<Proactor> Watcher, ConstRef<Socket> Taken, ConstRef<Endpoint> Origin, Ref<Mailbox> Output);

        /// \brief Ends the endpoint itself, and every peer with it, since none can outlive the listener.
        ///
        /// \param Watcher The proactor whatever is still outstanding is given up on through.
        /// \param Cause   The reason it is retired.
        /// \param Output  Receives the notices for the peers as each is retired.
        void Fold(Ref<Proactor> Watcher, Reason Cause, Ref<Mailbox> Output);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Socket                            mSocket;
        Endpoint                          mLocal;
        Pool<Retainer<Stream>, kMaxPeers> mPeers;
    };
}