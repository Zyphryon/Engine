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
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene::Protocol
{
    /// \brief Represents the side that follows a world it does not own, applying what the publisher says of it.
    class Subscriber final
    {
    public:

        /// \brief Constructs a subscriber, declaring the protocol's components in the host's world.
        ///
        /// \param Host The host the scene is found in, outliving the subscriber.
        explicit Subscriber(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Applies a frame from the publisher.
        ///
        /// \param Message The payload, exactly as the publisher's sink handed it.
        /// \return `true` if the message was the protocol's, `false` if it belongs to the game.
        Bool Receive(ConstSpan<Byte> Message);

        /// \brief Runs a load from disk, taking in the persistent replicas it made at once.
        ///
        /// \param Callback The function that loads.
        template<typename Callable>
        ZY_INLINE void Restore(AnyRef<Callable> Callback)
        {
            Callback();
            Refresh();
        }

        /// \brief Checks whether the greeting arrived and matched this build.
        ///
        /// \return `true` if the two sides agree, `false` before the greeting or when they do not.
        ZY_INLINE Bool IsAccepted() const
        {
            return mAccepted;
        }

        /// \brief Checks whether the publisher's traffic was refused, by a mismatched greeting or a malformed frame.
        ///
        /// \return `true` if nothing more is applied until a new session greets this side, `false` otherwise.
        ZY_INLINE Bool IsRejected() const
        {
            return mRejected;
        }

        /// \brief Gets the key the publisher knows this side by.
        ///
        /// \return The key, or zero before the greeting.
        ZY_INLINE UInt64 GetIdentity() const
        {
            return mIdentity;
        }

        /// \brief Finds the entity a replica identifier stands for, letting go of one no longer standing.
        ///
        /// \param Identifier The identifier.
        /// \return The entity, or one naming nothing when no living entity carries the identifier.
        Entity Find(UInt64 Identifier);

        /// \brief Finds the entity a link names.
        ///
        /// \param Value The link, which caches what it finds.
        /// \return The entity, or one naming nothing while its target is not held.
        Entity Resolve(Ref<Link> Value);

    private:

        /// \brief Represents what an arrival says of a replica.
        struct Arrival final
        {
            /// The identifier the replica travels under.
            UInt64          Identifier = 0;

            /// The entity it is attached to, or one naming nothing for a root.
            Entity          Parent;

            /// The slot of the archetype it is made from, or zero.
            UInt64          Archetype  = 0;

            /// The key of the peer that owns it, or zero for nobody.
            UInt64          Peer       = 0;

            /// The bytes of its set.
            ConstSpan<Byte> Set;

            /// `true` when this side did not hold it before.
            Bool            Fresh      = false;
        };

        /// \brief Represents a message held for a replica this side does not hold yet, its cell not loaded.
        struct Pending final
        {
            /// The key of the section the message came in, which a leave of that key drops it with.
            UInt64 Key;

            /// The message: an update of the replica, or the arrival of a child beneath it.
            Opcode Code;

            /// The bytes of the message after its opcode, or of the update's set.
            Blob   Data;
        };

        /// \brief Takes in the persistent replicas loaded since the last call.
        void Refresh();

        /// \brief Takes in one persistent replica, applying the messages that waited for it.
        ///
        /// \param Actor  The entity.
        /// \param Record The identity it travels under.
        void Take(Entity Actor, ConstRef<Replica> Record);

        /// \brief Applies the messages that waited for a replica this side now holds.
        ///
        /// \param Identifier The identifier the replica travels under.
        /// \param Actor      The entity.
        void Release(UInt64 Identifier, Entity Actor);

        /// \brief Applies one message that waited for a replica.
        ///
        /// \param Actor The entity it waited for.
        /// \param Item  The message.
        void Replay(Entity Actor, ConstRef<Pending> Item);

        /// \brief Drops the waiting sets of one replica that came under a key.
        ///
        /// \param Key  The key left.
        /// \param Sets The waiting sets, emptied of that key's.
        /// \return `true` if none is left, `false` otherwise.
        static Bool Discard(UInt64 Key, Ref<Sequence<Pending>> Sets);

        /// \brief Unpacks a frame packed with LZ4 and applies it, refusing the publisher when it does not unpack whole.
        ///
        /// \param Input The message, at its opcode.
        void Unpack(Ref<Reader> Input);

        /// \brief Applies one message.
        ///
        /// \param Input The frame, at the message's opcode.
        void Dispatch(Ref<Reader> Input);

        /// \brief Applies the greeting, which starts the session over.
        ///
        /// \param Input The frame, after the opcode.
        void OnHello(Ref<Reader> Input);

        /// \brief Drops every runtime replica under a key this side no longer sees.
        ///
        /// \param Input The frame, after the opcode.
        void OnLeave(Ref<Reader> Input);

        /// \brief Opens the section of a key, which also says this side sees it.
        ///
        /// \param Input The frame, after the opcode.
        void OnSection(Ref<Reader> Input);

        /// \brief Creates a replica, or overwrites the one already held.
        ///
        /// \param Input The frame, after the opcode.
        void OnEnter(Ref<Reader> Input);

        /// \brief Applies what changed on a replica, or on the world for id zero.
        ///
        /// \param Input The frame, after the opcode.
        void OnUpdate(Ref<Reader> Input);

        /// \brief Re-keys a replica that moved to a key this side sees, or destroys it.
        ///
        /// \param Input The frame, after the opcode.
        void OnMove(Ref<Reader> Input);

        /// \brief Destroys a replica.
        ///
        /// \param Input The frame, after the opcode.
        void OnForget(Ref<Reader> Input);

        /// \brief Lands everything an arrival names on its entity, inside one deferral.
        ///
        /// \param Actor   The entity.
        /// \param Message The arrival.
        void Settle(Entity Actor, ConstRef<Arrival> Message);

        /// \brief Sets who owns a replica, tagging it this side's or another's.
        ///
        /// \param Actor The entity.
        /// \param Peer  The key of the owner, or zero for nobody.
        void Claim(Entity Actor, UInt64 Peer);

        /// \brief Destroys every runtime root under a key, or every runtime root for \ref kGlobal.
        ///
        /// \param Key The key.
        void Clear(UInt64 Key);

        /// \brief Gathers a runtime replica for \ref Clear when it stands under the key and would not go with a parent.
        ///
        /// \param Actor  The entity.
        /// \param Place  The key it stands under, zero for a child.
        /// \param Record The identity it travels under.
        /// \param Key    The key cleared, or \ref kGlobal for every key.
        void Collect(Entity Actor, ConstRef<Locus> Place, ConstRef<Replica> Record, UInt64 Key);

        /// \brief Destroys a replica, letting go of its identifier.
        ///
        /// \param Actor The entity.
        void Destroy(Entity Actor);

        /// \brief Reads past a set, refusing the publisher when it is not whole.
        ///
        /// \param Input The frame, at the set.
        /// \return The bytes of the set, or none once refused.
        ConstSpan<Byte> Extract(Ref<Reader> Input);

        /// \brief Applies a set to an entity.
        ///
        /// \param Actor   The entity, the world entity for singletons.
        /// \param Set     The bytes of the set.
        /// \param Ignored The components read past.
        void Apply(Entity Actor, ConstSpan<Byte> Set, Mask Ignored);

        /// \brief Gets the components a replica ignores, which are the others' view of what this side owns.
        ///
        /// \param Peer The key of the owner.
        /// \return The mask of the `Others` components when this side owns it, none otherwise.
        Mask GetIgnored(UInt64 Peer) const;

        /// \brief Checks whether a peer key is this side's.
        ///
        /// \param Peer The key.
        /// \return `true` if it names this side, `false` otherwise.
        ZY_INLINE Bool IsMine(UInt64 Peer) const
        {
            return Peer != 0 && Peer == mIdentity;
        }

        /// \brief Gets the peer that owns a replica.
        ///
        /// \param Actor The entity.
        /// \return The key of the peer, or zero for nobody.
        static UInt64 GetOwner(Entity Actor);

        /// \brief Gets the key a replica stands under, its own or its root's.
        ///
        /// \param Actor The entity.
        /// \return The key, or zero when nothing above it names one.
        static UInt64 GetKey(Entity Actor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Service>                     mScene;
        Table<UInt64, Entity>            mReplicas;
        Bag<UInt64>                      mKeys;
        Table<UInt64, Sequence<Pending>> mPending;
        Cursor<Set<Replica>>             mLanded;
        ZyScene::Query                   mRoots;
        Sequence<Entity>                 mDoomed;
        Sequence<Byte>                   mUnpacked;
        UInt64                           mIdentity;
        UInt64                           mSection;
        Bool                             mAccepted;
        Bool                             mRejected;
    };
}