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

        /// \brief Checks whether the replica names anything.
        ///
        /// \return `true` if it carries an identifier, `false` if it was read back from a file that held a runtime one.
        ZY_INLINE Bool IsValid() const
        {
            return mIdentifier != 0;
        }

        /// \brief Serializes the identifier of a persistent replica, and nothing of a runtime one.
        ///
        /// \param Archive The archive to read from or write to.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            if constexpr (Serializer::IsWriter)
            {
                UInt64 Identifier = IsPersistent() ? mIdentifier : 0;
                Archive.Serialize(Identifier);
            }
            else
            {
                Archive.Serialize(mIdentifier);

                // An owner is a live connection, so whatever was written has nobody on the other end of it now.
                mOwner = Network::Connection();
            }
        }

    public:

        /// \brief Draws the identity of an entity held from disk, which nothing else was ever given.
        ///
        /// \param Draw The function that draws a random 64-bit number, called until one names something.
        /// \return The replica, persistent and owned by nobody.
        template<typename Callable>
        ZY_INLINE static Replica Persistent(AnyRef<Callable> Draw)
        {
            UInt64 Identifier;

            // The top bit is the protocol's own, and an identifier of nought names nothing.
            do
            {
                Identifier = static_cast<UInt64>(Draw()) & ~kPersistent;
            }
            while (Identifier == 0);

            return Replica(kPersistent | Identifier);
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
        /// \return The key, which is never zero for a scope peers can subscribe to.
        ZY_INLINE UInt64 GetKey() const
        {
            return mKey;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64 mKey;
    };

    /// \brief Tag type that marks an entity the publisher spawned on this side's behalf.
    using Possessed = Tag<"Possessed">;
    
    /// \brief Tag type that marks an entity another side has authority over, so the local simulation leaves it be.
    using Remote    = Tag<"Remote">;
}