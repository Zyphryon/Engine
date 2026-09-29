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
#include "Handle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the pull lists of a world, one per component and change, and the position of every reader.
    class ZY_API Ledger final
    {
    public:

        /// \brief Constructs a ledger with no reader, which records nothing.
        Ledger();

        /// \brief Destroys every value still kept.
        ~Ledger();

        /// \brief Ledgers are not copied, since entries own the values they keep.
        Ledger(ConstRef<Ledger> Other) = delete;

        /// \brief Ledgers are not assigned, since entries own the values they keep.
        Ref<Ledger> operator=(ConstRef<Ledger> Other) = delete;

        /// \brief Checks whether any reader watches a component for some changes, which is all an unwatched write pays.
        ///
        /// \param Identifier The component.
        /// \param Kinds      The changes.
        /// \return `true` if one does, `false` otherwise.
        ZY_INLINE Bool IsWatched(UInt32 Identifier, Pull Kinds) const
        {
            return Identifier < mWatched.GetSize() && (mWatched[Identifier] & static_cast<UInt8>(Kinds));
        }

        /// \brief Checks whether any reader exists at all.
        ///
        /// \return `true` if one does, `false` otherwise.
        ZY_INLINE Bool IsActive() const
        {
            return mCount > 0;
        }

        /// \brief Makes a reader, which sees every change recorded from now on.
        ///
        /// \param Identifier The component.
        /// \param Kind       The change, exactly one.
        /// \return The handle of the subscription, never zero.
        UInt32 Subscribe(UInt32 Identifier, Pull Kind);

        /// \brief Destroys a reader, letting its list drop what only it had left to read.
        ///
        /// \param Subscription The handle of the subscription, or zero.
        void Unsubscribe(UInt32 Subscription);

        /// \brief Records a change in the list some reader watches.
        ///
        /// \param Identifier The component.
        /// \param Kind       The change, exactly one.
        /// \param Actor      The entity.
        /// \param Value      The value leaving on a removal, copied when the component keeps it, or `nullptr`.
        void Record(UInt32 Identifier, Pull Kind, Handle Actor, ConstPtr<Byte> Value);

        /// \brief Gets the position of the first entry a reader has not read, counted from the list's first ever.
        ///
        /// \param Subscription The handle of the subscription.
        /// \return The position.
        ZY_INLINE UInt64 GetPosition(UInt32 Subscription) const
        {
            return mSubscriptions[Subscription - 1].Position;
        }

        /// \brief Gets the position past the last entry of the list a reader watches.
        ///
        /// \param Subscription The handle of the subscription.
        /// \return The position.
        ZY_INLINE UInt64 GetEnd(UInt32 Subscription) const
        {
            ConstRef<List> Target = * mLists[mSubscriptions[Subscription - 1].List];
            return Target.First + Target.Actors.GetSize();
        }

        /// \brief Gets the entity an entry of the list a reader watches is about.
        ///
        /// \param Subscription The handle of the subscription.
        /// \param Position     The position of the entry, which the reader has not read yet.
        /// \return The entity, whose generation tells when the slot was reused.
        ZY_INLINE Handle GetActor(UInt32 Subscription, UInt64 Position) const
        {
            ConstRef<List> Target = * mLists[mSubscriptions[Subscription - 1].List];
            return Target.Actors[Position - Target.First];
        }

        /// \brief Gets the value that left with an entry of the list a reader watches.
        ///
        /// \param Subscription The handle of the subscription.
        /// \param Position     The position of the entry, which the reader has not read yet.
        /// \return The value, kept only for a removal of a component declared to keep it, or `nullptr`.
        ZY_INLINE ConstPtr<Byte> GetValue(UInt32 Subscription, UInt64 Position) const
        {
            ConstRef<List> Target = * mLists[mSubscriptions[Subscription - 1].List];
            const UInt64   Index  = Position - Target.First;
            return Index < Target.Values.GetSize() ? Target.Values[Index] : nullptr;
        }

        /// \brief Moves a reader past what it read, dropping what every reader of the list is past.
        ///
        /// \param Subscription The handle of the subscription.
        /// \param Position     The position of the first entry it has not read.
        void SetPosition(UInt32 Subscription, UInt64 Position);

        /// \brief Destroys every value kept for a component before its code goes, leaving the entries without one.
        ///
        /// \param Identifier The component.
        void Retire(UInt32 Identifier);

    private:

        /// The list a destroyed reader names, so its handle can be handed out again.
        static constexpr UInt32 kVacant = 0xFFFFFFFF;

        /// \brief Represents one list, the entries some reader has still to read.
        struct List final
        {
            /// The component it records changes of.
            UInt32              Identifier;

            /// The readers of the list.
            UInt32              Subscribers;

            /// The position of the first entry held.
            UInt64              First;

            /// The entity of each entry, the first one at \ref First.
            Sequence<Handle>    Actors;

            /// The value each entry kept, held only by removals of a component that keeps them and never longer.
            Sequence<Ptr<Byte>> Values;

            /// \brief Constructs a list that holds no entry yet.
            ///
            /// \param Identifier The component it records changes of.
            ZY_INLINE explicit List(UInt32 Identifier)
                : Identifier  { Identifier },
                  Subscribers { 0 },
                  First       { 0 }
            {
            }
        };

        /// \brief Represents where one reader is.
        struct Subscription final
        {
            /// The list it reads, or \ref kVacant once the reader is gone.
            UInt32 List;

            /// The position of the first entry it has not read.
            UInt64 Position;
        };

        /// \brief Gets the index a list of a component and change sits at.
        ///
        /// \param Identifier The component.
        /// \param Kind       The change, exactly one.
        /// \return The index.
        ZY_INLINE static UInt32 GetListIndex(UInt32 Identifier, Pull Kind)
        {
            return Identifier * 3 + (static_cast<UInt32>(Kind) >> 1);
        }

        /// \brief Drops the entries every reader of a list is past, once they are at least half of it.
        ///
        /// \param Index The index of the list.
        void Trim(UInt32 Index);

        /// \brief Drops a number of entries from the front of a list, destroying the values they kept.
        ///
        /// \param Index The index of the list.
        /// \param Count The number of entries.
        void RemoveFront(UInt32 Index, UInt Count);

        /// \brief Drops a number of elements from the front of a sequence, keeping its storage.
        ///
        /// \param Items The sequence.
        /// \param Count The number of elements, at most as many as it holds.
        template<typename Type>
        ZY_INLINE static void DropFront(Ref<Sequence<Type>> Items, UInt Count)
        {
            if (Count == Items.GetSize())
            {
                Items.Clear();
            }
            else
            {
                Items.Remove(0, Count);
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<UInt8>        mWatched;
        Sequence<Unique<List>> mLists;
        Sequence<Subscription> mSubscriptions;
        UInt32                 mCount;
    };
}