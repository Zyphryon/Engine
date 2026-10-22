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
    /// \brief The section key of the global channel, which carries the world's singletons.
    inline constexpr UInt64 kGlobal  = 0;

    /// \brief The section key of a peer's private channel, which carries what only an owner receives.
    inline constexpr UInt64 kPrivate = kMaximum<UInt64>;

    /// \brief Specifies the messages the protocol writes, each opening with its opcode.
    enum class Opcode : UInt8
    {
        Frame,   ///< Opens a frame with the publisher's clock.
        Hello,   ///< Greets a peer with the manifest hash and the key it is known by.
        Leave,   ///< Tells a peer it no longer sees a key.
        Section, ///< Opens the messages of one channel.
        Enter,   ///< Gives a peer an entity whole, creating or overwriting it.
        Update,  ///< Gives a peer what changed on an entity and what was removed.
        Forget,  ///< Tells a peer an entity left its key, then the key it moved under or zero once destroyed.
        Message, ///< Hands peers the game's message, the peer that passes over it and its size first.
        Packed,  ///< Opens a frame packed with LZ4, its unpacked size first, which never appears inside a frame.
    };
}