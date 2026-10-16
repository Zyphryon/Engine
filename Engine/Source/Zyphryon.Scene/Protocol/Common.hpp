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

#include "Zyphryon.Scene/Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::Protocol
{
    /// \brief The most component types that can replicate, one bit each in a mask.
    inline constexpr UInt8  kMaxComponents = 128;

    /// \brief The byte that closes the present and the absent entries of a set, past every wire index.
    inline constexpr UInt8  kEnd           = 0xFF;

    /// \brief The first opcode left to the game, every one below it being the protocol's.
    inline constexpr UInt8  kMaxOpcode     = 16;

    /// \brief The size a section is cut at, always between two messages.
    inline constexpr UInt32 kMaxChunk      = 32 * 1024;

    /// \brief The largest frame a packed one may unpack to, which is the largest the network carries.
    inline constexpr UInt32 kMaxUnpacked   = 1024 * 1024;

    /// \brief The bit set on the identifier of a replica both sides load from disk.
    inline constexpr UInt64 kPersistent    = 1ull << 63;

    /// \brief The section key of the global channel, which carries the world's singletons.
    inline constexpr UInt64 kGlobal        = 0;

    /// \brief The section key of a peer's private channel, which carries what only an owner receives.
    inline constexpr UInt64 kPrivate       = ~0ull;

    /// \brief The seconds of real time between two frames to a peer that has nothing new.
    inline constexpr Real64 kHeartbeat     = 1.0;

    /// \brief Specifies who receives a replicated component and when.
    enum class Replication : UInt8
    {
        None     = 0,      ///< Nothing, which no declaration should state.
        Everyone = 1 << 0, ///< Every peer that sees the entity.
        Owner    = 1 << 1, ///< Only the peer that owns the entity.
        Others   = 1 << 2, ///< Every peer that sees the entity, while its owner ignores it.
        Once     = 1 << 3, ///< Only when the entity arrives, never as an update.
    };
    ZY_DEFINE_BITWISE_ENUM(Replication)

    /// \brief Specifies the messages the protocol writes, each opening with its opcode.
    enum class Opcode : UInt8
    {
        Frame,   ///< Opens a frame with the publisher's clock.
        Hello,   ///< Greets a peer with the manifest hash and the key it is known by.
        Leave,   ///< Tells a peer it no longer sees a key.
        Section, ///< Opens the messages of one channel.
        Enter,   ///< Gives a peer an entity whole, creating or overwriting it.
        Update,  ///< Gives a peer what changed on an entity and what was removed.
        Move,    ///< Tells a peer an entity now stands under another key.
        Forget,  ///< Tells a peer an entity was destroyed.
        Packed,  ///< Opens a frame packed with LZ4, its unpacked size first, which never appears inside a frame.
    };

    /// \brief Names a set of replicated components, one bit per place in the manifest.
    using Mask      = Bitset<kMaxComponents, UInt64>;

    /// \brief Names where frames go, one call per frame with the key of the peer it is for.
    using Sink      = Delegate<void(UInt64, ConstSpan<Byte>)>;

    /// \brief Represents the tag of a replica this side owns, which is this side's to steer.
    using Possessed = Tag<"Possessed">;

    /// \brief Represents the tag of a replica another side decides for, which `.Not<Remote>()` keeps out of a system.
    using Remote    = Tag<"Remote">;

    /// \brief Represents the tag that keeps an entity to the side serving the world, never sent to a follower.
    using Server    = Tag<"Server">;

    /// \brief Represents the tag that keeps an entity to each side following the world, which runs its own.
    using Client    = Tag<"Client">;

    /// \brief Represents messages written back to back, and the places a section may be cut between them.
    struct Block final
    {
        /// The messages.
        Writer           Bytes;

        /// The offsets a section may be cut at, each the start of a message, in ascending order.
        Sequence<UInt32> Cuts;

        /// \brief Starts a message, marking a cut before it once the piece since the last cut has filled a chunk.
        ///
        /// \param Code The opcode of the message.
        /// \return The writer the rest of the message goes into.
        ZY_INLINE Ref<Writer> Open(Opcode Code)
        {
            const UInt32 Last = Cuts.IsEmpty() ? 0 : Cuts.GetBack();

            if (Bytes.GetSize() - Last >= kMaxChunk)
            {
                Cuts.Append(Bytes.GetSize());
            }
            Bytes.Write<UInt8>(static_cast<UInt8>(Code));
            return Bytes;
        }

        /// \brief Drops every message and cut, keeping the storage.
        ZY_INLINE void Clear()
        {
            Bytes.Clear();
            Cuts.Clear();
        }

        /// \brief Gets the number of bytes written.
        ///
        /// \return The size of the messages.
        ZY_INLINE UInt GetSize() const
        {
            return Bytes.GetSize();
        }
    };
}