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

#include "Registry.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents an entity within the ECS (Entity-Component System).
    ///
    /// An entity is a lightweight handle or identifier that serves as a container for components.
    class Entity
    {
    public:

        /// \brief Underlying handle type used to represent the entity internally.
        using Handle = ecs_entity_t;

    public:

        /// \brief Constructs an invalid entity with no associated world or components.
        ZY_INLINE Entity()
            : mWorld  { nullptr },
              mHandle { 0 }
        {
        }

        /// \brief Constructs an entity from a raw identifier that carries no world of its own.
        ///
        /// \note This is how a built-in flecs identifier reaches the wrapper, since those need no world to resolve.
        ///
        /// \param Handle The raw entity identifier.
        ZY_INLINE Entity(Handle Handle)
            : mWorld  { nullptr },
              mHandle { Handle }
        {
        }

        /// \brief Constructs an entity bound to the world that issued it.
        ///
        /// \param World  The world the entity belongs to.
        /// \param Handle The raw entity identifier.
        ZY_INLINE Entity(Ptr<ecs_world_t> World, Handle Handle)
            : mWorld  { World },
              mHandle { Handle }
        {
        }

        /// \brief Gets the unique numeric identifier of this entity.
        ///
        /// \return The entity's unique identifier.
        ZY_INLINE UInt64 GetID() const
        {
            return mHandle;
        }

        /// \brief Gets the internal handle representing this entity.
        ///
        /// \return The entity internal handle.
        ZY_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the world this entity belongs to.
        ///
        /// \return The world of this entity, or null if the entity carries none.
        ZY_INLINE Ptr<ecs_world_t> GetWorld() const
        {
            return mWorld;
        }

        /// \brief Checks if this entity is valid (exists in the world).
        ///
        /// \return `true` if the entity is valid, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return mWorld && ecs_is_valid(mWorld, mHandle);
        }

        /// \brief Checks if this entity is currently alive (not destroyed).
        ///
        /// \return `true` if the entity is alive, `false` otherwise.
        ZY_INLINE Bool IsAlive() const
        {
            return mWorld && ecs_is_alive(mWorld, mHandle);
        }

        /// \brief Checks if this entity represents an archetype.
        ///
        /// \return `true` if the entity is an archetype, `false` otherwise.
        ZY_INLINE Bool IsArchetype() const
        {
            return ecs_has_id(mWorld, mHandle, EcsPrefab);
        }

        /// \brief Checks if this entity represents a component type.
        ///
        /// \return `true` if the entity is a component, `false` otherwise.
        ZY_INLINE Bool IsComponent() const
        {
            return ecs_has_id(mWorld, mHandle, ecs_id(EcsComponent));
        }

        /// \brief Checks if this entity represents a tag.
        ///
        /// \return `true` if the entity is a tag, `false` otherwise.
        ZY_INLINE Bool IsTag() const
        {
            return ecs_id_is_tag(mWorld, mHandle);
        }

        /// \brief Checks if this entity represents a relation pair.
        ///
        /// \return `true` if the entity is a pair, `false` otherwise.
        ZY_INLINE Bool IsPair() const
        {
            return ecs_id_is_pair(mHandle);
        }

        /// \brief Checks if this entity represents an overridable component.
        ///
        /// \return `true` if the entity is overridable, `false` otherwise.
        ZY_INLINE Bool IsOverridable() const
        {
            return !ecs_has_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsInherit))
                && !ecs_has_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsDontInherit));
        }

        /// \brief Destroys this entity and all of its components.
        ///
        /// \note The entity becomes invalid after destruction.
        ZY_INLINE void Destruct() const
        {
            ZY_ASSERT(IsValid(), "Attempted to destroy an invalid entity");

            ecs_delete(mWorld, mHandle);
        }

        /// \brief Enables this entity, allowing it to be processed by systems that require it to be awake.
        ///
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Awake() const
        {
            ecs_enable(mWorld, mHandle, true);
            return (* this);
        }

        /// \brief Disables this entity, preventing it from being processed by systems that require it to be awake.
        ///
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Sleep() const
        {
            ecs_enable(mWorld, mHandle, false);
            return (* this);
        }

        /// \brief Checks if this entity is currently awake (enabled).
        ///
        /// \return `true` if the entity is awake, `false` otherwise.
        ZY_INLINE Bool IsAwake() const
        {
            return !ecs_has_id(mWorld, mHandle, EcsDisabled);
        }

        /// \brief Attaches a component or tag to this entity.
        ///
        /// \tparam Component The component or tag type to attach.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Add() const
        {
            ecs_add_id(mWorld, mHandle, _::Identify<Component>());
            return (* this);
        }

        /// \brief Attaches a component or tag to this entity using a runtime entity.
        ///
        /// \param Component The component or tag entity to attach.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Add(Entity Component) const
        {
            ecs_add_id(mWorld, mHandle, Component.GetID());
            return (* this);
        }

        /// \brief Attaches a relation pair to this entity using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Add() const
        {
            ecs_add_id(mWorld, mHandle, _::Identify<Relation, Component>());
            return (* this);
        }

        /// \brief Attaches a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE Entity Add(Entity Component) const
        {
            ecs_add_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
            return (* this);
        }

        /// \brief Attaches a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity of the relation.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Add(Entity Relation, Entity Component) const
        {
            ecs_add_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
            return (* this);
        }

        /// \brief Sets the value of a component on this entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Set(AnyRef<Component> Data) const
        {
            Assign<StripAll<Component>>(_::Identify<Component>(), Data);
            return (* this);
        }

        /// \brief Sets the value of a component on a relation pair using a compile-time relation.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Set(AnyRef<Component> Data) const
        {
            Assign<StripAll<Component>>(_::Identify<Relation, Component>(), Data);
            return (* this);
        }

        /// \brief Sets the value of a component on a relation pair using a runtime relation entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Relation  The relation entity.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Set(Entity Relation, AnyRef<Component> Data) const
        {
            Assign<StripAll<Component>>(ecs_pair(Relation.GetID(), _::Identify<Component>()), Data);
            return (* this);
        }

        /// \brief Constructs a component directly on this entity, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZY_INLINE Entity Emplace(AnyRef<Arguments>... Parameters) const
            requires (!IsAnyOf<StripAll<Arguments>, Entity> && ...)
        {
            Construct<Component>(_::Identify<Component>(), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a component on this entity within a relation pair, forwarding arguments to its constructor.
        ///
        /// \tparam Relation   The relation type.
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component, typename... Arguments>
        ZY_INLINE Entity Emplace(AnyRef<Arguments>... Parameters) const
        {
            Construct<Component>(_::Identify<Relation, Component>(), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a component on this entity using a runtime relation, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Relation   The relation entity.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZY_INLINE Entity Emplace(Entity Relation, AnyRef<Arguments>... Parameters) const
        {
            Construct<Component>(
                ecs_pair(Relation.GetID(), _::Identify<Component>()), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Gets a writable pointer to a component, creating it if it does not exist.
        ///
        /// \tparam Component The component type to retrieve or create.
        /// \return A pointer to the component data.
        template<typename Component>
        ZY_INLINE Ptr<void> Ensure() const
        {
            return ecs_ensure_id(mWorld, mHandle, _::Identify<Component>(), sizeof(StripAll<Component>));
        }

        /// \brief Gets a writable pointer to a component by runtime entity, creating it if it does not exist.
        ///
        /// \param Component The component entity to retrieve or create.
        /// \return A pointer to the component data.
        ZY_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return ecs_ensure_id(mWorld, mHandle, Component.GetID(), Measure(Component.GetID()));
        }

        /// \brief Gets a writable pointer to a component on a relation pair using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return A pointer to the component data.
        template<typename Relation>
        ZY_INLINE Ptr<void> Ensure(Entity Component) const
        {
            const ecs_id_t Pair = _::Identify<Relation>(Component.GetID());
            return ecs_ensure_id(mWorld, mHandle, Pair, Measure(Pair));
        }

        /// \brief Gets a writable pointer to a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data.
        ZY_INLINE Ptr<void> Ensure(Entity Relation, Entity Component) const
        {
            const ecs_id_t Pair = ecs_pair(Relation.GetID(), Component.GetID());
            return ecs_ensure_id(mWorld, mHandle, Pair, Measure(Pair));
        }

        /// \brief Removes a component or tag from this entity.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Remove() const
        {
            ecs_remove_id(mWorld, mHandle, _::Identify<Component>());
            return (* this);
        }

        /// \brief Removes a component or tag from this entity using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Remove(Entity Component) const
        {
            ecs_remove_id(mWorld, mHandle, Component.GetID());
            return (* this);
        }

        /// \brief Removes a relation pair from this entity using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Remove() const
        {
            ecs_remove_id(mWorld, mHandle, _::Identify<Relation, Component>());
            return (* this);
        }

        /// \brief Removes a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE Entity Remove(Entity Component) const
        {
            ecs_remove_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
            return (* this);
        }

        /// \brief Removes a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to remove.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Remove(Entity Relation, Entity Component) const
        {
            ecs_remove_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
            return (* this);
        }

        /// \brief Checks if this entity has a given component or tag.
        ///
        /// \tparam Component The component or tag type to check.
        /// \return `true` if the entity has it, `false` otherwise.
        template<typename Component>
        ZY_INLINE Bool Has() const
        {
            return ecs_has_id(mWorld, mHandle, _::Identify<Component>());
        }

        /// \brief Checks if this entity has a given component or tag using a runtime entity.
        ///
        /// \param Component The component or tag entity to check.
        /// \return `true` if the entity has it, `false` otherwise.
        ZY_INLINE Bool Has(Entity Component) const
        {
            return ecs_has_id(mWorld, mHandle, Component.GetID());
        }

        /// \brief Checks if this entity has a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return `true` if the entity has the pair, `false` otherwise.
        template<typename Relation, typename Component>
        ZY_INLINE Bool Has() const
        {
            return ecs_has_id(mWorld, mHandle, _::Identify<Relation, Component>());
        }

        /// \brief Checks if this entity has a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to check.
        /// \return `true` if the entity has the pair, `false` otherwise.
        template<typename Relation>
        ZY_INLINE Bool Has(Entity Component) const
        {
            return ecs_has_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
        }

        /// \brief Checks if this entity has a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return `true` if the entity has the pair, `false` otherwise.
        ZY_INLINE Bool Has(Entity Relation, Entity Component) const
        {
            return ecs_has_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
        }

        /// \brief Checks if this entity owns a given component or tag directly, rather than inheriting it.
        ///
        /// \tparam Component The component or tag type to check.
        /// \return `true` if the entity owns it directly, `false` otherwise.
        template<typename Component>
        ZY_INLINE Bool Owns() const
        {
            return ecs_owns_id(mWorld, mHandle, _::Identify<Component>());
        }

        /// \brief Checks if this entity owns a given component or tag directly using a runtime entity.
        ///
        /// \param Component The component or tag entity to check.
        /// \return `true` if the entity owns it directly, `false` otherwise.
        ZY_INLINE Bool Owns(Entity Component) const
        {
            return ecs_owns_id(mWorld, mHandle, Component.GetID());
        }

        /// \brief Checks if this entity owns a relation pair directly using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return `true` if the entity owns the pair directly, `false` otherwise.
        template<typename Relation, typename Component>
        ZY_INLINE Bool Owns() const
        {
            return ecs_owns_id(mWorld, mHandle, _::Identify<Relation, Component>());
        }

        /// \brief Checks if this entity owns a relation pair directly using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to check.
        /// \return `true` if the entity owns the pair directly, `false` otherwise.
        template<typename Relation>
        ZY_INLINE Bool Owns(Entity Component) const
        {
            return ecs_owns_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
        }

        /// \brief Checks if this entity owns a relation pair directly using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return `true` if the entity owns the pair directly, `false` otherwise.
        ZY_INLINE Bool Owns(Entity Relation, Entity Component) const
        {
            return ecs_owns_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
        }

        /// \brief Gets a reference to a component on this entity.
        ///
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Component>
        ZY_INLINE Ref<Component> Get() const
        {
            return (* TryGet<Component>());
        }

        /// \brief Gets a reference to a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Relation, typename Component>
        ZY_INLINE Ref<Component> Get() const
        {
            return (* TryGet<Relation, Component>());
        }

        /// \brief Gets a pointer to a component, or null if the entity does not have it.
        ///
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Component>
        ZY_INLINE Ptr<Component> TryGet() const
        {
            return Fetch<Component>(_::Identify<Component>());
        }

        /// \brief Gets a raw pointer to a component by runtime entity, or null if not found.
        ///
        /// \param Component The component entity to look up.
        /// \return A pointer to the component data, or null if not found.
        ZY_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return ecs_get_mut_id(mWorld, mHandle, Component.GetID());
        }

        /// \brief Gets a pointer to a component on a relation pair, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation, typename Component>
        ZY_INLINE Ptr<Component> TryGet() const
        {
            return Fetch<Component>(_::Identify<Relation, Component>());
        }

        /// \brief Gets a raw pointer to a component on a relation pair using a runtime target, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation>
        ZY_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return ecs_get_mut_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
        }

        /// \brief Gets a raw pointer to a component on a relation pair using two runtime entities, or null if not found.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data, or null if not found.
        ZY_INLINE Ptr<void> TryGet(Entity Relation, Entity Component) const
        {
            return ecs_get_mut_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
        }

        /// \brief Notifies systems that a component on this entity has changed.
        ///
        /// \tparam Component The component type that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Notify() const
        {
            ecs_modified_id(mWorld, mHandle, _::Identify<Component>());
            return (* this);
        }

        /// \brief Notifies systems that a component has changed using a runtime entity.
        ///
        /// \param Component The component entity that was modified.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Notify(Entity Component) const
        {
            ecs_modified_id(mWorld, mHandle, Component.GetID());
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Notify() const
        {
            ecs_modified_id(mWorld, mHandle, _::Identify<Relation, Component>());
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE Entity Notify(Entity Component) const
        {
            ecs_modified_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()));
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity that was modified.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Notify(Entity Relation, Entity Component) const
        {
            ecs_modified_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
            return (* this);
        }

        /// \brief Enables a specific component on this entity, allowing systems to process it.
        ///
        /// \tparam Component The component type to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Enable() const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Component>(), true);
            return (* this);
        }

        /// \brief Enables a specific component on this entity using a runtime entity.
        ///
        /// \param Component The component entity to enable.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Enable(Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, Component.GetID(), true);
            return (* this);
        }

        /// \brief Enables a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Enable() const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Relation, Component>(), true);
            return (* this);
        }

        /// \brief Enables a component on a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE Entity Enable(Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()), true);
            return (* this);
        }

        /// \brief Enables a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to enable.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Enable(Entity Relation, Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()), true);
            return (* this);
        }

        /// \brief Disables a specific component on this entity, preventing systems from processing it.
        ///
        /// \tparam Component The component type to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZY_INLINE Entity Disable() const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Component>(), false);
            return (* this);
        }

        /// \brief Disables a specific component on this entity using a runtime entity.
        ///
        /// \param Component The component entity to disable.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Disable(Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, Component.GetID(), false);
            return (* this);
        }

        /// \brief Disables a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE Entity Disable() const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Relation, Component>(), false);
            return (* this);
        }

        /// \brief Disables a component on a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE Entity Disable(Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, _::Identify<Relation>(Component.GetID()), false);
            return (* this);
        }

        /// \brief Disables a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to disable.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Disable(Entity Relation, Entity Component) const
        {
            ecs_enable_id(mWorld, mHandle, ecs_pair(Relation.GetID(), Component.GetID()), false);
            return (* this);
        }

        /// \brief Sends an event to this entity, with an optional payload.
        ///
        /// If `Immediately` is `true`, the event is delivered right away. Otherwise, it is queued
        /// and processed at the end of the current frame.
        ///
        /// \tparam Event       The event type to dispatch.
        /// \param  Payload     The event data to send.
        /// \param  Immediately `true` to send immediately, `false` to queue.
        /// \return This entity, allowing for method chaining.
        template<typename Event>
        ZY_INLINE Entity Dispatch(ConstRef<Event> Payload, Bool Immediately = false) const
        {
            ecs_event_desc_t Description { };
            Description.event       = _::Identify<Event>();
            Description.entity      = mHandle;
            Description.const_param = AddressOf(Payload);
            Description.observable  = const_cast<Ptr<ecs_world_t>>(ecs_get_world(mWorld));

            if (Immediately)
            {
                ecs_emit(mWorld, AddressOf(Description));
            }
            else
            {
                ecs_enqueue(mWorld, AddressOf(Description));
            }
            return (* this);
        }

        /// \brief Subscribes to an event on this entity and invokes a callback when it fires.
        ///
        /// \tparam Event    The event type to listen for.
        /// \param  Callback The function to call when the event fires.
        /// \return This entity, allowing for method chaining.
        template<typename Event, typename Callable>
        ZY_INLINE Entity Subscribe(AnyRef<Callable> Callback) const
        {
            using Handler = Listener<StripAll<Callable>>;

            ecs_observer_desc_t Description { };
            Description.events[0]           = _::Identify<Event>();
            Description.query.terms->id     = EcsAny;
            Description.query.terms->src.id = mHandle;
            Description.callback            = Handler::OnInvoke;
            Description.callback_ctx        = new Handler(Forward<Callable>(Callback));
            Description.callback_ctx_free   = Handler::OnRelease;

            const ecs_entity_t Observer = ecs_observer_init(mWorld, AddressOf(Description));
            ecs_add_id(mWorld, Observer, ecs_pair(EcsChildOf, mHandle));

            return (* this);
        }

        /// \brief Looks up a child of this entity by its name.
        ///
        /// \param Name The name of the child to find.
        /// \return The child entity, or an invalid entity if not found.
        ZY_INLINE Entity Lookup(Text Name) const
        {
            ZY_ASSERT(mHandle, "Attempted to look up a child of an invalid entity");

            return Entity(mWorld, ecs_lookup_path_w_sep(mWorld, mHandle, Name.GetData(), "::", "::", false));
        }

        /// \brief Iterates over all child entities and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each child.
        template<typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            Children(EcsChildOf, Forward<Callable>(Callback));
        }

        /// \brief Iterates over all children related via a specific relation and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to filter children by.
        /// \param  Callback The function to call for each matching child.
        template<typename Relation, typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            Children(_::Identify<Relation>(), Forward<Callable>(Callback));
        }

        /// \brief Iterates over all components and tags on this entity and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each component or tag.
        template<typename Callable>
        ZY_INLINE void Each(AnyRef<Callable> Callback) const
        {
            if (const ConstPtr<ecs_type_t> Type = ecs_get_type(mWorld, mHandle))
            {
                for (SInt32 Element = 0; Element < Type->count; ++Element)
                {
                    Callback(Entity(mWorld, Type->array[Element]));
                }
            }

            // A component that does not fragment lives outside the entity's table, so it needs its own sweep.
            EachSparse(0, Forward<Callable>(Callback));
        }

        /// \brief Iterates over all targets of a specific relation on this entity and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to iterate targets for.
        /// \param  Callback The function to call for each target.
        template<typename Relation, typename Callable>
        ZY_INLINE void Each(AnyRef<Callable> Callback) const
        {
            const ecs_id_t Pattern = ecs_pair(_::Identify<Relation>(), EcsWildcard);

            const auto OnMatch = [&](Entity Pair)
            {
                Callback(Pair.GetComponent());
            };

            if (const Ptr<ecs_table_t> Table = ecs_get_table(mWorld, mHandle))
            {
                const ConstPtr<ecs_type_t> Type = ecs_table_get_type(Table);

                for (SInt32 Cursor = 0; (Cursor = ecs_search_offset(ecs_get_world(mWorld), Table, Cursor, Pattern, nullptr)) != -1; ++Cursor)
                {
                    OnMatch(Entity(mWorld, Type->array[Cursor]));
                }
            }
            EachSparse(Pattern, OnMatch);
        }

        /// \brief Attaches this entity to a parent, making it a child in the hierarchy.
        ///
        /// \param Parent    The entity to become the parent.
        /// \param Hierarchy The hierarchy type of the parent-child relationship.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Attach(Entity Parent, Hierarchy Hierarchy) const
        {
            switch (Hierarchy)
            {
            case Hierarchy::Open:
                ecs_add_id(mWorld, mHandle, ecs_pair(EcsChildOf, Parent.GetID()));
                break;
            case Hierarchy::Fixed:
            {
                const EcsParent Value(Parent.GetID());

                Assign<EcsParent>(ecs_id(EcsParent), Value);
            }
            break;
            }
            return (*this);
        }

        /// \brief Detaches this entity from its parent, promoting it to a root in the hierarchy.
        ///
        /// Clears both open (\ref Hierarchy::Open) and fixed (\ref Hierarchy::Fixed) parent relationships.
        ///
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity Detach() const
        {
            ecs_remove_id(mWorld, mHandle, ecs_pair(EcsChildOf, EcsWildcard));
            ecs_remove_id(mWorld, mHandle, ecs_id(EcsParent));
            return (* this);
        }

        /// \brief Gets the parent entity of this entity in the hierarchy.
        ///
        /// \return The parent entity, or an invalid entity if there is none.
        ZY_INLINE Entity GetParent() const
        {
            return Entity(mWorld, ecs_get_parent(mWorld, mHandle));
        }

        /// \brief Gets the immediate parent of this entity within a specific hierarchy type.
        ///
        /// Unlike \ref GetParent(), which returns whichever parent exists, this follows only the requested
        /// relationship and ignores the other.
        ///
        /// \param Hierarchy The hierarchy type whose parent to retrieve.
        /// \return The parent entity for that hierarchy, or an invalid entity if there is none.
        ZY_INLINE Entity GetParent(Hierarchy Hierarchy) const
        {
            switch (Hierarchy)
            {
            case Hierarchy::Open:
                return Entity(mWorld, ecs_get_target(mWorld, mHandle, EcsChildOf, 0));
            case Hierarchy::Fixed:
                if (const ConstPtr<EcsParent> Parent = Fetch<const EcsParent>(ecs_id(EcsParent)))
                {
                    return Entity(mWorld, Parent->value);
                }
                break;
            }
            return Entity();
        }

        /// \brief Assigns an archetype to this entity, inheriting its components and default values.
        ///
        /// \param Archetype The archetype entity to inherit from.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity SetArchetype(Entity Archetype) const
        {
            ecs_add_id(mWorld, mHandle, ecs_pair(EcsIsA, Archetype.GetID()));
            return (* this);
        }

        /// \brief Gets the archetype this entity inherits from, if any.
        ///
        /// \return The archetype entity, or an invalid entity if there is none.
        ZY_INLINE Entity GetArchetype() const
        {
            return Entity(mWorld, ecs_get_target(mWorld, mHandle, EcsIsA, 0));
        }

        /// \brief Sets the internal name of this entity, used for lookups and identification.
        ///
        /// \param Name The name to assign.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity SetName(Text Name) const
        {
            ecs_set_name(mWorld, mHandle, Name.GetData());
            return (* this);
        }

        /// \brief Gets the internal name of this entity.
        ///
        /// \return The entity's name, or an empty string if it has none.
        ZY_INLINE Text GetName() const
        {
            return Describe(ecs_get_name(mWorld, mHandle));
        }

        /// \brief Sets a human-readable display name (alias) for this entity, separate from its internal name.
        ///
        /// \param Name The display name to assign.
        /// \return This entity, allowing for method chaining.
        ZY_INLINE Entity SetAlias(Text Name) const
        {
            ecs_doc_set_name(mWorld, mHandle, Name.GetData());
            return (* this);
        }

        /// \brief Gets the human-readable display name (alias) of this entity.
        ///
        /// \return The alias string, or an empty string if none was set.
        ZY_INLINE Text GetAlias() const
        {
            return Describe(ecs_doc_get_name(mWorld, mHandle));
        }

        /// \brief Gets the relation side of a pair entity.
        ///
        /// \return The entity representing the relation (first element of the pair).
        ZY_INLINE Entity GetRelation() const
        {
            ZY_ASSERT(IsPair(), "Attempted to read the relation of an entity that is not a pair");

            return Resolve(ECS_PAIR_FIRST(mHandle));
        }

        /// \brief Gets the target side of a pair entity.
        ///
        /// \return The entity representing the component or target (second element of the pair).
        ZY_INLINE Entity GetComponent() const
        {
            ZY_ASSERT(IsPair(), "Attempted to read the target of an entity that is not a pair");

            return Resolve(ECS_PAIR_SECOND(mHandle));
        }

        /// \brief Loads this entity's components from a binary data stream.
        ///
        /// \param Archive The binary data reader to read the component data from.
        void Load(Ref<Reader> Archive) const;

        /// \brief Saves this entity's components to a binary data stream.
        ///
        /// \param Archive The binary data writer to write the component data to.
        void Save(Ref<Writer> Archive) const;

        /// \brief Copies this entity's components into a destination entity.
        ///
        /// \param Destination The entity to copy data into. If invalid, a new entity is created.
        /// \param Copy        `true` to copy component values, `false` to copy only component types.
        ZY_INLINE void Clone(Entity Destination = Entity(), Bool Copy = true) const
        {
            const Handle Target = Destination.GetHandle() ? Destination.GetHandle() : ecs_new(mWorld);

            ecs_clone(mWorld, Target, mHandle, Copy);
        }

        /// \brief Gets a hash value for this entity based on its unique identifier.
        ///
        /// \return The entity's unique identifier used as its hash.
        ZY_INLINE UInt64 Hash(UInt64) const
        {
            return GetID();
        }

        /// \brief Equals operator comparing two entities by their unique identifiers.
        ZY_INLINE Bool operator==(ConstRef<Entity> Other) const
        {
            return GetID() == Other.GetID();
        }

        /// \brief Inequality operator comparing two entities by their unique identifiers.
        ZY_INLINE Bool operator!=(ConstRef<Entity> Other) const = default;

    public:

        /// \brief Recursively attaches a tag to an entity and all of its descendants.
        ///
        /// \tparam Tag   The tag type to attach.
        /// \param  Actor The root entity to attach the tag to, along with its entire subtree.
        template<typename Tag>
        ZY_INLINE static void AddRecursively(Entity Actor)
        {
            const Ptr<ecs_world_t> World = Actor.GetWorld();

            const Bool Deferred = !ecs_is_deferred(World);

            if (Deferred)
            {
                ecs_defer_begin(World);
            }

            AddRecursivelyDeferred<Tag>(Actor);

            if (Deferred)
            {
                ecs_defer_end(World);
            }
        }

        /// \brief Recursively removes a tag from an entity and all of its descendants.
        ///
        /// \tparam Tag   The tag type to remove.
        /// \param  Actor The root entity to remove the tag from, along with its entire subtree.
        template<typename Tag>
        ZY_INLINE static void RemoveRecursively(Entity Actor)
        {
            const Ptr<ecs_world_t> World = Actor.GetWorld();

            const Bool Deferred = !ecs_is_deferred(World);

            if (Deferred)
            {
                ecs_defer_begin(World);
            }

            RemoveRecursivelyDeferred<Tag>(Actor);

            if (Deferred)
            {
                ecs_defer_end(World);
            }
        }

        /// \brief Resolves the topmost ancestor of an entity within a specific hierarchy type.
        ///
        /// \param Actor     The entity to resolve from.
        /// \param Hierarchy The hierarchy type whose parent chain to follow.
        /// \return The topmost ancestor in that hierarchy, or the actor itself if it has no such parent.
        ZY_INLINE static Entity ResolveRecursively(Entity Actor, Hierarchy Hierarchy)
        {
            Entity Root = Actor;

            for (Entity Parent = Root.GetParent(Hierarchy); Parent.IsValid(); Parent = Root.GetParent(Hierarchy))
            {
                Root = Parent;
            }
            return Root;
        }

    private:

        /// \brief Gets the size a component was registered with.
        ///
        /// \param Component The component to measure.
        /// \return The size of the component in bytes, or zero when it carries none.
        ZY_INLINE UInt32 Measure(ecs_id_t Component) const
        {
            const ConstPtr<ecs_type_info_t> Info = ecs_get_type_info(mWorld, Component);
            return Info ? static_cast<UInt32>(Info->size) : 0;
        }

        /// \brief Holds the callback an observer created by \ref Subscribe owns.
        template<typename Callable>
        struct Listener final
        {
            Callable Callback;

            /// \brief Constructs a listener taking ownership of a callable.
            ///
            /// \param Callback The callable the observer invokes.
            ZY_INLINE explicit Listener(AnyRef<Callable> Callback)
                : Callback { Move(Callback) }
            {
            }

            /// \brief Invokes the callback of the listener the result was created for.
            ///
            /// \param Handle The result flecs hands to the observer.
            ZY_INLINE static void OnInvoke(Ptr<ecs_iter_t> Handle)
            {
                ConstRef<Listener> Self = (* static_cast<ConstPtr<Listener>>(Handle->callback_ctx));

                if constexpr (requires { Self.Callback(Entity()); })
                {
                    Self.Callback(Entity(Handle->world, ecs_field_src(Handle, 0)));
                }
                else
                {
                    Self.Callback();
                }
            }

            /// \brief Releases the listener once flecs is done with the observer that owns it.
            ///
            /// \param Context The listener to release.
            ZY_INLINE static void OnRelease(Ptr<void> Context)
            {
                delete static_cast<Ptr<Listener>>(Context);
            }
        };

        /// \brief Recursive worker for \ref AddRecursively, run inside an already-open defer scope.
        ///
        /// \param  Actor The root entity to attach the tag to, along with its entire subtree.
        template<typename Tag>
        ZY_INLINE static void AddRecursivelyDeferred(Entity Actor)
        {
            Actor.Add<Tag>();

            Actor.Children([](Entity Child)
            {
                AddRecursivelyDeferred<Tag>(Child);
            });
        }

        /// \brief Recursive worker for \ref RemoveRecursively, run inside an already-open defer scope.
        ///
        /// \param  Actor The root entity to attach the tag to, along with its entire subtree.
        template<typename Tag>
        ZY_INLINE static void RemoveRecursivelyDeferred(Entity Actor)
        {
            Actor.Remove<Tag>();

            Actor.Children([](Entity Child)
            {
                RemoveRecursivelyDeferred<Tag>(Child);
            });
        }

        /// \brief Resolves a half of a pair back to the live entity it names.
        ///
        /// \param Handle The identifier stored in the pair, which carries no generation of its own.
        /// \return The live entity the half refers to.
        ZY_INLINE Entity Resolve(Handle Handle) const
        {
            return mWorld ? Entity(mWorld, ecs_get_alive(mWorld, Handle)) : Entity(Handle);
        }

        /// \brief Wraps a string flecs owns, which is null whenever the entity carries no such name.
        ///
        /// \param Value The string to wrap, or null.
        /// \return A view over \p Value, or an empty view when there is nothing to name.
        ZY_INLINE static Text Describe(ConstPtr<Char> Value)
        {
            return Value ? StrConvert(Value) : Text();
        }

        /// \brief Gets a pointer to a component, honouring whether the type was spelled as read-only.
        ///
        /// \param Identifier The identifier of the component to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Component>
        ZY_INLINE Ptr<Component> Fetch(ecs_id_t Identifier) const
        {
            if constexpr (IsImmutable<Component>)
            {
                return static_cast<Ptr<Component>>(ecs_get_id(mWorld, mHandle, Identifier));
            }
            else
            {
                return static_cast<Ptr<Component>>(ecs_get_mut_id(mWorld, mHandle, Identifier));
            }
        }

        /// \brief Writes a value into a component's storage, notifying whatever watches it.
        ///
        /// \param Identifier The identifier of the component to write.
        /// \param Data       The value written into the component.
        template<typename Component>
        ZY_INLINE void Assign(ecs_id_t Identifier, ConstRef<Component> Data) const
        {
            ecs_set_id(mWorld, mHandle, Identifier, sizeof(Component), AddressOf(Data));
        }

        /// \brief Builds a component in place from its constructor arguments and marks it as modified.
        ///
        /// \param Identifier The identifier of the component to build.
        /// \param Parameters Arguments forwarded to the component constructor.
        template<typename Component, typename... Arguments>
        ZY_INLINE void Construct(ecs_id_t Identifier, AnyRef<Arguments>... Parameters) const
        {
            Bool            Created = false;
            const Ptr<void> Memory  = ecs_emplace_id(mWorld, mHandle, Identifier, sizeof(Component), AddressOf(Created));

            if (Created)
            {
                new (Memory) Component(Forward<Arguments>(Parameters)...);
            }
            ecs_modified_id(mWorld, mHandle, Identifier);
        }

        /// \brief Iterates over the children reached through one relation and invokes a callback for each one.
        ///
        /// \param Relation The relation whose children are visited.
        /// \param Callback The function to call for each child.
        template<typename Callable>
        ZY_INLINE void Children(Handle Relation, AnyRef<Callable> Callback) const
        {
            // A wildcard names every entity rather than one, so asking for its children would match the world.
            if (mHandle == EcsWildcard || mHandle == EcsAny)
            {
                return;
            }

            for (ecs_iter_t Iterator = ecs_children_w_rel(mWorld, Relation, mHandle); ecs_children_next(& Iterator);)
            {
                for (SInt32 Element = 0; Element < Iterator.count; ++Element)
                {
                    Callback(Entity(mWorld, Iterator.entities[Element]));
                }
            }
        }

        /// \brief Iterates over the components this entity holds outside its table and matches them to a pattern.
        ///
        /// \param Pattern  The identifier pattern a component has to match, or `0` to accept every one of them.
        /// \param Callback The function to call for each matching component.
        template<typename Callable>
        ZY_INLINE void EachSparse(ecs_id_t Pattern, AnyRef<Callable> Callback) const
        {
            const Ptr<ecs_world_t>       World  = const_cast<Ptr<ecs_world_t>>(ecs_get_world(mWorld));
            const ConstPtr<ecs_record_t> Record = ecs_record_find(World, mHandle);

            if (!Record || !(Record->row & EcsEntityHasDontFragment))
            {
                return;
            }

            for (Ptr<ecs_component_record_t> Cursor = flecs_component_dont_fragment_first(World);
                 Cursor; Cursor = flecs_component_dont_fragment_next(Cursor))
            {
                const ecs_id_t Identifier = flecs_component_get_id(Cursor);

                if (ecs_id_is_wildcard(Identifier) || (Pattern && !ecs_id_match(Identifier, Pattern)))
                {
                    continue;
                }

                if (const Ptr<ecs_sparse_t> Storage = flecs_component_get_sparse(Cursor);
                    Storage && flecs_sparse_has(Storage, mHandle))
                {
                    Callback(Entity(mWorld, Identifier));
                }
            }
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<ecs_world_t> mWorld;
        Handle           mHandle;
    };
}