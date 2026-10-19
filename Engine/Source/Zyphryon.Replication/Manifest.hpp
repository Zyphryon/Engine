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
#include "Zyphryon.Scene/World.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief The most component types that can replicate, one bit each in a mask.
    inline constexpr UInt8 kMaxComponents = 128;

    /// \brief Names a set of replicated components, one bit per place in the manifest.
    using Mask = Bitset<kMaxComponents, UInt64>;

    /// \brief Represents the table of every component type that travels, ordered by name once a session opens.
    class Manifest final
    {
    public:

        /// \brief Names a reader of one pull list, appending the entities it gained since the last call.
        using Drain = Delegate<void(Ref<Sequence<ZyScene::Entity>>)>;

        /// \brief Represents a world's readers of one component's pull lists.
        struct Feed final
        {
            /// The reader of the entities that came to hold the component or wrote it.
            Drain Changes;

            /// The reader of the entities that lost the component, destroyed ones included.
            Drain Removals;
        };

        /// \brief Represents one replicated component type, with everything the wire needs resolved once.
        struct Entry final
        {
            /// The identifier of the component in the registry.
            UInt32                      Identifier;

            /// The layout and lifecycle of the component, whose save and load write and read its payload.
            ConstPtr<ZyScene::Metatype> Info;

            /// The peers that receive the component.
            Audience                    Policy;

            /// The number of ticks between two reads of its changes, one reading on every tick.
            UInt8                       Rate;

            /// The function making a world's readers of the component's pull lists.
            Feed                        (* Watch)(Ref<ZyScene::Storage> Scene);
        };

    public:

        /// \brief Constructs an empty table.
        Manifest();

        /// \brief Declares a component type replicated, once, before the table is sealed.
        ///
        /// \param Policy The peers that receive the component.
        /// \param Rate   The number of ticks between two reads of its changes, one reading on every tick.
        template<typename Type>
        void Register(Audience Policy, UInt8 Rate);

        /// \brief Orders the table by type name and builds its masks, so both sides agree whatever order they declared.
        void Seal();

        /// \brief Gets the number of replicated component types.
        ///
        /// \return The number of entries in the table.
        ZY_INLINE UInt8 GetCount() const
        {
            return mCount;
        }

        /// \brief Gets one replicated component type by its place in the table.
        ///
        /// \param Index The place in the table, below \ref GetCount.
        /// \return The entry, which holds while the table stands.
        ZY_INLINE ConstRef<Entry> GetEntry(UInt8 Index) const
        {
            ZY_ASSERT(Index < mCount, "Wire index names no replicated component");
            return mEntries[Index];
        }

        /// \brief Gets the components written into a key's channel, which every peer that sees the entity reads.
        ///
        /// \return The mask of the `Everyone` and `Others` components.
        ZY_INLINE Mask GetSharedMask() const
        {
            return mShared;
        }

        /// \brief Gets the components written into the owner's private channel alone.
        ///
        /// \return The mask of the `Owner` components.
        ZY_INLINE Mask GetOwnerMask() const
        {
            return mOwner;
        }

        /// \brief Gets the components the owner of an entity ignores.
        ///
        /// \return The mask of the `Others` components.
        ZY_INLINE Mask GetOthersMask() const
        {
            return mOthers;
        }

        /// \brief Checks whether a component's changes are read on a tick, which its rate and place spread.
        ///
        /// \param Index The place of the component in the table.
        /// \param Tick  The number of the publish being built.
        /// \return `true` if its changes are read on this tick, `false` if they wait in the list.
        ZY_INLINE Bool IsDue(UInt8 Index, UInt64 Tick) const
        {
            return (Tick + Index) % GetEntry(Index).Rate == 0;
        }

        /// \brief Gets the hash of the table, which both sides compare before anything else travels.
        ///
        /// \return The hash of every entry's name, size and audience, in order.
        UInt64 GetHash() const;

        /// \brief Writes a set behind its length, what the entity lacks first and what its owner ignores last.
        ///
        /// \param Output  The writer to append the set to.
        /// \param Actor   The entity, the world entity for singletons.
        /// \param Wanted  The components to write, named gone when the entity does not hold them.
        /// \param Dropped The components the entity lost, named gone unless it holds them again.
        void Write(Ref<Writer> Output, ZyScene::Entity Actor, Mask Wanted, Mask Dropped) const;

        /// \brief Reads a set into an entity, removing what is gone and adding or overwriting what is present.
        ///
        /// \param Input   The reader, at the set, left after it.
        /// \param Actor   The entity, which must be alive, the world entity for singletons.
        /// \param Ignored The components read past whole, present or gone, of the `Others` ones alone.
        /// \return `true` if the set was read whole, `false` if it was cut short or named an unknown component.
        Bool Read(Ref<Reader> Input, ZyScene::Entity Actor, Mask Ignored) const;

        /// \brief Reads past a set without applying it, checking its length fits.
        ///
        /// \param Input The reader, at the set, left after it.
        /// \return `true` if the set fits what is left, `false` if it was cut short.
        Bool Skip(Ref<Reader> Input) const;

    private:

        /// \brief Represents a reader of one pull list, kept inside a \ref Drain.
        template<typename List>
        struct Probe final
        {
            /// The cursor over the list.
            ZyScene::Cursor<List> Source;

            /// \brief Appends every entity the list gained since the last call.
            ///
            /// \param Output The list that receives them.
            ZY_INLINE void operator()(Ref<Sequence<ZyScene::Entity>> Output)
            {
                Source.Each([&](ZyScene::Entity Actor)
                {
                    Output.Append(Actor);
                });
            }
        };

        /// The byte that closes the absent entries of a set, past every place in the table.
        static constexpr UInt8 kEnd = 0xFF;

    public:

        /// \brief Makes a world's readers of one component's pull lists.
        ///
        /// \param Scene The world whose changes are read.
        /// \return The readers, which see every change from now on.
        template<typename Type>
        static Feed Watch(Ref<ZyScene::Storage> Scene)
        {
            Probe<ZyScene::Set<Type>>     Changes
            {
                ZyScene::Cursor<ZyScene::Set<Type>>(Scene)
            };
            Probe<ZyScene::Removed<Type>> Removals
            {
                ZyScene::Cursor<ZyScene::Removed<Type>>(Scene)
            };
            return Feed(Drain(Move(Changes)), Drain(Move(Removals)));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Entry, kMaxComponents> mEntries;
        UInt8                        mCount;
        Mask                         mShared;
        Mask                         mOwner;
        Mask                         mOthers;
        Bool                         mSealed;
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    void Manifest::Register(Audience Policy, UInt8 Rate)
    {
        ZY_ASSERT(!mSealed, "A component cannot be replicated once a session opened");

        const UInt32 Identifier = ZyScene::IdentifierOf<Type>();

        // Declared again, it keeps the entry it has.
        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            if (mEntries[Index].Identifier == Identifier)
            {
                return;
            }
        }

        ConstRef<ZyScene::Metatype> Info = ZyScene::Registry::Get().GetMetatype(Identifier);

        ZY_ASSERT(mCount < kMaxComponents, "Too many replicated components for one mask");
        ZY_ASSERT(Rate > 0, "A replicated component needs a rate of at least one");
        ZY_ASSERT(Info.IsTag() || (Info.Save && Info.Load), "A replicated component must write and read itself");

        mEntries[mCount++] = Entry(Identifier, AddressOf(Info), Policy, Rate, & Watch<Type>);
    }
}