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

#include "Listener.hpp"
#include "Locus.hpp"
#include "Manifest.hpp"
#include "Replica.hpp"
#include "Wire.hpp"
#include "Zyphryon.Scene/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Represents the side that follows a world it does not own, applying what the publisher says of it.
    class Subscriber final
    {
    public:

        /// \brief Constructs a subscriber over a world, taking in the persistent replicas it already holds.
        ///
        /// \param Scene    The world it follows, outliving the subscriber.
        /// \param Table    The sealed table of what travels, outliving the subscriber.
        /// \param Listener The listener told when what this side plays waits for a replica it has not loaded.
        Subscriber(Ref<ZyScene::Service> Scene, ConstRef<Manifest> Table, ConstRetainer<Listener> Listener);

        /// \brief Applies a frame from the publisher.
        ///
        /// \param Message The payload, exactly as the publisher sent it.
        void Receive(ConstSpan<Byte> Message);

        /// \brief Takes in the persistent replicas loaded since the last call, applying what waited for them.
        void Refresh();

        /// \brief Finds the entity a replica identifier stands for, letting go of one no longer standing.
        ///
        /// \param Identifier The identifier the replica travels under.
        /// \return The entity, or one naming nothing when no living entity carries the identifier.
        ZyScene::Entity Find(UInt64 Identifier);

    private:

        /// \brief Represents a message held for a replica this side does not hold yet, its cell not loaded.
        struct Pending final
        {
            /// The key of the section the message came in, which a leave of that key drops it with.
            UInt64 Key;

            /// The identifier of the replica the message is about, the one updated or the child arriving.
            UInt64 Identifier;

            /// The message, an update of the replica or the arrival of a child beneath it.
            Opcode Code;

            /// The bytes of the message after its opcode, or of the update's set.
            Blob   Data;
        };

        /// The largest frame a packed one may unpack to, which is the largest the network carries.
        static constexpr UInt32 kMaxUnpacked = 1024 * 1024;

        /// The most bytes held for replicas this side does not hold yet, past which the publisher is refused.
        static constexpr UInt32 kMaxHeld     = 4 * kMaxUnpacked;

        /// \brief Takes in one persistent replica, applying the messages that waited for it.
        ///
        /// \param Actor  The entity loaded from disk.
        /// \param Record The identity it travels under, runtime ones skipped.
        void Take(ZyScene::Entity Actor, ConstRef<Replica> Record);

        /// \brief Holds a message for a replica this side does not hold yet, refusing the publisher past \ref kMaxHeld.
        ///
        /// \param Under      The identifier of the replica it waits for.
        /// \param Identifier The identifier of the replica it is about.
        /// \param Code       The message, an update or an arrival.
        /// \param Bytes      The bytes of the message after its opcode, or of the update's set.
        void Hold(UInt64 Under, UInt64 Identifier, Opcode Code, ConstSpan<Byte> Bytes);

        /// \brief Checks whether an arrival of a replica waits, so the updates after it wait too.
        ///
        /// \param Identifier The identifier the replica travels under.
        /// \return `true` if its arrival is held, `false` otherwise.
        Bool IsHeld(UInt64 Identifier) const;

        /// \brief Applies the messages that waited for a replica this side now holds, from one list instead of nesting.
        ///
        /// \param Identifier The identifier the replica travels under, already found by \ref Find.
        void Release(UInt64 Identifier);

        /// \brief Drops the held messages one test picks, counting their bytes off.
        ///
        /// \param Picks The test, given the identifier waited for and the message.
        template<typename Callable>
        void Drop(AnyRef<Callable> Picks)
        {
            mPending.EraseIf([this, &Picks](UInt64 Under, Ref<Sequence<Pending>> Items)
            {
                Items.RemoveSomeIf([this, &Picks, Under](ConstRef<Pending> Item)
                {
                    if (!Picks(Under, Item))
                    {
                        return false;
                    }
                    mHeld -= GetCost(Item.Data.GetSize());
                    return true;
                });
                return Items.IsEmpty();
            });
        }

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

        /// \brief Creates a replica, or overwrites the one already held, landing all it names in one move.
        ///
        /// \param Input The frame, after the opcode.
        void OnEnter(Ref<Reader> Input);

        /// \brief Applies what changed on a replica, or on the world for id zero.
        ///
        /// \param Input The frame, after the opcode.
        void OnUpdate(Ref<Reader> Input);

        /// \brief Destroys a replica that moved to a key this side does not see.
        ///
        /// \param Input The frame, after the opcode.
        void OnMove(Ref<Reader> Input);

        /// \brief Destroys a replica.
        ///
        /// \param Input The frame, after the opcode.
        void OnForget(Ref<Reader> Input);

        /// \brief Sets who owns a replica, tagging it this side's or another's.
        ///
        /// \param Actor The entity whose owner is set.
        /// \param Peer  The key of the owner, or zero for nobody.
        void Claim(ZyScene::Entity Actor, UInt64 Peer);

        /// \brief Destroys every runtime root under a key, or every runtime root for \ref kGlobal.
        ///
        /// \param Key The interest key left, or \ref kGlobal for all of them.
        void Clear(UInt64 Key);

        /// \brief Destroys a replica, letting go of its identifier.
        ///
        /// \param Actor The entity destroyed, which must be alive.
        void Destroy(ZyScene::Entity Actor);

        /// \brief Reads past a set, refusing the publisher when it is not whole.
        ///
        /// \param Input The frame, at the set.
        /// \return The bytes of the set, or none once refused.
        ConstSpan<Byte> Extract(Ref<Reader> Input);

        /// \brief Applies a set to an entity.
        ///
        /// \param Actor   The entity written, the world entity for singletons.
        /// \param Set     The bytes of the set.
        /// \param Ignored The components read past, which this side keeps as it decided them.
        void Apply(ZyScene::Entity Actor, ConstSpan<Byte> Set, Mask Ignored);

        /// \brief Gets the components a replica ignores, which are the others' view of what this side owns.
        ///
        /// \param Peer The key of the owner, or zero for nobody.
        /// \return The mask of the `Others` components when this side owns it, none otherwise.
        Mask GetIgnored(UInt64 Peer) const;

        /// \brief Checks whether a peer key is this side's.
        ///
        /// \param Peer The key of a peer, or zero for nobody.
        /// \return `true` if it names this side, `false` otherwise.
        ZY_INLINE Bool IsMine(UInt64 Peer) const
        {
            return Peer != 0 && Peer == mIdentity;
        }

        /// \brief Gets what a held message counts against \ref kMaxHeld.
        ///
        /// \param Size The bytes of the message.
        /// \return The bytes, with room for the record kept of it.
        ZY_INLINE static UInt GetCost(UInt Size)
        {
            return Size + sizeof(Pending);
        }

        /// \brief Gets the peer that owns a replica.
        ///
        /// \param Actor The entity checked.
        /// \return The key of the peer, or zero for nobody.
        static UInt64 GetOwner(ZyScene::Entity Actor);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<ZyScene::Service>                  mScene;
        ConstRef<Manifest>                     mManifest;
        Retainer<Listener>                     mListener;
        Table<UInt64, ZyScene::Entity>         mReplicas;
        Bag<UInt64>                            mKeys;
        Table<UInt64, Sequence<Pending>>       mPending;
        UInt                                   mHeld;
        Sequence<UInt64>                       mReleases;
        ZyScene::Cursor<ZyScene::Set<Replica>> mLanded;
        ZyScene::Query                         mRoots;
        Sequence<Byte>                         mUnpacked;
        UInt64                                 mIdentity;
        UInt64                                 mSection;
        Bool                                   mAccepted;
        Bool                                   mRejected;
    };
}