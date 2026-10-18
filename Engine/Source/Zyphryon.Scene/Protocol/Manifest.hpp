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

namespace ZyScene::Protocol
{
    /// \brief Represents the table of every component type that travels, in the order both sides declared them.
    class Manifest final
    {
    public:

        /// \brief Names a reader of one pull list, appending the entities it gained since the last call.
        using Drain = Delegate<void(Ref<Sequence<Entity>>)>;

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
            UInt32             Identifier;

            /// The layout and lifecycle of the component, whose save and load write and read its payload.
            ConstPtr<Metatype> Info;

            /// The peers that receive the component.
            Replication        Policy;

            /// The number of ticks between two reads of its changes, one reading on every tick.
            UInt8              Rate;

            /// The function making a world's readers of the component's pull lists.
            Feed            (* Watch)(Ref<Storage> Scene);
        };

    public:

        /// \brief Constructs an empty table.
        Manifest();

        /// \brief Declares a component type replicated, giving it the next place unless it has one.
        ///
        /// \param Policy The peers that receive the component.
        /// \param Rate   The number of ticks between two reads of its changes, one reading on every tick.
        template<typename Type>
        void Register(Replication Policy, UInt8 Rate);

        /// \brief Gets the number of replicated component types.
        ///
        /// \return The number of entries in the table.
        ZY_INLINE UInt8 GetCount() const
        {
            return mCount;
        }

        /// \brief Gets one replicated component type by its place in the table.
        ///
        /// \param Index The place in the table.
        /// \return The entry.
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
        void Write(Ref<Writer> Output, Entity Actor, Mask Wanted, Mask Dropped) const;

        /// \brief Reads a set into an entity, removing what is gone and adding or overwriting what is present.
        ///
        /// \param Input   The reader, at the set, left after it.
        /// \param Actor   The entity, which must be alive, the world entity for singletons.
        /// \param Ignored The components read past whole, present or gone, of the `Others` ones alone.
        /// \return `true` if the set was read whole, `false` if it was cut short or named an unknown component.
        Bool Read(Ref<Reader> Input, Entity Actor, Mask Ignored) const;

        /// \brief Reads past a set without applying it, checking its length fits.
        ///
        /// \param Input The reader, at the set, left after it.
        /// \return `true` if the set fits what is left, `false` if it was cut short.
        Bool Skip(Ref<Reader> Input) const;

    public:

        /// \brief Gets the table, which is one per process like the identifiers of the components themselves.
        ///
        /// \return The table.
        static Ref<Manifest> Get();

    private:

        /// \brief Represents a reader of one pull list, kept inside a \ref Drain.
        template<typename List>
        struct Probe final
        {
            /// The cursor over the list.
            Cursor<List> Source;

            /// \brief Appends every entity the list gained since the last call.
            ///
            /// \param Output The list that receives them.
            ZY_INLINE void operator()(Ref<Sequence<Entity>> Output)
            {
                Source.Each([&](Entity Actor)
                {
                    Output.Append(Actor);
                });
            }
        };

        /// \brief Makes a world's readers of one component's pull lists.
        ///
        /// \param Scene The world.
        /// \return The readers, which see every change from now on.
        template<typename Type>
        static Feed Watch(Ref<Storage> Scene)
        {
            Probe<Set<Type>>     Changes
            {
                Cursor<Set<Type>>(Scene)
            };
            Probe<Removed<Type>> Removals
            {
                Cursor<Removed<Type>>(Scene)
            };
            return Feed(Drain(Move(Changes)), Drain(Move(Removals)));
        }

        /// \brief Writes one component an entity holds itself as an entry of a set.
        ///
        /// \param Output The writer.
        /// \param Actor  The entity, which holds the component itself.
        /// \param Index  The place of the component in the table.
        void WriteEntry(Ref<Writer> Output, Entity Actor, UInt8 Index) const;

        /// \brief Names one component gone in a set, unless the entity holds it again.
        ///
        /// \param Output The writer.
        /// \param Actor  The entity.
        /// \param Index  The place of the component in the table.
        void WriteAbsent(Ref<Writer> Output, Entity Actor, UInt8 Index) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Entry, kMaxComponents> mEntries;
        UInt8                        mCount;
        Mask                         mShared;
        Mask                         mOwner;
        Mask                         mOthers;
    };

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Type>
    void Manifest::Register(Replication Policy, UInt8 Rate)
    {
        const UInt32 Identifier = IdentifierOf<Type>();

        // Registered again, as a second world does, it keeps the place it was given.
        for (UInt8 Index = 0; Index < mCount; ++Index)
        {
            if (mEntries[Index].Identifier == Identifier)
            {
                return;
            }
        }

        ConstRef<Metatype> Info = Registry::Get().GetMetatype(Identifier);

        ZY_ASSERT(mCount < kMaxComponents, "Too many replicated components for one mask");
        ZY_ASSERT(
            CountBits(static_cast<UInt8>(Policy) & 0b111u) == 1,
            "A replicated component needs exactly one audience");
        ZY_ASSERT(Rate > 0, "A replicated component needs a rate of at least one");
        ZY_ASSERT(Info.IsTag() || (Info.Save && Info.Load), "A replicated component must write and read itself");

        const UInt8 Index = mCount++;
        mEntries[Index] = Entry(Identifier, AddressOf(Info), Policy, Rate, & Watch<Type>);

        if (HasBit(Policy, Replication::Owner))
        {
            mOwner.Set(Index);
        }
        else
        {
            mShared.Set(Index);
        }

        if (HasBit(Policy, Replication::Others))
        {
            mOthers.Set(Index);
        }
    }

    /// \brief Declares a component replicated, in the same order on both sides since the order is the wire.
    ///
    /// \param Policy The peers that receive the component.
    /// \param Rate   The number of ticks between two reads of its changes, one reading on every tick.
    template<typename Type>
    ZY_INLINE void Replicate(Replication Policy, UInt8 Rate = 1)
    {
        Manifest::Get().Register<Type>(Policy, Rate);
    }
}