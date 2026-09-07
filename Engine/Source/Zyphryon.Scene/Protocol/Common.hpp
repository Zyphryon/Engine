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

#include "Zyphryon.Network/Connection.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Maximum number of component types that can be replicated, one bit each in a dirty mask.
    inline constexpr UInt8  kMaxComponents = 128;

    /// \brief Bit set on the identifier of a replica both sides already hold from disk, so it is never spawned.
    inline constexpr UInt64 kPersistent    = 1ull << 63;

    /// \brief First message opcode left to the application, every one below it belonging to the protocol.
    inline constexpr UInt8  kMaxOpcode     = 16;

    /// \brief Specifies how a replicated component travels: who receives it and under which guarantee.
    enum class Replication : UInt8
    {
        None      = 0,       				///< Nothing, which no declaration should state.
        Self      = 1 << 0,  				///< The peer that owns the entity receives it.
        Others    = 1 << 1,  				///< Every peer but the one that owns the entity receives it.
        Once      = 1 << 2,  				///< Sent when the entity is spawned and never again, so touching it is a no-op.
        Streamed  = 1 << 3,  				///< Sent unreliably whenever it changes, latest wins, for state that moves every tick.

        Everyone  = Self     | Others, 	 	///< Every peer that sees the entity receives it, reliably, as it changes.
        Streaming = Everyone | Streamed,	///< Every peer receives it unreliably as it moves, latest wins.
        Spawned   = Everyone | Once,        ///< Every peer receives it with the spawn and never again.
        Mirrored  = Others   | Streamed,  	///< Everyone but the owner receives it as it moves, the owner being its source.
    };
    ZY_DEFINE_BITWISE_ENUM(Replication)

    /// \brief Specifies the messages the protocol exchanges, each the first byte of what it sends.
    enum class Opcode : UInt8
    {
        Hello,  ///< The publisher greets a peer with the manifest hash, the world stamp and the peer's own key.
        World,  ///< The replicated singletons that changed, or all of them when a peer is greeted.
        Spawn,  ///< An entity a peer now sees, with its archetype, owner and the components it owns.
        Forget, ///< An entity a peer no longer sees, whether it was destroyed or left the peer's scopes.
        Update, ///< The reliable components of one entity that changed or were removed.
        Stream, ///< The streamed components of many entities that changed, under one sequence number.
    };

    /// \brief Names a set of replicated components, one bit per place in the manifest.
    using Mask = Bitset<kMaxComponents>;

    /// \brief Gets the bit a place in the manifest occupies in every mask.
    ///
    /// \param Index The place in the manifest.
    /// \return The bit.
    ZY_INLINE constexpr Mask BitOf(UInt8 Index)
    {
        Mask Result;
        Result.Set(Index);
        return Result;
    }

    /// \brief Derives the key a peer is known by on both sides of a connection.
    ///
    /// \param Link The connection naming the peer.
    /// \return The key, which the publisher writes into the greeting so the peer can recognise itself.
    ZY_INLINE constexpr UInt64 KeyOf(Network::Connection Link)
    {
        return (static_cast<UInt64>(Link.GetChannel().GetValue()) << 32)
              | static_cast<UInt64>(Link.GetPeer().GetValue());
    }
}