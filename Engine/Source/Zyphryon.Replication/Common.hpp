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

namespace ZyReplication
{
    /// \brief The bit set on the identifier of a replica both sides load from disk.
    inline constexpr UInt64 kPersistent = 1ull << 63;

    /// \brief Specifies who receives a replicated component.
    enum class Audience : UInt8
    {
        Everyone, ///< Every peer that sees the entity.
        Owner,    ///< Only the peer that owns the entity.
        Others,   ///< Every peer that sees the entity, while its owner ignores it.
    };

    /// \brief Represents the tag of a replica this side owns, which is this side's to steer.
    using Possessed = ZyScene::Tag<"Possessed">;

    /// \brief Represents the tag of a replica another side decides for, which `.Not<Remote>()` keeps out of a system.
    using Remote    = ZyScene::Tag<"Remote">;

    /// \brief Represents the tag that keeps an entity to the side serving the world, never sent to a follower.
    using Server    = ZyScene::Tag<"Server">;

    /// \brief Represents the tag that keeps an entity to each side following the world, which runs its own.
    using Client    = ZyScene::Tag<"Client">;
}