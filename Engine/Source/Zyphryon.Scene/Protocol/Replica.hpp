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
#include "Zyphryon.Scene/Tag.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Represents the identity an entity carries on the wire.
    class Replica final
    {
    public:

        /// \brief Constructs a replica that names nothing.
        ZY_INLINE Replica()
            : mIdentifier { 0 }
        {
        }

        /// \brief Constructs a replica under an identifier, owned by a peer or by nobody.
        ///
        /// \param Identifier The identifier the entity is known by on both sides.
        /// \param Owner      The peer that owns the entity, which receives what is addressed to the owner alone.
        ZY_INLINE explicit Replica(UInt64 Identifier, Network::Connection Owner = Network::Connection())
            : mIdentifier { Identifier },
              mOwner      { Owner }
        {
        }

        /// \brief Gets the identifier the entity is known by on both sides.
        ///
        /// \return The identifier.
        ZY_INLINE UInt64 GetIdentifier() const
        {
            return mIdentifier;
        }

        /// \brief Sets the peer that owns the entity.
        ///
        /// \param Owner The peer, or an invalid connection when nobody owns it.
        ZY_INLINE void SetOwner(Network::Connection Owner)
        {
            mOwner = Owner;
        }

        /// \brief Gets the peer that owns the entity.
        ///
        /// \return The peer, or an invalid connection when nobody owns it.
        ZY_INLINE Network::Connection GetOwner() const
        {
            return mOwner;
        }

        /// \brief Checks whether both sides hold the entity from disk, so it is only ever updated and never spawned.
        ///
        /// \return `true` if the entity is persistent, `false` if it was spawned at runtime.
        ZY_INLINE Bool IsPersistent() const
        {
            return (mIdentifier & kPersistent) != 0;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64              mIdentifier;
        Network::Connection mOwner;
    };

    /// \brief Represents a group of entities peers subscribe to as one, which is every replica beneath it.
    class Scope final
    {
    public:

        /// \brief Constructs a scope under no key.
        ZY_INLINE Scope()
            : mKey { 0 }
        {
        }

        /// \brief Constructs a scope under a key.
        ///
        /// \param Key The key the scope is subscribed to by, which must not be zero.
        ZY_INLINE explicit Scope(UInt64 Key)
            : mKey { Key }
        {
        }

        /// \brief Gets the key the scope is subscribed to by.
        ///
        /// \return The key.
        ZY_INLINE UInt64 GetKey() const
        {
            return mKey;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mKey;
    };

    /// \brief Tag type that marks an entity another side has authority over, so the local simulation leaves it be.
    using Remote = Tag<"Remote">;
}