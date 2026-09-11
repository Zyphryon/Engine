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
#include "Zyphryon.Scene/Component.hpp"
#include "Zyphryon.Scene/Factory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Represents the table of every component type that travels, in the order both sides declared them.
    class Manifest final
    {
    public:

        /// \brief Marks a component type that was never declared replicated.
        static constexpr UInt8 kInvalid = 0xFF;

        /// \brief Represents one replicated component type, with everything the wire needs resolved once.
        struct Entry final
        {
            /// The identifier of the component in the world.
            ecs_entity_t Component;

            /// Whether the component carries no data, so only its presence travels.
            Bool         Tag;

            /// The serializer the component is written and read through.
            Factory      Serializer;
        };

    public:

        /// \brief Declares a component type replicated, giving it the next place in the table.
        ///
        /// \note A replicated component is made serializable, since that is how its bytes are produced.
        ///
        /// \param World  The world the component belongs to.
        /// \param Policy How the component travels.
        template<typename Type>
        ZY_INLINE void Register(Ptr<ecs_world_t> World, Replication Policy)
        {
            using Component = StripAll<Type>;

            const Scene::Component<Component> Handle(World, _::Identify<Component>(World));
            Handle.Grant(Trait::Serializable);

            // A second world declaring the same type keeps the place the first one was given.
            if (Place<Component> != kInvalid)
            {
                return;
            }

            ZY_ASSERT(mCount < kMaxComponents, "Too many replicated components for one dirty mask");
            ZY_ASSERT(
                HasBit(Policy, Replication::Self)
             || HasBit(Policy, Replication::Others), "A replicated component needs an audience");

            const UInt8 Index = mCount++;
            Place<Component> = Index;

            mEntries[Index] = Entry(Handle.GetID(), ecs_id_is_tag(World, Handle.GetID()), Factory::Create<Component>());

            mAll.Set(Index);

            if (HasBit(Policy, Replication::Self))
            {
                mSelf.Set(Index);
            }
            if (HasBit(Policy, Replication::Others))
            {
                mOthers.Set(Index);
            }
            if (HasBit(Policy, Replication::Once))
            {
                mOnce.Set(Index);
            }
            if (HasBit(Policy, Replication::Streamed))
            {
                mStream.Set(Index);
            }

            // The name rather than the identifier, since identifiers are handed out in whatever order the world
            // registers and the table only has to agree with the other side's declarations.
            mHash = HashCombine(mHash, Handle.GetName().Hash(), static_cast<UInt64>(Policy));
        }

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
            ZY_ASSERT(Index < mCount, "Wire identifier names no replicated component");
            return mEntries[Index];
        }

        /// \brief Gets the mask naming every component in the table.
        ///
        /// \return The mask.
        ZY_INLINE Mask GetMask() const
        {
            return mAll;
        }

        /// \brief Gets the bits of every component the owner of an entity receives.
        ///
        /// \return The mask.
        ZY_INLINE Mask GetSelfMask() const
        {
            return mSelf;
        }

        /// \brief Gets the bits of every component a peer that does not own an entity receives.
        ///
        /// \return The mask.
        ZY_INLINE Mask GetOthersMask() const
        {
            return mOthers;
        }

        /// \brief Gets the bits of every component that only travels with a spawn.
        ///
        /// \return The mask.
        ZY_INLINE Mask GetOnceMask() const
        {
            return mOnce;
        }

        /// \brief Gets the bits of every component that travels unreliably.
        ///
        /// \return The mask.
        ZY_INLINE Mask GetStreamMask() const
        {
            return mStream;
        }

        /// \brief Gets the hash of the table, which both sides compare before anything else travels.
        ///
        /// \return The hash.
        ZY_INLINE UInt64 GetHash() const
        {
            return mHash;
        }

    public:

        /// \brief Gets the table, which is one per process like the identifiers of the components themselves.
        ///
        /// \return The table.
        ZY_INLINE static Ref<Manifest> Get()
        {
            static Manifest Instance;
            return Instance;
        }

        /// \brief Gets the bit a component type occupies in every dirty mask.
        ///
        /// \return The bit, or an empty mask when the type was never declared replicated.
        template<typename Type>
        ZY_INLINE static Mask Bit()
        {
            const UInt8 Index = Place<StripAll<Type>>;
            return Index != kInvalid ? BitOf(Index) : Mask();
        }

    private:

        /// The place one component type occupies in the table, or \ref kInvalid until it is declared replicated.
        template<typename Type>
        static inline UInt8 Place = kInvalid;

        /// \brief Constructs an empty table.
        ZY_INLINE Manifest()
            : mCount { 0 },
              mHash  { 0 }
        {
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Entry, kMaxComponents> mEntries;
        UInt8                        mCount;
        Mask                         mAll;
        Mask                         mSelf;
        Mask                         mOthers;
        Mask                         mOnce;
        Mask                         mStream;
        UInt64                       mHash;
    };
}

namespace Scene::DSL
{
    /// \brief Represents the declaration of a component that travels between peers, under one policy.
    ///
    /// \tparam Value How the component travels.
    template<Protocol::Replication Value>
    struct Replicating final
    {
        /// \brief Declares one component replicated, giving it its place on the wire.
        ///
        /// \tparam Type  The component that travels.
        /// \param  World The world the component belongs to.
        template<typename Type>
        ZY_INLINE static void Apply(Ptr<ecs_world_t> World)
        {
            Protocol::Manifest::Get().Register<Type>(World, Value);
        }
    };

    /// \brief Declares a component replicated under a policy, beside the traits it carries.
    ///
    /// \note Both sides must declare their replicated components in the same order, since the order is the wire.
    ///
    /// \tparam Value How the component travels.
    template<Protocol::Replication Value>
    inline constexpr Replicating<Value> Replicated { };
}