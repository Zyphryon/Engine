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

#include "Zyphryon.Engine/Locator.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Represents the side that follows a world it does not own, applying what the publisher says of it.
    class Subscriber final : public Engine::Locator<Service>
    {
    public:

        /// \brief Constructs a subscriber instance with the specified service host.
        ///
        /// \param Host The service host to associate with the subscriber.
        explicit Subscriber(Ref<Engine::Subsystem::Host> Host);

        /// \brief Destructor, which takes down what was hung on the world.
        ~Subscriber();

        /// \brief Sets the stamp of the world, which the publisher's greeting has to match.
        ///
        /// \param Stamp The stamp, which is the application's to derive from what both sides load from disk.
        ZY_INLINE void SetStamp(UInt64 Stamp)
        {
            mStamp = Stamp;
        }

        /// \brief Applies a message from the publisher.
        ///
        /// \param Message The payload, exactly as the publisher wrote it.
        /// \return `true` if the message was the protocol's, `false` if it belongs to the application.
        Bool Receive(ConstSpan<Byte> Message);

        /// \brief Checks whether the publisher's greeting arrived and matched this build and world.
        ///
        /// \return `true` if the two sides agree, `false` before the greeting or when they do not.
        ZY_INLINE Bool IsAccepted() const
        {
            return mAccepted;
        }

        /// \brief Checks whether the publisher's greeting arrived and did not match this build or world.
        ///
        /// \return `true` if the two sides disagree, `false` otherwise.
        ZY_INLINE Bool IsRejected() const
        {
            return mRejected;
        }

        /// \brief Gets the key the publisher knows this side by, which is what a spawn names as the owner.
        ///
        /// \return The key, or zero before the greeting arrived.
        ZY_INLINE UInt64 GetIdentity() const
        {
            return mIdentity;
        }

        /// \brief Finds the entity a replica identifier stands for.
        ///
        /// \param Identifier The identifier.
        /// \return The entity, or an invalid entity when none carries the identifier.
        ZY_INLINE Entity Find(UInt64 Identifier) const
        {
            return mReplicas.FindOrDefault(Identifier).Actor;
        }

    private:

        /// \brief Represents an entity a replica identifier stands for, and the last stream applied to it.
        struct Known final
        {
            /// The entity carrying the replica.
            Entity Actor;

            /// The sequence of the last datagram applied, so an older one is dropped.
            UInt32 Sequence = 0;
        };

        /// \brief Applies the publisher's greeting.
        ///
        /// \param Input The stream positioned after the opcode.
        void OnHello(Ref<Reader> Input);

        /// \brief Applies a spawn, or holds it until its scope exists.
        ///
        /// \param Input The stream positioned after the opcode.
        void OnSpawn(Ref<Reader> Input);

        /// \brief Applies the forgetting of a replica.
        ///
        /// \param Input The stream positioned after the opcode.
        void OnForget(Ref<Reader> Input);

        /// \brief Applies a reliable update, or holds it until its replica exists.
        ///
        /// \param Input The stream positioned after the opcode.
        void OnUpdate(Ref<Reader> Input);

        /// \brief Applies a datagram of streamed updates, dropping the ones a newer datagram already superseded.
        ///
        /// \param Input The stream positioned after the opcode.
        void OnStream(Ref<Reader> Input);

        /// \brief Creates the entity a spawn describes beneath its scope.
        ///
        /// \param Scope The entity carrying the scope.
        /// \param Body  The body of the spawn.
        void Materialize(Entity Scope, ConstSpan<Byte> Body);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64                        mStamp;
        UInt64                        mIdentity;
        Bool                          mAccepted;
        Bool                          mRejected;
        Table<UInt64, Known>          mReplicas;
        Table<UInt64, Entity>         mScopes;
        Table<UInt64, Sequence<Blob>> mPendingSpawns;
        Table<UInt64, Sequence<Blob>> mPendingUpdates;
        Array<Entity, 4>              mObservers;
    };
}