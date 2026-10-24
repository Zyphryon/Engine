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

#include "Replica.hpp"
#include "Zyphryon.Scene/Entity.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyReplication
{
    /// \brief Represents a reference to another replica, held inside a replicated component.
    class Link final
    {
    public:

        /// \brief Constructs a link naming nothing.
        ZY_INLINE Link()
            : mIdentifier { 0 }
        {
        }

        /// \brief Constructs a link to an entity of this world.
        ///
        /// \param Target The entity, which names its replica identity once it has one.
        ZY_INLINE explicit Link(ZyScene::Entity Target)
            : mCache      { Target },
              mIdentifier { 0 }
        {
            const ConstPtr<Replica> Record = Target.IsAlive() ? Target.Get<const Replica>() : nullptr;

            // A persistent identity outlives its entity, so the link keeps naming it once the target unloads.
            if (Record && Record->IsPersistent())
            {
                mIdentifier = Record->GetIdentifier();
            }
        }

        /// \brief Gets the identifier the link names now.
        ///
        /// \return The live target's identity, else the identifier it travelled as, or zero for none.
        ZY_INLINE UInt64 GetIdentifier() const
        {
            const ConstPtr<Replica> Record = mCache.IsAlive() ? mCache.Get<const Replica>() : nullptr;

            if (Record && Record->IsValid())
            {
                return Record->GetIdentifier();
            }
            return mIdentifier;
        }

        /// \brief Finds the entity the link names, caching it.
        ///
        /// \param Find The function finding the entity a replica identifier stands for.
        /// \return The entity, or one naming nothing while the target is not held.
        template<typename Callable>
        ZY_INLINE ZyScene::Entity Resolve(AnyRef<Callable> Find)
        {
            // Never reused, an identifier names the same replica again when it comes back as another entity.
            if (!mCache.IsAlive() && mIdentifier != 0)
            {
                mCache = Find(mIdentifier);
            }

            if (mCache.IsAlive())
            {
                return mCache;
            }
            return ZyScene::Entity();
        }

        /// \brief Writes or reads the identifier the link names.
        ///
        /// \param Archive The archive to write to or read from.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            if constexpr (Serializer::IsWriter)
            {
                UInt64 Identifier = GetIdentifier();

                // A runtime identifier it only remembers names nothing once its target is gone.
                if (!mCache.IsAlive() && (Identifier & kPersistent) == 0)
                {
                    Identifier = 0;
                }
                Archive.Serialize(Identifier);
            }
            else
            {
                Archive.Serialize(mIdentifier);
                mCache = ZyScene::Entity();
            }
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ZyScene::Entity mCache;
        UInt64          mIdentifier;
    };
}