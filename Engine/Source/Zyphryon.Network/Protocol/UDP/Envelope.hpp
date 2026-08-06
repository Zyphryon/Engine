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

namespace Network::UDP
{
    /// \brief Number of bytes the envelope takes at the front of every datagram.
    inline constexpr UInt kEnvelope  = 1;

    /// \brief Number of bytes the proof an address is asked for takes.
    inline constexpr UInt kCookie    = sizeof(UInt64);

    /// \brief Number of bytes a challenge or an answer takes.
    inline constexpr UInt kChallenge = kEnvelope + kCookie;

    /// \brief Specifies what a datagram carries, which the byte it opens with says.
    enum class Envelope : UInt8
    {
        Data,      ///< Whatever follows is a record's own packet.
        Challenge, ///< The endpoint asks whoever sent to show that the address it sent from reaches it.
        Answer,    ///< What was asked for, sent back unchanged.
    };
}