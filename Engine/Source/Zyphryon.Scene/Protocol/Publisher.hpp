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

#include "Ledger.hpp"
#include "Wire.hpp"
#include "Zyphryon.Engine/Locator.hpp"
#include "Zyphryon.Network/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Represents the side that owns the world and tells every peer what it sees of it.
    class Publisher final : public Engine::Locator<Service, Network::Service>
    {
    public:

        /// \brief Size the reliable stream of a peer is sent at without waiting for the tick to end.
        static constexpr UInt32 kMaxChunk = 16 * 1024;

    public:

        /// \brief Constructs a publisher instance with the specified service host.
        ///
        /// \param Host The service host to associate with the publisher.
        explicit Publisher(Ref<Engine::Subsystem::Host> Host);

        /// \brief Destructor, which takes down what was hung on the world.
        ~Publisher();

        /// \brief Sets the stamp of the world, which a peer must present the same of to be kept.
        ///
        /// \param Stamp The stamp, which is the application's to derive from what both sides load from disk.
        ZY_INLINE void SetStamp(UInt64 Stamp)
        {
            mStamp = Stamp;
        }

        /// \brief Takes on a peer, greeting it with the manifest, the stamp and every replicated singleton.
        ///
        /// \param Link The peer, which is known from here on by \ref Protocol::KeyOf.
        void Admit(Network::Connection Link);

        /// \brief Drops a peer, forgetting what it was subscribed to and what it knew.
        ///
        /// \param Link The peer.
        void Expel(Network::Connection Link);

        /// \brief Sets the scopes a peer sees, replacing whatever it saw before.
        ///
        /// \param Key    The key of the peer, as \ref Protocol::KeyOf derives it.
        /// \param Scopes The keys of the scopes the peer sees from here on.
        void Subscribe(UInt64 Key, ConstSpan<UInt64> Scopes);

        /// \brief Sends every peer what changed since the last time, once the world has moved for the tick.
        void Publish();

    private:

        /// \brief Represents one peer and everything owed to it.
        struct Member final
        {
            /// The connection the peer is reached by.
            Network::Connection Link;

            /// The keys of the scopes the peer sees.
            Bag<UInt64>         Scopes;

            /// The identifiers of the spawned replicas the peer holds, which have to be forgotten by name.
            Bag<UInt64>         Known;

            /// The sequence the next datagram goes out under.
            UInt32              Sequence = 0;

            /// The reliable messages gathered for the peer this tick.
            Writer              Reliable;

            /// The datagram being filled for the peer this tick.
            Writer              Datagram;
        };

        /// \brief Represents one scope and who sees it.
        struct Group final
        {
            /// The entity carrying the scope, or invalid while nothing carries it.
            Entity                 Actor;

            /// The peers subscribed to the scope, few enough that walking them beats hashing them.
            Sequence<Ptr<Member>>  Subscribers;
        };

        /// \brief Represents a spawned replica that went away before the tick ended.
        struct Departure final
        {
            /// The identifier the replica was known by.
            UInt64 Identifier;

            /// The key of the scope the replica was last seen in.
            UInt64 Scope;
        };

    private:

        /// \brief Finds the key of the nearest scope up an entity's parent chain.
        ///
        /// \param Actor The entity to look up from.
        /// \return The key, or zero when no scope stands over the entity.
        static UInt64 Locate(Entity Actor);

        /// \brief Gets the components of a replica a peer is allowed to receive.
        ///
        /// \param Peer   The peer.
        /// \param Record The replica.
        /// \return The mask of components addressed to that peer.
        static Mask Audience(ConstRef<Member> Peer, ConstRef<Replica> Record);

        /// \brief Finds a peer by its key.
        ///
        /// \param Key The key of the peer.
        /// \return The peer, or null when no peer was admitted under the key.
        Ptr<Member> Find(UInt64 Key);

        /// \brief Takes a peer off a group's subscribers.
        ///
        /// \param Group The group.
        /// \param Peer  The peer.
        static void Leave(Ref<Group> Group, Ptr<Member> Peer);

        /// \brief Gives a peer a replica it now sees: a spawn, or for one held from disk, only what departed from it.
        ///
        /// \param Peer     The peer.
        /// \param Actor    The entity carrying the replica.
        /// \param Record   The replica.
        /// \param Tracking What the publisher remembers about the replica.
        /// \param Key      The key of the scope the replica stands in.
        void Reveal(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, ConstRef<Tracker> Tracking, UInt64 Key);

        /// \brief Takes a replica away from a peer that no longer sees it, if it was ever given.
        ///
        /// \note A replica held from disk was never given, so it is never taken away.
        ///
        /// \param Peer       The peer.
        /// \param Identifier The identifier of the replica.
        void Conceal(Ref<Member> Peer, UInt64 Identifier);

        /// \brief Tells a peer about every replica beneath a scope it now sees.
        ///
        /// \param Actor The entity carrying the scope, or one beneath it.
        /// \param Peer  The peer.
        void Announce(Entity Actor, Ref<Member> Peer);

        /// \brief Makes a peer forget every spawned replica beneath a scope it no longer sees.
        ///
        /// \param Actor The entity carrying the scope, or one beneath it.
        /// \param Peer  The peer.
        void Retract(Entity Actor, Ref<Member> Peer);

        /// \brief Moves a replica between scopes, telling the peers that stop or start seeing it.
        ///
        /// \param Actor    The entity carrying the replica.
        /// \param Record   The replica.
        /// \param Tracking What the publisher remembers about the replica.
        /// \param Key      The key of the scope the replica now stands in, or zero.
        void Move(Entity Actor, ConstRef<Replica> Record, Ref<Tracker> Tracking, UInt64 Key);

        /// \brief Sends what a replica touched or removed to every peer that sees it.
        ///
        /// \param Actor    The entity carrying the replica.
        /// \param Record   The replica.
        /// \param Tracking What the publisher remembers about the replica.
        void Broadcast(Entity Actor, ConstRef<Replica> Record, Ref<Tracker> Tracking);

        /// \brief Writes the spawn of a replica into a peer's reliable stream.
        ///
        /// \param Peer   The peer.
        /// \param Actor  The entity carrying the replica.
        /// \param Record The replica.
        /// \param Key    The key of the scope the replica stands in.
        void WriteSpawn(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, UInt64 Key);

        /// \brief Writes what a replica departed from disk into a peer's reliable stream, changed or gone alike.
        ///
        /// \param Peer   The peer.
        /// \param Actor  The entity carrying the replica.
        /// \param Record The replica.
        /// \param Wanted The components that departed, written when held and named as gone when not.
        void WriteUpdate(Ref<Member> Peer, Entity Actor, ConstRef<Replica> Record, Mask Wanted);

        /// \brief Writes the forgetting of a replica into a peer's reliable stream.
        ///
        /// \param Peer       The peer.
        /// \param Identifier The identifier of the replica.
        void WriteForget(Ref<Member> Peer, UInt64 Identifier);

        /// \brief Adds an entry to a peer's datagram, sending the one being filled when it has no room left.
        ///
        /// \param Peer  The peer.
        /// \param Entry The entry, which must fit a datagram on its own.
        void WriteStream(Ref<Member> Peer, ConstSpan<Byte> Entry);

        /// \brief Sends a peer's reliable stream if it grew past the chunk size.
        ///
        /// \param Peer The peer.
        void Drain(Ref<Member> Peer);

        /// \brief Sends whatever a peer has gathered, reliable stream and datagram both.
        ///
        /// \param Peer The peer.
        void Flush(Ref<Member> Peer);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt64                        mStamp;
        Table<UInt64, Unique<Member>> mMembers;
        Table<UInt64, Group>          mGroups;
        Sequence<Departure>           mDepartures;
        Array<Entity, 4>              mObservers;
        Sequence<Entity>              mWatchers;
        Writer                        mScratch;
        Writer                        mEntry;
        Wire::Slices                  mSlices;
    };
}