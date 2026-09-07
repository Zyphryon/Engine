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

#include "Manifest.hpp"
#include "Replica.hpp"
#include "Zyphryon.Scene/World.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene::Protocol
{
    /// \brief Represents what the publisher remembers about a replica between publishes.
    ///
    /// A replica brings one along wherever it is attached, so the game never creates or reads it.
    struct Tracker final
    {
        /// The key of the scope the publisher last saw the entity in, zero before.
        UInt64 Scope     = 0;

        /// The components touched since the last publish, one bit each.
        Mask   Touched;

        /// The components removed since the last publish, one bit each.
        Mask   Removed;

        /// The components of a persistent entity that departed from disk, whether changed or removed.
        Mask   Diverged;

        /// Whether the publisher has told the entity's scope about it.
        Bool   Announced = false;

        /// Whether the entity already waits in the ledger for the next publish.
        Bool   Queued    = false;
    };

    /// \brief Represents what the world has to say since the last publish.
    struct Ledger final
    {
        /// The singletons touched since the last publish, one bit each.
        Mask             Touched;

        /// The entities touched since the last publish, each once.
        Sequence<Entity> Pending;

        /// Whether a publisher drains the ledger, else a touch has nobody to reach.
        Bool             Publishing = false;

        /// How many loads from disk are underway, during which what lands is the baseline and not a change.
        UInt32           Restoring  = 0;

        /// \brief Reads or writes nothing, since a ledger is never persisted, but a component that is not trivially
        ///        copyable has to say how it would be.
        ///
        /// \param Archive The archive to read from or write to.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
        }
    };

    /// \brief Runs a load from disk, during which what lands is the baseline the wire measures change against.
    ///
    /// \param World    The world being loaded into.
    /// \param Callback The function that loads.
    template<typename Callable>
    ZY_INLINE void Restore(World World, AnyRef<Callable> Callback)
    {
        Ref<Ledger> Book = World.Get<Ledger>();

        ++Book.Restoring;
        Callback();
        --Book.Restoring;
    }

    /// \brief Puts an entity in line for the next publish, once per tick.
    ///
    /// \param Actor    The entity.
    /// \param Tracking The tracker the entity carries.
    /// \return `true` if what the entity has to say will be looked at, `false` if there is nobody to say it to.
    ZY_INLINE Bool Enlist(Entity Actor, Ref<Tracker> Tracking)
    {
        // The ledger is only reached on the first touch of a tick, so a body moving every frame pays one lookup.
        if (!Tracking.Queued)
        {
            Ref<Ledger> Book = World(Actor.GetWorld()).Get<Ledger>();

            if (!Book.Publishing || Book.Restoring)
            {
                return false;
            }

            Book.Pending.Append(Actor);
            Tracking.Queued = true;
        }
        return true;
    }

    /// \brief Marks a component of an entity changed, on a tracker already in hand.
    ///
    /// \note A system that queries \ref Tracker as a term takes this path and pays no lookup for the touch itself.
    ///
    /// \tparam Type The component that changed.
    /// \param  Actor    The entity the component belongs to.
    /// \param  Tracking The tracker the entity carries.
    template<typename Type>
    ZY_INLINE void Touch(Entity Actor, Ref<Tracker> Tracking)
    {
        if (Enlist(Actor, Tracking))
        {
            Tracking.Touched |= Manifest::Bit<Type>();
        }
    }

    /// \brief Marks a component of an entity changed, so the next publish sends it to whoever sees the entity.
    ///
    /// \note An entity that carries no replica is left alone, so a system may touch without asking first.
    ///
    /// \tparam Type The component that changed.
    /// \param  Actor The entity the component belongs to.
    template<typename Type>
    ZY_INLINE void Touch(Entity Actor)
    {
        if (const Ptr<Tracker> Tracking = Actor.TryGet<Tracker>())
        {
            Touch<Type>(Actor, * Tracking);
        }
    }

    /// \brief Marks a singleton of the world changed, so the next publish sends it to every peer.
    ///
    /// \tparam Type The singleton that changed.
    /// \param  World The world the singleton belongs to.
    template<typename Type>
    ZY_INLINE void Touch(World World)
    {
        World.Get<Ledger>().Touched |= Manifest::Bit<Type>();
    }
}