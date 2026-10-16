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
#include "Locus.hpp"
#include "Manifest.hpp"
#include "Owner.hpp"
#include "Zyphryon.Job/Service.hpp"
#include "Zyphryon.Network/Service.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::Protocol
{
    /// \brief Represents the side that owns the world and tells every peer what it sees of it.
    class Publisher final
    {
    public:

        /// \brief Constructs a publisher, sending through the host's network service when it has one.
        ///
        /// \param Host The host the scene, the jobs and the network, if any, are found in, outliving the publisher.
        explicit Publisher(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Takes on a peer reached through the network.
        ///
        /// \param Link The connection naming the peer.
        /// \return The key the peer is known by, as \ref ZyNetwork::Connection::GetValue derives it.
        UInt64 Admit(ZyNetwork::Connection Link);

        /// \brief Takes on a peer reached through the sink alone, starting over if the key was already admitted.
        ///
        /// \param Peer The key the peer is known by.
        void Admit(UInt64 Peer);

        /// \brief Drops a peer and every key it saw.
        ///
        /// \param Peer The key of the peer.
        void Expel(UInt64 Peer);

        /// \brief Replaces the keys a peer sees.
        ///
        /// \param Peer The key of the peer.
        /// \param Keys The keys it sees from here on.
        void Subscribe(UInt64 Peer, ConstSpan<UInt64> Keys);

        /// \brief Sends every peer what changed since the last publish, once the world has moved for the tick.
        void Publish();

        /// \brief Runs a load from disk, whose replicas land as the baseline and whose values count as no change.
        ///
        /// \param Callback The function that loads.
        template<typename Callable>
        ZY_INLINE void Restore(AnyRef<Callable> Callback)
        {
            // Changes waiting for their turn are taken now, so the load's reads cannot swallow them.
            Read(false, true);
            Callback();
            Read(true, true);
        }

        /// \brief Finds the entity a replica identifier stands for.
        ///
        /// \param Identifier The identifier.
        /// \return The entity, or one naming nothing when no living entity carries the identifier.
        Entity Find(UInt64 Identifier) const;

        /// \brief Finds the entity a link names.
        ///
        /// \param Value The link, which caches what it finds.
        /// \return The entity, or one naming nothing.
        Entity Resolve(Ref<Link> Value) const;

        /// \brief Sets where frames go, replacing the network.
        ///
        /// \param Output The sink.
        ZY_INLINE void SetSink(AnyRef<Sink> Output)
        {
            mSink = Move(Output);
        }

        /// \brief Sets whether frames go out packed with LZ4 when that shrinks them.
        ///
        /// \param Packing `true` to pack them, which is the default, `false` to send every frame as it was built.
        ZY_INLINE void SetPacking(Bool Packing)
        {
            mPacking = Packing;
        }

    private:

        /// \brief Represents what the publisher remembers about a replica between two publishes.
        struct Tracker final
        {
            /// The identifier in the world of the entity holding the tracker, zero for a free slot.
            UInt64 Actor      = 0;

            /// The key the replica was last published under, zero before its first publish.
            UInt64 Key        = 0;

            /// The identifier the replica travels under.
            UInt64 Identifier = 0;

            /// The identifier of the replica it is attached to, zero for a root.
            UInt64 Parent     = 0;

            /// The components changed since the last publish.
            Mask   Dirty;

            /// The components lost since the last publish.
            Mask   Dropped;

            /// The components of a persistent replica that departed from disk, changed or removed alike.
            Mask   Diverged;

            /// `true` when the replica already waits in line for the next publish.
            Bool   Queued     = false;

            /// `true` when the replica has to arrive whole again, as it does after changing owner.
            Bool   Reenter    = false;
        };

        /// \brief Represents one interest key's traffic for a tick and the roots standing under it.
        struct Channel final
        {
            /// The messages written for this key during the tick, which every subscriber reads.
            Block       Stream;

            /// The whole of what stands under the key, which a new subscriber reads instead of the stream.
            Block       Catchup;

            /// The roots standing under the key, by their identifier in the world.
            Bag<UInt64> Members;

            /// The number of peers that see the key.
            UInt32      Subscribers = 0;

            /// `true` once the catch-up was built this tick.
            Bool        Caught      = false;

            /// \brief Clears the tick's traffic and catch-up, keeping members and subscribers.
            ZY_INLINE void Reset()
            {
                Stream.Clear();
                Catchup.Clear();
                Caught = false;
            }

            /// \brief Checks whether the channel holds nothing and nobody sees it, so it can be dropped.
            ///
            /// \return `true` if it is idle, `false` otherwise.
            ZY_INLINE Bool IsIdle() const
            {
                return Members.IsEmpty() && Subscribers == 0;
            }
        };

        /// \brief Represents one peer, the keys it sees and the frames built for it this tick.
        struct Session final
        {
            /// The bytes a frame header takes: the opcode, the clock and its multiplier.
            static constexpr UInt32 kHeader = sizeof(UInt8) + sizeof(Real64) + sizeof(Real32);

            /// The key the peer is known by.
            UInt64                Key;

            /// The connection the peer is reached through, invalid for a sink alone.
            ZyNetwork::Connection Link;

            /// The keys the peer sees.
            Bag<UInt64>           Keys;

            /// The keys the peer started seeing since its last frame.
            Sequence<UInt64>      Fresh;

            /// The keys the peer stopped seeing since its last frame.
            Sequence<UInt64>      Departed;

            /// The messages only this peer receives, as the owner of what they name.
            Block                 Private;

            /// The frames built this tick, back to back.
            Writer                Frame;

            /// The end of each frame in \ref Frame.
            Sequence<UInt32>      Ends;

            /// The frames packed, which go out in place of \ref Frame while packing.
            Writer                Packed;

            /// The end of each frame in \ref Packed.
            Sequence<UInt32>      Seals;

            /// The room one frame is packed into.
            Sequence<Byte>        Room;

            /// The publisher's clock, which every frame header carries.
            Real64                Time       = 0.0;

            /// The publisher's timescale, which every frame header carries.
            Real32                Multiplier = 1.0f;

            /// `true` once the peer was greeted.
            Bool                  Greeted    = false;

            /// `true` while the frame being built carries a message.
            Bool                  Carries    = false;

            /// \brief Constructs a session for a peer that sees nothing yet.
            ///
            /// \param Peer       The key the peer is known by.
            /// \param Connection The connection the peer is reached through, or an invalid one for a sink alone.
            Session(UInt64 Peer, ZyNetwork::Connection Connection);

            /// \brief Replaces the keys the peer sees.
            ///
            /// \param Wanted  The keys it sees from here on, where the global and private ones are ignored.
            /// \param Added   The list that receives the keys it did not see before.
            /// \param Removed The list that receives the keys it no longer sees.
            void Watch(ConstSpan<UInt64> Wanted, Ref<Sequence<UInt64>> Added, Ref<Sequence<UInt64>> Removed);

            /// \brief Starts the tick's frames.
            ///
            /// \param Clock The publisher's clock, which every frame carries.
            void Open(ConstRef<ZyScene::Clock> Clock);

            /// \brief Writes the greeting.
            ///
            /// \param Hash The hash of the manifest.
            void WriteHello(UInt64 Hash);

            /// \brief Tells the peer it no longer sees a key.
            ///
            /// \param Gone The key.
            void WriteLeave(UInt64 Gone);

            /// \brief Writes a channel's messages as one section per piece, which a new key gets even when empty.
            ///
            /// \param Section  The key of the section.
            /// \param Messages The messages, cut where they mark.
            void WriteSection(UInt64 Section, ConstRef<Block> Messages);

            /// \brief Ends the tick's frames, keeping the last when it carries anything or a heartbeat is due.
            ///
            /// \param Heartbeat `true` to keep the last frame even when it carries nothing.
            void Close(Bool Heartbeat);

            /// \brief Packs every frame of the tick with LZ4, keeping as it was each one packing does not shrink.
            void Pack();

            /// \brief Packs one frame into \ref Packed.
            ///
            /// \param Raw The frame as it was built.
            void Seal(ConstSpan<Byte> Raw);

            /// \brief Drops the tick's frames and key changes once they were handed out.
            void Reset();

            /// \brief Writes one piece of a channel's messages as a section of its own.
            ///
            /// \param Section The key of the section.
            /// \param Piece   The messages of the piece.
            void WritePiece(UInt64 Section, ConstSpan<Byte> Piece);

            /// \brief Writes a frame header.
            void Header();

            /// \brief Starts a message, first ending the frame being built when the message would take it past a chunk.
            ///
            /// \param Code   The opcode of the message.
            /// \param Needed The bytes the message carries after its opcode.
            void Begin(Opcode Code, UInt32 Needed);
        };

        /// \brief Finds the tracker of a replica, by the entity's slot rather than a lookup.
        ///
        /// \param Actor The entity, alive or just destroyed.
        /// \return The tracker, or null when the entity is not tracked.
        ZY_INLINE Ptr<Tracker> Track(Entity Actor)
        {
            const UInt32 Slot = Actor.GetHandle().GetIndex();
            return Slot < mTrackers.GetSize() && mTrackers[Slot].Actor == Actor.GetID() ? AddressOf(mTrackers[Slot]) : nullptr;
        }

        /// \brief Finds the tracker of a replica, by the entity's slot rather than a lookup.
        ///
        /// \param Actor The entity, alive or just destroyed.
        /// \return The tracker, or null when the entity is not tracked.
        ZY_INLINE ConstPtr<Tracker> Track(Entity Actor) const
        {
            const UInt32 Slot = Actor.GetHandle().GetIndex();
            return Slot < mTrackers.GetSize() && mTrackers[Slot].Actor == Actor.GetID() ? AddressOf(mTrackers[Slot]) : nullptr;
        }

        /// \brief Gets the tracker of a replica, making a blank one when it has none.
        ///
        /// \param Actor The entity.
        /// \return The tracker, valid until another is made.
        ZY_INLINE Ref<Tracker> Enlist(Entity Actor)
        {
            const UInt32 Slot = Actor.GetHandle().GetIndex();
            mTrackers.Expand(Slot + 1);

            // A slot an earlier entity left is taken over whole.
            if (mTrackers[Slot].Actor != Actor.GetID())
            {
                mTrackers[Slot]       = Tracker();
                mTrackers[Slot].Actor = Actor.GetID();
            }
            return mTrackers[Slot];
        }

        /// \brief Drops the tracker of a replica.
        ///
        /// \param Actor The entity, alive or just destroyed.
        ZY_INLINE void Untrack(Entity Actor)
        {
            if (const Ptr<Tracker> Tracking = Track(Actor))
            {
                * Tracking = Tracker();
            }
        }

        /// \brief Takes in every replica the world held before the publisher, awake or asleep.
        void Adopt();

        /// \brief Makes a reader for every replicated component declared since the last call.
        void Watch();

        /// \brief Reads every pull list, marking in the trackers what changed.
        ///
        /// \param Restoring `true` right after a load, whose values mark nothing.
        /// \param Whole     `true` to read the changes of rated components whose turn has not come.
        void Read(Bool Restoring, Bool Whole);

        /// \brief Reads one replicated component's pull lists.
        ///
        /// \param Index     The place of the component in the manifest.
        /// \param Restoring `true` right after a load, whose values mark nothing.
        /// \param Whole     `true` to read its changes whether or not its turn has come.
        void Sift(UInt8 Index, Bool Restoring, Bool Whole);

        /// \brief Retires a replica that lost its locus or its life, unless it was given a locus again.
        ///
        /// \param Actor The entity.
        void Depart(Entity Actor);

        /// \brief Takes in an entity whose locus landed or changed, giving a new replica its identifier.
        ///
        /// \param Actor The entity.
        void Place(Entity Actor);

        /// \brief Makes a published replica whose owner changed arrive whole again.
        ///
        /// \param Actor The entity.
        void Claim(Entity Actor);

        /// \brief Marks one replicated component changed or lost on a replica, or on the world for a singleton.
        ///
        /// \param Actor The entity.
        /// \param Index The place of the component in the manifest.
        /// \param Gone  `true` when the entity lost it, `false` when it came or changed.
        void Mark(Entity Actor, UInt8 Index, Bool Gone);

        /// \brief Puts a replica in line for the next publish, once.
        ///
        /// \param Actor    The entity.
        /// \param Tracking The tracker.
        void Enqueue(Entity Actor, Ref<Tracker> Tracking);

        /// \brief Takes a replica off the wire, so its key's subscribers forget it, and drops its tracker.
        ///
        /// \param Actor    The entity.
        /// \param Tracking The tracker, gone once this returns.
        void Retire(Entity Actor, Ref<Tracker> Tracking);

        /// \brief Moves every replica in line into its channels.
        void Drain();

        /// \brief Moves one replica in line into its channels.
        ///
        /// \param Actor The entity.
        void Process(Entity Actor);

        /// \brief Publishes a replica for the first time under a key, with its subtree.
        ///
        /// \param Actor    The entity.
        /// \param Tracking The tracker.
        /// \param Key      The key it stands under.
        /// \param Parent   The identifier of the replica it is attached to, zero for a root.
        void Arrive(Entity Actor, Ref<Tracker> Tracking, UInt64 Key, UInt64 Parent);

        /// \brief Moves a replica from the key it was published under to another.
        ///
        /// \param Actor    The entity.
        /// \param Tracking The tracker.
        /// \param Key      The key it stands under now.
        void Relocate(Entity Actor, Ref<Tracker> Tracking, UInt64 Key);

        /// \brief Writes a replica whole into a stream, with its owner's part to its owner.
        ///
        /// \param Output   The stream.
        /// \param Actor    The entity.
        /// \param Tracking The tracker, left with nothing waiting.
        void Reveal(Ref<Block> Output, Entity Actor, Ref<Tracker> Tracking);

        /// \brief Hands every descendant travelling in an entity's channel to a callback, parents first.
        ///
        /// \param Actor The entity.
        /// \param Visit The callable, taking each descendant.
        template<typename Callable>
        ZY_INLINE void Descend(Entity Actor, AnyRef<Callable> Visit)
        {
            Actor.Children([this, &Visit](Entity Child)
            {
                Branch(Child, Visit);
            });
        }

        /// \brief Hands one child within \ref Descend to a callback, then what stands beneath it.
        ///
        /// \param Child The child.
        /// \param Visit The callable, taking each descendant.
        template<typename Callable>
        ZY_INLINE void Branch(Entity Child, Ref<Callable> Visit)
        {
            // A root beneath stands under its own key, and parts come with the peer's archetype.
            if (IsRoot(Child) || Child.IsPart())
            {
                return;
            }
            Visit(Child);
            Descend(Child, Visit);
        }

        /// \brief Publishes one child of a root that just arrived under a key.
        ///
        /// \param Output The stream the root's arrival was written into.
        /// \param Child  The child.
        /// \param Key    The key the subtree stands under.
        void Embrace(Ref<Block> Output, Entity Child, UInt64 Key);

        /// \brief Writes one child's arrival into a catch-up.
        ///
        /// \param Output The catch-up.
        /// \param Child  The child.
        void Retell(Ref<Block> Output, Entity Child);

        /// \brief Marks unpublished one child of a root that left the wire alive, whose peers lose it with the root.
        ///
        /// \param Child The child.
        void Abandon(Entity Child);

        /// \brief Writes what a replica changed or lost into its channel and its owner's private stream.
        ///
        /// \param Actor    The entity.
        /// \param Tracking The tracker, left with nothing waiting.
        void Broadcast(Entity Actor, Ref<Tracker> Tracking);

        /// \brief Writes the singletons changed or lost since the last publish into the global channel.
        void Proclaim();

        /// \brief Builds and hands out every peer's frames.
        ///
        /// \param Time The world's clock.
        void Deliver(ConstRef<Clock> Time);

        /// \brief Builds the catch-ups a peer needs this tick, and its private catch-up.
        ///
        /// \param Peer The session.
        void Prepare(Ref<Session> Peer);

        /// \brief Builds a peer's frames from channels that no longer change this tick.
        ///
        /// \param Peer      The session.
        /// \param Time      The world's clock.
        /// \param Heartbeat `true` to send a frame even when it carries nothing.
        void Assemble(Ref<Session> Peer, ConstRef<Clock> Time, Bool Heartbeat);

        /// \brief Builds the frames of a range of sessions, on whichever worker took the range.
        ///
        /// \param Begin     The first session.
        /// \param End       The session past the last.
        /// \param Time      The world's clock.
        /// \param Heartbeat `true` to send a frame even when it carries nothing.
        void AssembleRange(UInt32 Begin, UInt32 End, ConstRef<Clock> Time, Bool Heartbeat);
        /// \brief Hands a peer's frames to the sink.
        ///
        /// \param Peer The session.
        void Hand(Ref<Session> Peer);

        /// \brief Gets the whole of what stands under a key, built once per tick.
        ///
        /// \param Key The key, or \ref kGlobal for the world's singletons.
        /// \return The messages of the catch-up.
        ConstRef<Block> Catchup(UInt64 Key);

        /// \brief Writes the private part of every replica a peer owns under a key it just started seeing.
        ///
        /// \param Peer The session.
        /// \param Key  The key.
        void Bestow(Ref<Session> Peer, UInt64 Key);

        /// \brief Writes the private part of a replica when a peer owns it.
        ///
        /// \param Peer  The session.
        /// \param Actor The entity.
        void Endow(Ref<Session> Peer, Entity Actor);

        /// \brief Clears the tick's traffic, drops idle channels and advances the tick.
        void Reset();

        /// \brief Writes the arrival of a replica into a catch-up.
        ///
        /// \param Output The catch-up.
        /// \param Actor  The entity.
        void WriteArrival(Ref<Block> Output, Entity Actor);

        /// \brief Writes a replica whole, with everything its key's subscribers receive.
        ///
        /// \param Output   The stream.
        /// \param Actor    The entity.
        /// \param Tracking The tracker.
        void WriteEnter(Ref<Block> Output, Entity Actor, ConstRef<Tracker> Tracking);

        /// \brief Writes an update of some components.
        ///
        /// \param Output     The stream.
        /// \param Identifier The identifier of the replica, or zero for the world.
        /// \param Actor      The entity, the world entity for singletons.
        /// \param Wanted     The components to write, named gone when not held.
        /// \param Dropped    The components lost.
        void WriteUpdate(Ref<Block> Output, UInt64 Identifier, Entity Actor, Mask Wanted, Mask Dropped);

        /// \brief Gets the channel of a key, creating it when needed.
        ///
        /// \param Key The key.
        /// \return The channel.
        Ref<Channel> Acquire(UInt64 Key);

        /// \brief Finds the channel of a key without creating it.
        ///
        /// \param Key The key.
        /// \return The channel, or null when nothing stands under the key and nobody sees it.
        Ptr<Channel> Seek(UInt64 Key);

        /// \brief Finds the session of a peer.
        ///
        /// \param Peer The key of the peer.
        /// \return The session, or null when the peer was not admitted.
        Ptr<Session> Reach(UInt64 Peer);

        /// \brief Finds the session of a replica's owner while it sees a key.
        ///
        /// \param Actor The entity.
        /// \param Key   The key the replica stands under.
        /// \return The session, or null when nobody owns it, or its owner was not admitted or does not see the key.
        Ptr<Session> Witness(Entity Actor, UInt64 Key);

        /// \brief Sends a frame through the network, which is the sink unless another one is set.
        ///
        /// \param Peer  The key of the peer.
        /// \param Frame The frame.
        void Transmit(UInt64 Peer, ConstSpan<Byte> Frame);

        /// \brief Gets the identifier of the nearest replicated ancestor.
        ///
        /// \param Actor The entity.
        /// \return The identifier, or zero for a root.
        UInt64 GetParentOf(Entity Actor) const;

        /// \brief Checks whether a replica with an identifier was published under a key.
        ///
        /// \param Identifier The identifier.
        /// \param Key        The key.
        /// \return `true` if it went out under that key, `false` otherwise.
        Bool IsPublished(UInt64 Identifier, UInt64 Key) const;

        /// \brief Gets the key a replica stands under, its own or its root's.
        ///
        /// \param Actor The entity.
        /// \return The key, or zero while it stands under none.
        static UInt64 GetKey(Entity Actor);

        /// \brief Checks whether an entity stands under a key of its own.
        ///
        /// \param Actor The entity.
        /// \return `true` for a root, `false` for a child or an entity with no locus.
        static Bool IsRoot(Entity Actor);

        /// \brief Gets the peer that owns a replica.
        ///
        /// \param Actor The entity.
        /// \return The key of the peer, or zero for nobody.
        static UInt64 GetOwner(Entity Actor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Service>                   mScene;
        Ref<ZyJob::Service>            mJobs;
        Retainer<ZyNetwork::Service>   mNetwork;
        Sink                           mSink;
        Table<UInt64, Unique<Session>> mSessions;
        Sequence<Ptr<Session>>         mOrder;
        Table<UInt64, Unique<Channel>> mChannels;
        Sequence<Tracker>              mTrackers;
        Table<UInt64, Entity>          mReplicas;
        Sequence<Entity>               mQueue;
        Sequence<UInt64>               mAdded;
        Sequence<UInt64>               mRemoved;
        Sequence<Entity>               mTouched;
        Sequence<Manifest::Feed>       mFeeds;
        Cursor<Set<Locus>>             mPlaced;
        Cursor<Removed<Locus>>         mUprooted;
        Cursor<Set<Owner>>             mClaimed;
        Cursor<Removed<Owner>>         mDisowned;
        Mask                           mDirty;
        Mask                           mDropped;
        UInt64                         mWorld;
        UInt64                         mNext;
        UInt64                         mTick;
        Bool                           mPacking;
        Bool                           mPublishing;
    };
}