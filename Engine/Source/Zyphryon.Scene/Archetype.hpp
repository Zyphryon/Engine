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
            UnlockRecursively();
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
            const Ptr<ecs_world_t> World = mHandle.GetWorld();

            ecs_iter_t Iterator = ecs_each_id(World, ecs_pair(EcsIsA, mHandle.GetID()));

            while (ecs_each_next(AddressOf(Iterator)))
            {
                for (SInt32 Element = 0; Element < Iterator.count; ++Element)
                {
                    Callback(Entity(World, Iterator.entities[Element]));
                }
            }
        }

        /// \brief Attaches an existing archetype as a fixed part of this one and stands it under every live instance.
        ///
        /// \param Part The archetype to attach as a fixed child. Allocated by \ref Service::CreateArchetype.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Attach(Archetype Part) const
        {
            Unlock();
            Part.mHandle.Attach(mHandle, Hierarchy::Fixed);

            // A prefab's children reach an instance only as its archetype link is made, so the ones already
            // standing are given the new part by hand.
            Children([World = mHandle.GetWorld(), Part](Entity Instance)
            {
                Entity(World, ecs_new(World))
                    .SetAlias(Part.GetAlias())
                    .SetArchetype(Part.mHandle)
                    .Attach(Instance, Hierarchy::Fixed);
            });
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

        /// \brief Lifts a fixed part out of this archetype without destroying it, leaving it a root of its own.
        ///
        /// \param Part The part archetype to release. Must be a fixed child of this archetype.
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Release(Archetype Part) const
        {
            Unlock();
            Part.UnlockRecursively();
            Part.mHandle.Detach();
            return (* this);
        }

        /// \brief Gives every live instance the components this archetype and its bases carry that it does not own.
        ///
        /// \note An instance only ever receives copies at the moment its archetype link is created.
        ///
        /// \return This archetype, allowing for method chaining.
        ZY_INLINE Archetype Reapply() const
        {
            Children([this](Entity Instance)
            {
                for (Archetype Base = (* this); Base.IsValid(); Base = Base.GetArchetype())
                {
                    Base.mHandle.Each([Instance](Entity Component)
                    {
                        if (!Component.IsPair() && Component.IsOverridable() && !Instance.Owns(Component))
                        {
                            Instance.Add(Component);
                        }
                    });
                }
            });
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

        /// \brief Checks whether this archetype stands somewhere under another in the hierarchy.
        ///
        /// \param Ancestor The archetype to look for above this one.
        /// \return `true` when \p Ancestor is a parent of this one at any depth, `false` otherwise.
        ZY_INLINE Bool IsDescendantOf(Archetype Ancestor) const
        {
            for (Archetype Walk = GetParent(); Walk.IsValid(); Walk = Walk.GetParent())
            {
                if (Walk == Ancestor)
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Checks whether this archetype inherits from another, however far back.
        ///
        /// \param Base The archetype to look for along the chain this one inherits through.
        /// \return `true` when this one inherits from \p Base at any remove, `false` otherwise.
        ZY_INLINE Bool IsInstanceOf(Archetype Base) const
        {
            for (Archetype Walk = GetArchetype(); Walk.IsValid(); Walk = Walk.GetArchetype())
            {
                if (Walk == Base)
                {
                    return true;
                }
            }
            return false;
        }

        /// \brief Gets the archetype this one hangs from, which is itself when nothing stands above it.
        ///
        /// \return The topmost archetype of the hierarchy this one belongs to.
        ZY_INLINE Archetype GetRoot() const
        {
            Archetype Root(* this);

            for (Archetype Walk = Root.GetParent(); Walk.IsValid(); Walk = Walk.GetParent())
            {
                Root = Walk;
            }
            return Root;
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
            if (const Entity Spawner(ecs_id(EcsTreeSpawner)); mHandle.Has(Spawner))
            {
                mHandle.Remove(Spawner);
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