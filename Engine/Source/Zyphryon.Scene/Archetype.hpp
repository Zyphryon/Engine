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

        /// \brief Gets the unique numeric identifier of this archetype.
        ///
        /// \return The archetype's unique identifier.
        ZY_INLINE UInt64 GetID() const
        {
            return mHandle.GetID();
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
            return mHandle.IsValid() && mHandle.IsArchetype();
        }

        /// \brief Checks whether this archetype's underlying entity is currently alive (not destroyed).
        ///
        /// \return `true` if the entity is alive, `false` otherwise.
        ZY_INLINE Bool IsAlive() const
        {
            return mHandle.IsAlive();
        }

        /// \brief Destroys this archetype and its entire fixed-part subtree.
        ///
        /// \note Only the definition is removed, live instances spawned from this archetype are left dangling.
        ZY_INLINE void Destruct() const
        {
            UnlockRecursively();
            mHandle.Destruct();
        }

        /// \brief Invalidates this archetype's cached tree spawner, forcing it to be rebuilt on next instantiation.
        ZY_INLINE void Invalidate() const
        {
            Unlock();
        }

        /// \brief Enables this archetype, allowing it to be processed by systems that require it to be awake.
        ///
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Awake() const
        {
            mHandle.Awake();
            return (* this);
        }

        /// \brief Disables this archetype, preventing it from being processed by systems that require it to be awake.
        ///
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Sleep() const
        {
            mHandle.Sleep();
            return (* this);
        }

        /// \brief Checks whether this archetype is currently awake (enabled).
        ///
        /// \return `true` if the archetype is awake, `false` otherwise.
        ZY_INLINE Bool IsAwake() const
        {
            return mHandle.IsAwake();
        }

        /// \brief Looks up a fixed part of this archetype by its name.
        ///
        /// \param Name The name of the part to find.
        /// \return The part as an archetype, or an invalid archetype if not found.
        ZY_INLINE Archetype Lookup(Text Name) const
        {
            return Archetype(mHandle.Lookup(Name));
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

        /// \brief Gets the parent archetype of this archetype in the hierarchy.
        ///
        /// \return The parent as an archetype, or an invalid archetype if there is none.
        ZY_INLINE Archetype GetParent() const
        {
            return Archetype(mHandle.GetParent());
        }

        /// \brief Gets the archetype this archetype inherits from, if any.
        ///
        /// \return The inherited archetype, or an invalid archetype if there is none.
        ZY_INLINE Archetype GetArchetype() const
        {
            return Archetype(mHandle.GetArchetype());
        }

        /// \brief Sets the internal name of this archetype, used for lookups and identification.
        ///
        /// \param Name The name to assign.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype SetName(Text Name) const
        {
            mHandle.SetName(Name);
            return (* this);
        }

        /// \brief Gets the internal name of this archetype.
        ///
        /// \return The archetype's name, or an empty string if it has none.
        ZY_INLINE Text GetName() const
        {
            return mHandle.GetName();
        }

        /// \brief Sets a human-readable display name (alias) for this archetype, separate from its internal name.
        ///
        /// \param Name The display name to assign.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype SetAlias(Text Name) const
        {
            mHandle.SetAlias(Name);
            return (* this);
        }

        /// \brief Gets the human-readable display name (alias) of this archetype.
        ///
        /// \return The alias string, or an empty string if none was set.
        ZY_INLINE Text GetAlias() const
        {
            return mHandle.GetAlias();
        }

        /// \brief Gets a hash value for this archetype based on its unique identifier.
        ///
        /// \return The archetype's unique identifier used as its hash.
        ZY_INLINE UInt64 Hash(UInt64 Seed) const
        {
            return mHandle.Hash(Seed);
        }

        /// \brief Equals operator comparing two archetypes by their unique identifiers.
        ZY_INLINE Bool operator==(ConstRef<Archetype> Other) const
        {
            return mHandle == Other.mHandle;
        }

        /// \brief Inequality operator comparing two archetypes by their unique identifiers.
        ZY_INLINE Bool operator!=(ConstRef<Archetype> Other) const = default;

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
