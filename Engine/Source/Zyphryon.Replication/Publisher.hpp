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

#include "Locus.hpp"
#include "Manifest.hpp"
#include "Owner.hpp"
#include "Replica.hpp"
#include "Wire.hpp"
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Network/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Represents the side that owns the world and tells every peer what it sees of it.
    class Publisher final
    {
    public:

        /// \brief Constructs a publisher over a world, taking in every replica it already holds.
        ///
        /// \param Scene   The world it publishes, outliving the publisher.
        /// \param Jobs    The job service frames are assembled on, outliving the publisher.
        /// \param Network The network service frames are sent through, outliving the publisher.
        /// \param Table   The sealed table of what travels, outliving the publisher.
        Publisher(
            Ref<ZyScene::Service>   Scene,
            Ref<ZyJob::Service>     Jobs,
            Ref<ZyNetwork::Service> Network,
            ConstRef<Manifest>      Table);

        /// \brief Takes on a peer reached through the network.
        ///
        /// \param Link The connection the peer is reached through, whose value becomes its key.
        /// \return The key the peer is known by, as \ref ZyNetwork::Connection::GetValue derives it.
        UInt64 Admit(ZyNetwork::Connection Link);

        /// \brief Drops a peer and every key it saw.
        ///
        /// \param Peer The key the peer is known by.
        /// \return The connection the peer was reached through, or an invalid one when it was not admitted.
        ZyNetwork::Connection Expel(UInt64 Peer);

        /// \brief Replaces the keys a peer sees.
        ///
        /// \param Peer The key the peer is known by.
        /// \param Keys The interest keys it sees from here on, the global and private ones implied.
        void Subscribe(UInt64 Peer, ConstSpan<UInt64> Keys);

        /// \brief Sends every peer what changed since the last publish, once the world has moved for the tick.
        void Publish();

        /// \brief Hands a peer the game's message with its next frame, after what changed in the world before it.
        ///
        /// \param Peer    The key the peer is known by, nobody receiving it when the peer was not admitted.
        /// \param Message The payload, up to \ref kMaxChunk bytes, a longer one dropped and logged.
        void Post(UInt64 Peer, ConstSpan<Byte> Message);

        /// \brief Hands the game's message to the peers that see an entity, with their next frame after what changed.
        ///
        /// \param Actor    The entity it is about, the world for every peer, and none hears of one never published.
        /// \param Audience The peers that receive the message, as they would a component of the entity.
        /// \param Message  The payload, up to \ref kMaxChunk bytes, a longer one dropped and logged.
        void Post(ZyScene::Entity Actor, Audience Audience, ConstSpan<Byte> Message);

        /// \brief Reads every pull list, marking in the trackers what changed.
        ///
        /// \param Restoring `true` right after a load, whose values mark nothing, `false` otherwise.
        /// \param Whole     `true` to read the changes of rated components whose turn has not come, `false` otherwise.
        void Read(Bool Restoring, Bool Whole);

        /// \brief Finds the entity a replica identifier stands for.
        ///
        /// \param Identifier The identifier the replica travels under.
        /// \return The entity, or one naming nothing when no living entity carries the identifier.
        ZyScene::Entity Find(UInt64 Identifier) const;

    private:

        /// \brief Represents messages written back to back, and the places a section may be cut between them.
        struct Block final
        {
            /// The messages, each opening with its opcode.
            Writer           Bytes;

            /// The offsets a section may be cut at, each the start of a message, in ascending order.
            Sequence<UInt32> Cuts;
        };

        /// \brief Represents what the publisher remembers about a replica between two publishes.
        struct Tracker final
        {
            /// The identifier in the world of the entity holding the tracker, zero for a free slot.
            UInt64 Actor      = 0;

            /// The key the replica was last published under, zero before its first publish.
            UInt64 Key        = 0;

            /// The identifier the replica travels under, zero for the world, whose singletons it marks.
            UInt64 Identifier = 0;

            /// The identifier of the replica it is attached to, zero for a root.
            UInt64 Parent     = 0;

            /// The components changed since the last publish.
            Mask   Dirty;

            /// The components lost since the last publish.
            Mask   Dropped;

            /// The components of a persistent replica that departed from disk, changed or removed alike.
            Mask   Diverged;

            /// `true` when the replica already waits in line for the next publish, `false` otherwise.
            Bool   Queued     = false;

            /// `true` when the replica has to arrive whole again, as it does after changing owner, `false` otherwise.
            Bool   Reenter    = false;
        };

        /// \brief Represents one interest key's traffic for a tick and the roots standing under it.
        struct Channel final
        {
            /// The messages written for this key during the tick, which every subscriber reads.
            Block       Stream;

            /// The whole of what stands under the key, built once per tick for the peers that start seeing it.
            Block       Catchup;

            /// The roots standing under the key, by their identifier in the world.
            Bag<UInt64> Members;

            /// The number of peers that see the key.
            UInt32      Subscribers = 0;
        };

        /// \brief Represents one peer, the keys it sees and the frames built for it this tick.
        struct Session final
        {
            /// The connection the peer is reached through, whose value is the key the peer is known by.
            ZyNetwork::Connection Link;

            /// The interest keys the peer sees.
            Bag<UInt64>           Keys;

            /// The keys the peer started seeing since its last frame.
            Sequence<UInt64>      Fresh;

            /// The keys the peer stopped seeing since its last frame.
            Sequence<UInt64>      Departed;

            /// The messages only this peer receives, as the owner of what they name.
            Block                 Private;

            /// The frame being built, behind the clock it opens with.
            Writer                Frame;

            /// The sealed frames of the tick back to back, each packed or as it was built.
            Writer                Out;

            /// The end of each sealed frame in \ref Out.
            Sequence<UInt32>      Ends;

            /// `true` once the peer was greeted, `false` before its first frame.
            Bool                  Greeted = false;
        };

        /// The bytes a frame header takes, its opcode, the clock and its multiplier.
        static constexpr UInt32 kHeader    = sizeof(UInt8) + sizeof(Real64) + sizeof(Real32);

        /// The bytes a packed frame opens with, its opcode and the size it unpacks to.
        static constexpr UInt32 kPacked    = sizeof(UInt8) + sizeof(UInt32);

        /// The bytes a stream piece and a frame are cut at, always between two messages.
        static constexpr UInt32 kMaxChunk  = 32 * 1024;

        /// The seconds of real time between two frames to a peer that has nothing new.
        static constexpr Real64 kHeartbeat = 1.0;

        /// \brief Finds the tracker of a replica, by the entity's slot rather than a lookup.
        ///
        /// \param Actor The entity, alive or just destroyed.
        /// \return The tracker, or null when the entity is not tracked.
        ZY_INLINE Ptr<Tracker> Track(ZyScene::Entity Actor)
        {
            const UInt32 Slot = static_cast<UInt32>(Actor.GetID());
            return Slot < mTrackers.GetSize() && mTrackers[Slot].Actor == Actor.GetID()
                ? AddressOf(mTrackers[Slot])
                : nullptr;
        }

        /// \brief Gets the tracker of an entity, making a blank one when it has none.
        ///
        /// \param Actor The entity, which must be alive.
        /// \return The tracker, valid until another is made.
        ZY_INLINE Ref<Tracker> Enlist(ZyScene::Entity Actor)
        {
            const UInt32 Slot = static_cast<UInt32>(Actor.GetID());
            mTrackers.Expand(Slot + 1);

            // A slot an earlier entity left is taken over whole.
            if (mTrackers[Slot].Actor != Actor.GetID())
            {
                mTrackers[Slot]       = Tracker();
                mTrackers[Slot].Actor = Actor.GetID();
            }
            return mTrackers[Slot];
        }

        /// \brief Takes in an entity whose locus landed or changed, giving a new replica its identifier.
        ///
        /// \param Actor The entity holding the locus, skipped when dead or kept to one side.
        void Place(ZyScene::Entity Actor);

        /// \brief Puts a replica in line for the next publish, once.
        ///
        /// \param Actor    The entity the tracker belongs to.
        /// \param Tracking The tracker, marked as waiting in line.
        void Enqueue(ZyScene::Entity Actor, Ref<Tracker> Tracking);

        /// \brief Takes a replica off the wire, so its key's subscribers forget it, and drops its tracker.
        ///
        /// \param Actor    The entity that lost its locus or its life.
        /// \param Tracking The tracker of the entity, blank once this returns.
        void Retire(ZyScene::Entity Actor, Ref<Tracker> Tracking);

        /// \brief Moves one replica in line into its channels, as an arrival, a move or what it changed.
        ///
        /// \param Actor The entity taken off the line, skipped when it died or is no longer tracked.
        void Process(ZyScene::Entity Actor);

        /// \brief Publishes a replica for the first time under a key, with its subtree.
        ///
        /// \param Actor    The entity arriving.
        /// \param Tracking The tracker of the entity, published under the key once this returns.
        /// \param Key      The interest key it stands under.
        /// \param Parent   The identifier of the replica it is attached to, zero for a root.
        void Arrive(ZyScene::Entity Actor, Ref<Tracker> Tracking, UInt64 Key, UInt64 Parent);

        /// \brief Takes a replica out of the key it stood under, telling that key's peers where it went.
        ///
        /// \param Actor    The entity leaving, dropped from the old key's members when it is a root.
        /// \param Tracking The tracker of the entity, still holding the key it leaves.
        /// \param Key      The interest key it moves under, which peers that also watch it keep the replica in.
        void Depart(ZyScene::Entity Actor, ConstRef<Tracker> Tracking, UInt64 Key);

        /// \brief Writes a replica whole into a stream, with its owner's part to its owner.
        ///
        /// \param Output   The stream of the key it stands under.
        /// \param Actor    The entity written.
        /// \param Tracking The tracker of the entity, left with nothing waiting.
        void Reveal(Ref<Block> Output, ZyScene::Entity Actor, Ref<Tracker> Tracking);

        /// \brief Writes what a replica changed or lost into its channel and its owner's private stream.
        ///
        /// \param Actor    The entity written.
        /// \param Tracking The tracker of the entity, left with nothing waiting.
        void Broadcast(ZyScene::Entity Actor, Ref<Tracker> Tracking);

        /// \brief Hands every descendant travelling in an entity's channel to a callback, parents first.
        ///
        /// \param Actor The entity whose children are walked, itself not handed.
        /// \param Visit The callable, taking each descendant, never a root beneath nor a part.
        template<typename Callable>
        ZY_INLINE void Descend(ZyScene::Entity Actor, AnyRef<Callable> Visit)
        {
            Actor.Children([this, &Visit](ZyScene::Entity Child)
            {
                // A root beneath stands under its own key, and parts come with the peer's archetype.
                if (!IsRoot(Child) && !Child.IsPart())
                {
                    Visit(Child);
                    Descend(Child, Visit);
                }
            });
        }

        /// \brief Builds and hands out every peer's frames.
        ///
        /// \param Time The world's clock, which every frame opens with.
        void Deliver(ConstRef<ZyScene::Clock> Time);

        /// \brief Builds a peer's frames from channels that no longer change this tick, on any worker.
        ///
        /// \param Peer      The session whose frames are built.
        /// \param Time      The world's clock, which every frame opens with.
        /// \param Heartbeat `true` to send a frame even when it carries nothing, `false` otherwise.
        void Assemble(Ref<Session> Peer, ConstRef<ZyScene::Clock> Time, Bool Heartbeat);

        /// \brief Gets the whole of what stands under a key, built once per tick.
        ///
        /// \param Key The interest key, or \ref kGlobal for the world's singletons.
        /// \return The messages of the catch-up.
        ConstRef<Block> Catchup(UInt64 Key);

        /// \brief Writes the owner part of every replica a peer owns under a key into its private stream.
        ///
        /// \param Peer The session that started seeing the key.
        /// \param Key  The interest key the replicas stand under.
        void WriteOwned(Ref<Session> Peer, UInt64 Key);

        /// \brief Writes a replica whole, with everything its key's subscribers receive.
        ///
        /// \param Output   The stream the arrival goes into.
        /// \param Actor    The entity written.
        /// \param Tracking The tracker of the entity, naming its identifier and parent.
        void WriteEnter(Ref<Block> Output, ZyScene::Entity Actor, ConstRef<Tracker> Tracking);

        /// \brief Writes an update of some components.
        ///
        /// \param Output     The stream the update goes into.
        /// \param Identifier The identifier the replica travels under, or zero for the world.
        /// \param Actor      The entity written, the world entity for singletons.
        /// \param Wanted     The components to write, named gone when the entity does not hold them.
        /// \param Dropped    The components the entity lost, named gone unless it holds them again.
        /// \param Delta      The components the peers hold, written as what they changed since their last delta.
        void WriteUpdate(
            Ref<Block>      Output,
            UInt64          Identifier,
            ZyScene::Entity Actor,
            Mask            Wanted,
            Mask            Dropped,
            Mask            Delta);

        /// \brief Gets the channel of a key, creating it when needed.
        ///
        /// \param Key The interest key.
        /// \return The channel, whose address holds while it stands.
        Ref<Channel> GetChannel(UInt64 Key);

        /// \brief Finds the channel of a key without creating it, safe to call on any worker.
        ///
        /// \param Key The interest key.
        /// \return The channel, or null when nothing stands under the key and nobody sees it.
        Ptr<Channel> FindChannel(UInt64 Key);

        /// \brief Finds the session of a peer.
        ///
        /// \param Peer The key the peer is known by, or zero for nobody.
        /// \return The session, or null when the peer was not admitted.
        Ptr<Session> FindSession(UInt64 Peer);

        /// \brief Finds the session of a replica's owner while it sees a key.
        ///
        /// \param Actor The entity whose owner is looked for.
        /// \param Key   The interest key the replica stands under.
        /// \return The session, or null when nobody owns it, or its owner was not admitted or does not see the key.
        Ptr<Session> FindOwner(ZyScene::Entity Actor, UInt64 Key);

        /// \brief Gets the identifier of the nearest replicated ancestor.
        ///
        /// \param Actor The entity whose ancestors are walked.
        /// \return The identifier, or zero for a root.
        UInt64 GetParentOf(ZyScene::Entity Actor);

        /// \brief Starts a message in a stream, cutting before it once the piece since the last cut fills a chunk.
        ///
        /// \param Output The stream the message goes into.
        /// \param Code   The opcode of the message.
        /// \return The writer the rest of the message goes into.
        static Ref<Writer> Open(Ref<Block> Output, Opcode Code);

        /// \brief Starts a message in a frame, first sealing the frame when the message would take it past a chunk.
        ///
        /// \param Peer   The session whose frame the message goes into.
        /// \param Code   The opcode of the message.
        /// \param Needed The bytes the message carries after its opcode.
        static void Begin(Ref<Session> Peer, Opcode Code, UInt32 Needed);

        /// \brief Seals the frame being built into the tick's frames, packed with LZ4 when that shrinks it.
        ///
        /// \param Peer The session whose frame is sealed, left with a frame holding only its clock.
        static void Seal(Ref<Session> Peer);

        /// \brief Writes a channel's messages as one section per piece, which a new key gets even when empty.
        ///
        /// \param Peer     The session whose frames the section goes into.
        /// \param Key      The interest key of the section.
        /// \param Messages The messages, cut where they mark.
        static void WriteSection(Ref<Session> Peer, UInt64 Key, ConstRef<Block> Messages);

        /// \brief Writes the game's message, which one peer may be told to pass over.
        ///
        /// \param Output  The stream the message goes into.
        /// \param Skipped The key of the peer that passes over it, or zero for none.
        /// \param Message The payload, up to \ref kMaxChunk bytes, a longer one dropped and logged.
        static void WriteMessage(Ref<Block> Output, UInt64 Skipped, ConstSpan<Byte> Message);

        /// \brief Gets the key a replica stands under, its own or its root's.
        ///
        /// \param Actor The entity whose ancestors are walked.
        /// \return The key, or zero while it stands under none.
        static UInt64 GetKey(ZyScene::Entity Actor);

        /// \brief Checks whether an entity stands under a key of its own.
        ///
        /// \param Actor The entity checked.
        /// \return `true` for a root, `false` for a child or an entity with no locus.
        static Bool IsRoot(ZyScene::Entity Actor);

        /// \brief Gets the peer that owns a replica.
        ///
        /// \param Actor The entity checked.
        /// \return The key of the peer, or zero for nobody.
        static UInt64 GetOwner(ZyScene::Entity Actor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<ZyScene::Service>          mScene;
        Ref<ZyJob::Service>            mJobs;
        Ref<ZyNetwork::Service>        mNetwork;
        ConstRef<Manifest>             mManifest;
        Table<UInt64, Unique<Session>> mSessions;
        Table<UInt64, Unique<Channel>> mChannels;
        Sequence<Tracker>              mTrackers;
        Table<UInt64, ZyScene::Entity> mReplicas;
        Sequence<ZyScene::Entity>      mQueue;
        Sequence<ZyScene::Entity>      mTouched;
        Sequence<Manifest::Feed>       mFeeds;
        Manifest::Feed                 mLoci;
        Manifest::Feed                 mOwners;
        UInt64                         mNext;
        UInt64                         mTick;
    };
}