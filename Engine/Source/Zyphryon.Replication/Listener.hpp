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

#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Defines the interface the game implements to hear about a session's peers and their messages.
    class Listener : public Retainable<Listener>
    {
    public:

        /// \brief Destructor for proper cleanup in derived classes.
        virtual ~Listener() = default;

        /// \brief Called when a peer joins the session this side serves, or when this side reaches the serving one.
        ///
        /// \param Peer The key the peer is known by, or zero on the following side for the serving one.
        virtual void OnJoin(UInt64 Peer)
        {
        }

        /// \brief Called when a peer leaves the session this side serves, or when the serving side goes away.
        ///
        /// \param Peer The key the peer is known by, or zero on the following side for the serving one.
        virtual void OnLeave(UInt64 Peer)
        {
        }

        /// \brief Called for each message a peer sends the side serving the session, or the serving side sends this one.
        ///
        /// \param Peer    The key the peer is known by, as \ref OnJoin named it, or zero for the serving side.
        /// \param Message The payload, exactly as the peer wrote it.
        virtual void OnMessage(UInt64 Peer, ConstSpan<Byte> Message)
        {
        }

        /// \brief Called on the following side when what it plays arrives under a persistent replica not loaded.
        ///
        /// \param Identifier The identifier of the persistent replica it waits for, which lands it once loaded.
        virtual void OnAwait(UInt64 Identifier)
        {
        }
    };
}