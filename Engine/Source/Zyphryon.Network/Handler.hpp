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

#include "Connection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNetwork
{
    /// \brief Defines the interface the application implements to hear about one endpoint and its peers.
    class Handler : public Retainable<Handler>
    {
    public:

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Handler() = default;

        /// \brief Called when a peer becomes ready to exchange messages.
        ///
        /// \param Link   The peer that was opened.
        /// \param Origin The address the peer is at, or the one reached for when this side connected.
        virtual void OnConnect(Connection Link, ConstRef<Address> Origin)
        {
        }

        /// \brief Called when a peer stops exchanging messages, whichever side ended it.
        ///
        /// \param Link   The peer that was closed.
        /// \param Reason The reason the connection ended.
        virtual void OnDisconnect(Connection Link, Reason Reason)
        {
        }

        /// \brief Called for each message received from a peer.
        ///
        /// \param Link     The peer the message came from.
        /// \param Delivery The guarantee the message was sent under.
        /// \param Message  The payload, exactly as the sender wrote it.
        virtual void OnMessage(Connection Link, Delivery Delivery, ConstSpan<Byte> Message)
        {
        }
    };
}