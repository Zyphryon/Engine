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

#include "Entity.hpp"
#include "Tag.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief A thin editing view over an archetype (prefab) entity that owns the rules for restructuring it.
    class Archetype final
    {
    public:

        /// \brief Constructs an invalid archetype with no associated entity.
        ZY_INLINE Archetype() = default;

        /// \brief Constructs an archetype view over an existing entity.
        ///
        /// \param Actor The entity to view as an archetype. Must be a valid archetype to be edited.
        ZY_INLINE Archetype(Entity Actor)
            : mHandle { Actor }
        {
        }

        /// \brief Gets the underlying entity this view refers to.
        ///
        /// \return The archetype entity.
        ZY_INLINE Entity GetEntity() const
        {
            return mHandle;
        }

        /// \brief Checks whether this view refers to a live archetype entity.
        ///
        /// \return `true` if the entity is alive and marked as an archetype, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return mHandle.IsAlive() && mHandle.IsArchetype();
        }

        /// \brief Destroys this archetype and its entire fixed-part subtree.
        ///
        /// \note Only the definition is removed, live instances spawned from this archetype are left dangling.
        ZY_INLINE void Destruct() const
        {
            UnlockRecursively();
            mHandle.Destruct();
        }

        /// \brief Attaches an existing archetype as a fixed part of this one.
        ///
        /// \param Part The archetype to attach as a fixed child. Allocated by \ref Service::CreateArchetype.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Attach(Archetype Part) const
        {
            Unlock();
            Part.mHandle.Attach(mHandle, Hierarchy::Fixed);
            return (* this);
        }

        /// \brief Removes and destroys a fixed part of this archetype.
        ///
        /// \param Part The part archetype to remove. Must be a fixed child of this archetype.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Detach(Archetype Part) const
        {
            Unlock();
            Part.Destruct();
            return (* this);
        }

        /// \brief Marks this archetype and its fixed parts as deprecated.
        ///
        /// A deprecated archetype stays fully resolvable (so existing regions keep loading) but is meant to be
        /// hidden from authoring and reclaimed later by a purge pass. The mark never propagates to instances.
        ///
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Deprecate() const
        {
            Entity::AddRecursively<Deprecated>(mHandle);
            return (* this);
        }

        /// \brief Clears the deprecated mark from this archetype and its fixed parts.
        ///
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Restore() const
        {
            Entity::RemoveRecursively<Deprecated>(mHandle);
            return (* this);
        }

        /// \brief Checks whether this archetype has been marked deprecated.
        ///
        /// \return `true` if the archetype is deprecated, `false` otherwise.
        ZY_INLINE Bool IsDeprecated() const
        {
            return mHandle.Has<Deprecated>();
        }

        /// \brief Iterates over every live instance spawned from this archetype.
        ///
        /// \param Callback The function to call for each instance entity.
        template<typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            mHandle.GetHandle().world().query_builder<>()
                .with(flecs::IsA, mHandle.GetID())
                .each([& Callback](flecs::entity Instance)
                {
                    Callback(Entity(Instance));
                });
        }

        /// \brief Destroys every live instance spawned from this archetype, leaving the definition intact.
        ///
        /// \note Only affects instances currently in memory; those unloaded are handled defensively on load.
        ZY_INLINE void Despawn() const
        {
            Sequence<Entity> Doomed;

            Children([&](Entity Instance)
            {
                Doomed.Append(Instance);
            });

            for (Entity Instance : Doomed)
            {
                Instance.Destruct();
            }
        }

    private:

        /// \brief Drops this archetype's cached tree spawner, unfreezing its part set for restructuring.
        ///
        /// A no-op when the archetype was never instantiated (no cache exists).
        ZY_INLINE void Unlock() const
        {
            if (const Entity::Handle Handle = mHandle.GetHandle(); Handle.has(ecs_id(EcsTreeSpawner)))
            {
                Handle.remove(ecs_id(EcsTreeSpawner));
            }
        }

        /// \brief Drops the cached tree spawner across this archetype and all of its fixed parts.
        ///
        /// The freeze assert walks the ancestor chain, so tearing down a multi-level assembly needs every node
        /// in the subtree unlocked before any child leaves it.
        ZY_INLINE void UnlockRecursively() const
        {
            Unlock();

            mHandle.Children([](Entity Child)
            {
                Archetype(Child).UnlockRecursively();
            });
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Entity mHandle;
    };
}
