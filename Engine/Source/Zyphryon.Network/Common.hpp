// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Network
{
    /// \brief Maximum number of endpoints an application may open at once.
    inline constexpr UInt16 kMaxEndpoints = 128;

    /// \brief Maximum number of peers a single endpoint can accept.
    inline constexpr UInt16 kMaxPeers     = 4096;

    /// \brief Maximum size, in bytes, of the payload carried by a single unreliable message.
    inline constexpr UInt16 kMaxDatagram  = 1200;

    /// \brief Maximum size, in bytes, a peer may have waiting to go out before anything further is dropped.
    inline constexpr UInt32 kMaxBacklog   = 32 * 1024;

    /// \brief Maximum size, in bytes, a peer may announce a single framed message to be.
    inline constexpr UInt32 kMaxFrame     = 1024 * 1024;

    /// \brief Maximum size, in bytes, taken from a socket in one read.
    inline constexpr UInt16 kMaxRead      = 8 * 1024;

    /// \brief Maximum number of finished operations collected from the platform in one wait.
    inline constexpr UInt16 kMaxHarvest   = 256;

    /// \brief Specifies the delivery guarantees a message can be sent under.
    enum class Delivery : UInt8
    {
        Unreliable, ///< May be dropped or reordered, superseded by the next message.
        Unordered,  ///< Always arrives, in no particular order relative to its neighbours.
        Reliable,   ///< Always arrives, in particular order relative to its neighbours.
    };

    /// \brief Specifies what an operation handed to the platform was asked to do.
    enum class Operation : UInt8
    {
        Rouse,   ///< Nothing was asked for, and the wait was ended by the application rather than by a socket.
        Accept,  ///< Take on the next peer reaching for a socket that accepts.
        Connect, ///< Reach for a peer, which settles either way.
        Receive, ///< Fill a buffer with whatever arrives.
        Send,    ///< Empty a buffer onto the wire.
    };

    /// \brief Specifies the reasons a connection can end.
    enum class Reason : UInt8
    {
        None,           ///< Nothing went wrong, which is what a socket still carrying traffic answers.
        Closed,         ///< Either side closed the connection deliberately.
        Refused,        ///< The peer was reachable and declined the connection.
        Unreachable,    ///< The peer could not be reached.
        Timeout,        ///< The peer stopped answering and the connection lapsed.
        Protocol,       ///< The peer sent data this build cannot decode.
        Congested,      ///< The peer stopped reading and fell too far behind to be kept waiting for.
    };

    /// \brief Specifies the transports a connection can carry its traffic over.
    enum class Transport : UInt8
    {
        TCP,    ///< Reliable ordered stream, at the cost of head-of-line blocking.
        UDP,    ///< Unordered datagrams, where each message states its own guarantees.
    };

    /// \brief Represents a host and the port used to reach it.
    struct Address final
    {
        /// The host, either a name to resolve or a literal address in IPv4 or IPv6 notation.
        Str    Host;

        /// The port, in host byte order.
        UInt16 Port = 0;

        /// \brief Checks whether the address specifies a port.
        ///
        /// \return `true` if the address is worth attempting, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return Port > 0;
        }

        /// \brief Checks whether the address refers to every local interface rather than naming one.
        ///
        /// \return `true` if no host is specified, `false` otherwise.
        ZY_INLINE Bool IsAny() const
        {
            return Host.IsEmpty();
        }
    };

    /// \brief Represents what a connection has cost and how well it is holding up.
    struct Statistics final
    {
        /// The round trip to the peer and back, in milliseconds.
        Real32 Latency  = 0.0f;

        /// The share of sent messages the peer never acknowledged, from zero to one.
        Real32 Loss     = 0.0f;

        /// The bytes handed over and not yet given to the transport.
        UInt64 Pending  = 0;

        /// The total bytes handed to the transport, payload and framing together.
        UInt64 Sent     = 0;

        /// The total bytes taken from the transport, payload and framing together.
        UInt64 Received = 0;
    };
}