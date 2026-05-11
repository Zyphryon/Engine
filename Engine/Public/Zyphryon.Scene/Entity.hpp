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
        using Handle = flecs::entity;

    public:

        /// \brief Constructs an invalid entity with no associated world or components.
        ZYPHRYON_INLINE Entity()
            : mHandle { Handle::null() }
        {
        }

        /// \brief Constructs an entity from an existing handle.
        ///
        /// \param Handle The handle of this entity.
        ZYPHRYON_INLINE Entity(Handle Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Constructs an entity from a raw numeric identifier.
        ///
        /// \param Handle The raw entity identifier.
        ZYPHRYON_INLINE Entity(flecs::entity_t Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Constructs an entity from an identifier object.
        ///
        /// \param Id The identifier object containing the world and raw ID for this entity.
        ZYPHRYON_INLINE Entity(flecs::id Id)
            : mHandle { Id.world(), Id.raw_id() }
        {
        }

        /// \brief Gets the unique numeric identifier of this entity.
        ///
        /// \return The entity's unique identifier.
        ZYPHRYON_INLINE UInt64 GetID() const
        {
            return mHandle.id();
        }

        /// \brief Gets the internal handle representing this entity.
        ///
        /// \return The entity internal handle.
        ZYPHRYON_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Checks if this entity is valid (exists in the world).
        ///
        /// \return `true` if the entity is valid, `false` otherwise.
        ZYPHRYON_INLINE Bool IsValid() const
        {
            return mHandle.is_valid();
        }

        /// \brief Checks if this entity is currently alive (not destroyed).
        ///
        /// \return `true` if the entity is alive, `false` otherwise.
        ZYPHRYON_INLINE Bool IsAlive() const
        {
            return mHandle.is_alive();
        }

        /// \brief Checks if this entity represents an archetype.
        ///
        /// \return `true` if the entity is an archetype, `false` otherwise.
        ZYPHRYON_INLINE Bool IsArchetype() const
        {
            return mHandle.has(flecs::Prefab);
        }

        /// \brief Checks if this entity represents a component type.
        ///
        /// \return `true` if the entity is a component, `false` otherwise.
        ZYPHRYON_INLINE Bool IsComponent() const
        {
            return mHandle.has<flecs::Component>();
        }

        /// \brief Checks if this entity represents a tag.
        ///
        /// \return `true` if the entity is a tag, `false` otherwise.
        ZYPHRYON_INLINE Bool IsTag() const
        {
            return ecs_id_is_tag(mHandle.world(), mHandle.id());
        }

        /// \brief Checks if this entity represents a relation pair.
        ///
        /// \return `true` if the entity is a pair, `false` otherwise.
        ZYPHRYON_INLINE Bool IsPair() const
        {
            return mHandle.is_pair();
        }

        /// \brief Destroys this entity and all of its components.
        ///
        /// \note The entity becomes invalid after destruction.
        ZYPHRYON_INLINE void Destruct() const
        {
            LOG_ASSERT(IsValid(), "Attempted to destroy an invalid entity");

            mHandle.destruct();
        }

        /// \brief Enables this entity, allowing it to be processed by systems that require it to be awake.
        ///
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Awake() const
        {
            mHandle.enable();
            return (* this);
        }

        /// \brief Disables this entity, preventing it from being processed by systems that require it to be awake.
        ///
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Sleep() const
        {
            mHandle.disable();
            return (* this);
        }

        /// \brief Checks if this entity is currently awake (enabled).
        ///
        /// \return `true` if the entity is awake, `false` otherwise.
        ZYPHRYON_INLINE Bool IsAwake() const
        {
            return mHandle.enabled();
        }

        /// \brief Attaches a component or tag to this entity.
        ///
        /// \tparam Component The component or tag type to attach.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Add() const
        {
            mHandle.add<Component>();
            return (* this);
        }

        /// \brief Attaches a component or tag to this entity using a runtime entity.
        ///
        /// \param Component The component or tag entity to attach.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Add(Entity Component) const
        {
            mHandle.add(Component.GetID());
            return (* this);
        }

        /// \brief Attaches a relation pair to this entity using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Add() const
        {
            mHandle.add<Relation, Component>();
            return (* this);
        }

        /// \brief Attaches a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE Entity Add(Entity Component) const
        {
            mHandle.add<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Attaches a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity of the relation.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Add(Entity Relation, Entity Component) const
        {
            mHandle.add(Relation.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Sets the value of a component on this entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Set(AnyRef<Component> Data) const
        {
            mHandle.set<Component>(Move(Data));
            return (* this);
        }

        /// \brief Sets the value of a component on a relation pair using a compile-time relation.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Set(AnyRef<Component> Data) const
        {
            mHandle.set_second<Relation>(Move(Data));
            return (* this);
        }

        /// \brief Sets the value of a component on a relation pair using a runtime relation entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Relation  The relation entity.
        /// \param  Data      The data to assign to the component.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Set(Entity Relation, AnyRef<Component> Data) const
        {
            mHandle.set_second(Relation.GetID(), Move(Data));
            return (* this);
        }

        /// \brief Constructs a component directly on this entity, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE Entity Emplace(AnyRef<Arguments>... Parameters) const
            requires (!IsEqual<Decay<Arguments>, Entity> && ...)
        {
            mHandle.emplace<Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a component on this entity within a relation pair, forwarding arguments to its constructor.
        ///
        /// \tparam Relation   The relation type.
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component, typename... Arguments>
        ZYPHRYON_INLINE Entity Emplace(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<Relation, Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a component on this entity using a runtime relation, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Relation   The relation entity.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This entity, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE Entity Emplace(Entity Relation, AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace_second<Component>(Relation.GetID(), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Gets a writable pointer to a component, creating it if it does not exist.
        ///
        /// \tparam Component The component type to retrieve or create.
        /// \return A pointer to the component data.
        template<typename Component>
        ZYPHRYON_INLINE Ptr<void> Ensure() const
        {
            return mHandle.ensure<Component>();
        }

        /// \brief Gets a writable pointer to a component by runtime entity, creating it if it does not exist.
        ///
        /// \param Component The component entity to retrieve or create.
        /// \return A pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return mHandle.ensure(Component.GetID());
        }

        /// \brief Gets a writable pointer to a component on a relation pair using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return A pointer to the component data.
        template<typename Relation>
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return mHandle.ensure<Relation>(Component.GetID());
        }

        /// \brief Gets a writable pointer to a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Relation, Entity Component) const
        {
            return mHandle.ensure(Relation.GetID(), Component.GetID());
        }

        /// \brief Removes a component or tag from this entity.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Remove() const
        {
            mHandle.remove<Component>();
            return (* this);
        }

        /// \brief Removes a component or tag from this entity using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Remove(Entity Component) const
        {
            mHandle.remove(Component.GetID());
            return (* this);
        }

        /// \brief Removes a relation pair from this entity using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Remove() const
        {
            mHandle.remove<Relation, Component>();
            return (* this);
        }

        /// \brief Removes a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE Entity Remove(Entity Component) const
        {
            mHandle.remove<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Removes a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to remove.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Remove(Entity Relation, Entity Component) const
        {
            mHandle.remove(Relation.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Checks if this entity has a given component or tag.
        ///
        /// \tparam Component The component or tag type to check.
        /// \return `true` if the entity has it, `false` otherwise.
        template<typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Component>();
        }

        /// \brief Checks if this entity has a given component or tag using a runtime entity.
        ///
        /// \param Component The component or tag entity to check.
        /// \return `true` if the entity has it, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Component) const
        {
            return mHandle.has(Component.GetID());
        }

        /// \brief Checks if this entity has a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return `true` if the entity has the pair, `false` otherwise.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Relation, Component>();
        }

        /// \brief Checks if this entity has a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to check.
        /// \return `true` if the entity has the pair, `false` otherwise.
        template<typename Relation>
        ZYPHRYON_INLINE Bool Has(Entity Component) const
        {
            return mHandle.has<Relation>(Component.GetID());
        }

        /// \brief Checks if this entity has a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return `true` if the entity has the pair, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Relation, Entity Component) const
        {
            return mHandle.has(Relation.GetID(), Component.GetID());
        }

        /// \brief Gets a reference to a component on this entity.
        ///
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Component>
        ZYPHRYON_INLINE Ref<Component> Get() const
        {
            if constexpr (IsImmutable<Component>)
            {
                return mHandle.get<Component>();
            }
            else
            {
                return mHandle.get_mut<Component>();
            }
        }

        /// \brief Gets a reference to a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Ref<Component> Get() const
        {
            if constexpr (IsImmutable<Component>)
            {
                return mHandle.get<Relation, Component>();
            }
            else
            {
                return mHandle.get_mut<Relation, Component>();
            }
        }

        /// \brief Gets a pointer to a component, or null if the entity does not have it.
        ///
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Component>
        ZYPHRYON_INLINE Ptr<Component> TryGet() const
        {
            if constexpr (IsImmutable<Component>)
            {
                return mHandle.try_get<Component>();
            }
            else
            {
                return mHandle.try_get_mut<Component>();
            }
        }

        /// \brief Gets a raw pointer to a component by runtime entity, or null if not found.
        ///
        /// \param Component The component entity to look up.
        /// \return A pointer to the component data, or null if not found.
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return mHandle.try_get_mut(Component.GetID());
        }

        /// \brief Gets a pointer to a component on a relation pair, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Ptr<Component> TryGet() const
        {
            if constexpr (IsImmutable<Component>)
            {
                return mHandle.try_get<Relation, Component>();
            }
            else
            {
                return mHandle.try_get_mut<Relation, Component>();
            }
        }

        /// \brief Gets a raw pointer to a component on a relation pair using a runtime target, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation>
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return mHandle.try_get_mut<Relation>(Component.GetID());
        }

        /// \brief Gets a raw pointer to a component on a relation pair using two runtime entities, or null if not found.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data, or null if not found.
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Relation, Entity Component) const
        {
            return mHandle.try_get_mut(Relation.GetID(), Component.GetID());
        }

        /// \brief Notifies systems that a component on this entity has changed.
        ///
        /// \tparam Component The component type that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Notify() const
        {
            mHandle.modified<Component>();
            return (* this);
        }

        /// \brief Notifies systems that a component has changed using a runtime entity.
        ///
        /// \param Component The component entity that was modified.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Notify(Entity Component) const
        {
            mHandle.modified(Component.GetID());
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Notify() const
        {
            mHandle.modified<Relation, Component>();
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity that was modified.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE Entity Notify(Entity Component) const
        {
            mHandle.modified<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Notifies systems that a component on a relation pair has changed using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity that was modified.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Notify(Entity Relation, Entity Component) const
        {
            mHandle.modified(Relation.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Enables a specific component on this entity, allowing systems to process it.
        ///
        /// \tparam Component The component type to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Enable() const
        {
            mHandle.enable<Component>();
            return (* this);
        }

        /// \brief Enables a specific component on this entity using a runtime entity.
        ///
        /// \param Component The component entity to enable.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Enable(Entity Component) const
        {
            mHandle.enable(Component.GetID());
            return (* this);
        }

        /// \brief Enables a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Enable() const
        {
            mHandle.enable<Relation, Component>();
            return (* this);
        }

        /// \brief Enables a component on a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to enable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE Entity Enable(Entity Component) const
        {
            mHandle.enable<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Enables a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to enable.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Enable(Entity Relation, Entity Component) const
        {
            mHandle.enable(Relation.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Disables a specific component on this entity, preventing systems from processing it.
        ///
        /// \tparam Component The component type to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE Entity Disable() const
        {
            mHandle.disable<Component>();
            return (* this);
        }

        /// \brief Disables a specific component on this entity using a runtime entity.
        ///
        /// \param Component The component entity to disable.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Disable(Entity Component) const
        {
            mHandle.disable(Component.GetID());
            return (* this);
        }

        /// \brief Disables a component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Entity Disable() const
        {
            mHandle.disable<Relation, Component>();
            return (* this);
        }

        /// \brief Disables a component on a relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to disable.
        /// \return This entity, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE Entity Disable(Entity Component) const
        {
            mHandle.disable<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Disables a component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity to disable.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity Disable(Entity Relation, Entity Component) const
        {
            mHandle.disable(Relation.GetID(), Component.GetID());
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
        ZYPHRYON_INLINE Entity Dispatch(ConstRef<Event> Payload, Bool Immediately = false) const
        {
            if (Immediately)
            {
                mHandle.emit(Payload);
            }
            else
            {
                mHandle.enqueue(Payload);
            }
            return (* this);
        }

        /// \brief Subscribes to an event on this entity and invokes a callback when it fires.
        ///
        /// \tparam Event    The event type to listen for.
        /// \param  Callback The function to call when the event fires.
        /// \return This entity, allowing for method chaining.
        template<typename Event, typename Function>
        ZYPHRYON_INLINE Entity Subscribe(AnyRef<Function> Callback) const
        {
            mHandle.observe<Event>(Move(Callback));
            return (* this);
        }

        /// \brief Iterates over all child entities and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each child.
        template<typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children(Forward<Function>(Callback));
        }

        /// \brief Iterates over all children related via a specific relation and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to filter children by.
        /// \param  Callback The function to call for each matching child.
        template<typename Relation, typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children<Relation>(Forward<Function>(Callback));
        }

        /// \brief Iterates over all components and tags on this entity and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each component or tag.
        template<typename Function>
        ZYPHRYON_INLINE void Each(AnyRef<Function> Callback) const
        {
            mHandle.each(Forward<Function>(Callback));
        }

        /// \brief Iterates over all targets of a specific relation on this entity and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to iterate targets for.
        /// \param  Callback The function to call for each target.
        template<typename Relation, typename Function>
        ZYPHRYON_INLINE void Each(AnyRef<Function> Callback) const
        {
            mHandle.each<Relation>(Forward<Function>(Callback));
        }

        /// \brief Sets the parent of this entity, making it a child in the hierarchy.
        ///
        /// \param Parent The entity to become the parent.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity SetParent(Entity Parent) const
        {
            mHandle.child_of(Parent.GetHandle());
            return (* this);
        }

        /// \brief Gets the parent entity of this entity in the hierarchy.
        ///
        /// \return The parent entity, or an invalid entity if there is none.
        ZYPHRYON_INLINE Entity GetParent() const
        {
            return Entity(mHandle.parent());
        }

        /// \brief Assigns an archetype to this entity, inheriting its components and default values.
        ///
        /// \param Archetype The archetype entity to inherit from.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity SetArchetype(Entity Archetype) const
        {
            mHandle.is_a(Archetype.GetHandle());
            return (* this);
        }

        /// \brief Gets the archetype this entity inherits from, if any.
        ///
        /// \return The archetype entity, or an invalid entity if there is none.
        ZYPHRYON_INLINE Entity GetArchetype() const
        {
            return Entity(mHandle.target(flecs::IsA));
        }

        /// \brief Sets the internal name of this entity, used for lookups and identification.
        ///
        /// \param Name The name to assign.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity SetName(ConstStr8 Name) const
        {
            mHandle.set_name(Str8(Name).c_str()); // TODO: Remove heap allocation (Flecs)
            return (* this);
        }

        /// \brief Gets the internal name of this entity.
        ///
        /// \return The entity's name, or an empty string if it has none.
        ZYPHRYON_INLINE ConstStr8 GetName() const
        {
            const flecs::string_view Name = mHandle.name();
            return ConstStr8(Name.c_str(), Name.size());
        }

        /// \brief Sets a human-readable display name (alias) for this entity, separate from its internal name.
        ///
        /// \param Name The display name to assign.
        /// \return This entity, allowing for method chaining.
        ZYPHRYON_INLINE Entity SetAlias(ConstStr8 Name) const
        {
            mHandle.set_doc_name(Str8(Name).c_str()); // TODO: Remove heap allocation (Flecs)
            return (* this);
        }

        /// \brief Gets the human-readable display name (alias) of this entity.
        ///
        /// \return The alias string, or an empty string if none was set.
        ZYPHRYON_INLINE ConstStr8 GetAlias() const
        {
            const ConstPtr<Char> Name = mHandle.doc_name();
            return ConstStr8(Name ? Name : "");
        }

        /// \brief Gets the relation side of a pair entity.
        ///
        /// \return The entity representing the relation (first element of the pair).
        ZYPHRYON_INLINE Entity GetRelation() const
        {
            return Entity(mHandle.first());
        }

        /// \brief Gets the target side of a pair entity.
        ///
        /// \return The entity representing the component or target (second element of the pair).
        ZYPHRYON_INLINE Entity GetComponent() const
        {
            return Entity(mHandle.second());
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
        ZYPHRYON_INLINE void Clone(Entity Destination = Entity(), Bool Copy = true) const
        {
            mHandle.clone(Copy, Destination.GetHandle());
        }

        /// \brief Gets a hash value for this entity based on its unique identifier.
        ///
        /// \return The entity's unique identifier used as its hash.
        ZYPHRYON_INLINE UInt64 Hash(UInt64) const
        {
            return GetID();
        }

        /// \brief Equals operator comparing two entities by their unique identifiers.
        ZYPHRYON_INLINE Bool operator==(ConstRef<Entity> Other) const
        {
            return GetID() == Other.GetID();
        }

        /// \brief Inequality operator comparing two entities by their unique identifiers.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Entity> Other) const = default;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}

