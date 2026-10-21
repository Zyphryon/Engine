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

#include "Link.hpp"
#include "Listener.hpp"
#include "Locus.hpp"
#include "Manifest.hpp"
#include "Owner.hpp"
#include "Zyphryon.Network/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    class Publisher;
    class Subscriber;

    /// \brief Represents the replication of the scene to the peers of a session, serving it or following one served.
    class ZY_API Service final : public ZyEngine::Subsystem
    {
    public:

        /// \brief Constructs the replication service, declaring its components in the scene.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Destroys the replication service, closing the session when one is open.
        ~Service() override;

        /// \brief Sends every peer what changed this tick, while this side serves a session.
        ///
        /// \param Delta The elapsed time in seconds since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Closes the session, when one is open.
        void OnTeardown() override;

        /// \brief Makes a component travel, declared alike on both sides before a session opens.
        ///
        /// \param To   The peers that receive the component.
        /// \param Rate The ticks between two reads of its changes, one to read them on every tick.
        template<typename Type>
        ZY_INLINE void Replicate(Audience To, UInt8 Rate = 1)
        {
            mManifest.Register<Type>(To, Rate);
        }

        /// \brief Makes a component travel by its identifier, declared alike on both sides before a session opens.
        ///
        /// \param Identifier The identifier of the component in the registry.
        /// \param To         The peers that receive the component.
        /// \param Rate       The ticks between two reads of its changes, one to read them on every tick.
        /// \param Watch      The function making a world's readers of the component's pull lists.
        ZY_INLINE void Replicate(UInt32 Identifier, Audience To, UInt8 Rate, Manifest::Watcher Watch)
        {
            mManifest.Register(Identifier, To, Rate, Watch);
        }

        /// \brief Stops a component travelling, before a session opens, which one declared since may take again.
        ///
        /// \param Identifier The identifier of the component in the registry.
        ZY_INLINE void Forget(UInt32 Identifier)
        {
            mManifest.Forget(Identifier);
        }

        /// \brief Serves the scene to every peer that reaches an address.
        ///
        /// \param Address   The address to accept peers on.
        /// \param Transport The transport peers reach it over.
        /// \param Listener  The listener told about the peers and their messages.
        /// \return `true` if the session opened, `false` when one is open, the network is missing or refused it.
        Bool Serve(
            ConstRef<ZyNetwork::Address> Address,
            ZyNetwork::Transport         Transport,
            ConstRetainer<Listener>      Listener);

        /// \brief Follows the scene another side serves at an address.
        ///
        /// \param Address   The address the serving side accepts peers on.
        /// \param Transport The transport to reach it over.
        /// \param Listener  The listener told when the serving side is reached and when it goes away.
        /// \return `true` if the session is under way, `false` when one is open, the network is missing or refused it.
        Bool Join(
            ConstRef<ZyNetwork::Address> Address,
            ZyNetwork::Transport         Transport,
            ConstRetainer<Listener>      Listener);

        /// \brief Closes the session, its peers or the serving side going with it.
        void Close();

        /// \brief Replaces the keys a peer sees, while this side serves.
        ///
        /// \param Peer The key the peer is known by.
        /// \param Keys The interest keys it sees from here on, the global and private ones implied.
        void Watch(UInt64 Peer, ConstSpan<UInt64> Keys);

        /// \brief Hands an entity to a peer, which alone receives its owner components and steers it.
        ///
        /// \param Peer  The key of the peer, or zero to hand the entity back to the serving side.
        /// \param Actor The entity, which must be alive.
        void Possess(UInt64 Peer, ZyScene::Entity Actor);

        /// \brief Ends a peer's part in the session, while this side serves.
        ///
        /// \param Peer The key the peer is known by, its listener told once the connection closed.
        void Expel(UInt64 Peer);

        /// \brief Sends a message to the serving side, reliable and in order, while this side follows.
        ///
        /// \param Message The payload, which the serving side's listener receives as it was written.
        void Send(ConstSpan<Byte> Message);

        /// \brief Runs a load both sides read from disk, so its replicas land as the baseline.
        ///
        /// \param Load The callable that loads the world from disk, run once.
        template<typename Callable>
        ZY_INLINE void Restore(AnyRef<Callable> Load)
        {
            Settle(false);
            Load();
            Settle(true);
        }

        /// \brief Finds the entity a replica identifier names on this side.
        ///
        /// \param Identifier The identifier the replica travels under.
        /// \return The entity, or one that names nothing when no living entity carries it.
        ZyScene::Entity Find(UInt64 Identifier);

        /// \brief Finds the entity a link names.
        ///
        /// \param Value The link, which caches what it finds.
        /// \return The entity, or one that names nothing while its target is not held.
        ZyScene::Entity Resolve(Ref<Link> Value);

        /// \brief Checks whether this side serves a session.
        ///
        /// \return `true` while it serves one, `false` otherwise.
        ZY_INLINE Bool IsServing() const
        {
            return static_cast<Bool>(mPublisher);
        }

        /// \brief Checks whether this side follows a session.
        ///
        /// \return `true` while it follows one, `false` otherwise.
        ZY_INLINE Bool IsFollowing() const
        {
            return static_cast<Bool>(mSubscriber);
        }

    private:

        /// \brief Hears the session's connections, on either side.
        class Gate final : public ZyNetwork::Handler
        {
        public:

            /// \brief Constructs a gate for a service.
            ///
            /// \param Owner    The service, outliving the gate's use.
            /// \param Listener The listener the game hears the session through.
            Gate(Ref<Service> Owner, ConstRetainer<Listener> Listener);

            /// \see ZyNetwork::Handler::OnConnect(ZyNetwork::Connection)
            void OnConnect(ZyNetwork::Connection Link) override;

            /// \see ZyNetwork::Handler::OnDisconnect(ZyNetwork::Connection, ZyNetwork::Reason)
            void OnDisconnect(ZyNetwork::Connection Link, ZyNetwork::Reason Reason) override;

            /// \see ZyNetwork::Handler::OnMessage(ZyNetwork::Connection, ZyNetwork::Delivery, ConstSpan<Byte>)
            void OnMessage(ZyNetwork::Connection Link, ZyNetwork::Delivery Delivery, ConstSpan<Byte> Message) override;

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Ref<Service>       mOwner;
            Retainer<Listener> mListener;
        };

        /// \brief Opens the session's endpoint once the table is sealed and a side is made.
        ///
        /// \param Address   The address to accept peers on or to reach.
        /// \param Transport The transport of the endpoint.
        /// \param Listener  The listener the game hears the session through, or null to hear nothing.
        /// \param Serving   `true` to accept peers, `false` to reach the serving side.
        /// \return `true` if the endpoint opened, `false` when one is open, the network is missing or refused it.
        Bool Open(
            ConstRef<ZyNetwork::Address> Address,
            ZyNetwork::Transport         Transport,
            ConstRetainer<Listener>      Listener,
            Bool                         Serving);

        /// \brief Takes in the changes around a load from disk, whose values count as no change.
        ///
        /// \param Loaded `true` once the load ran, `false` before it, when changes waiting their turn are taken.
        void Settle(Bool Loaded);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Manifest              mManifest;
        Unique<Publisher>     mPublisher;
        Unique<Subscriber>    mSubscriber;
        Retainer<Gate>        mGate;
        ZyNetwork::Connection mLink;
    };
}