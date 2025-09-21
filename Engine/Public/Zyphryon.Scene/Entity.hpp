// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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
#include <flecs.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents an entity within the ECS (Entity-Component System).
    ///
    /// An entity is a lightweight handle or identifier that serves as a container for components and tags.
    /// Entities are used to compose game objects, define relationships, and participate in system logic.
    class Entity
    {
    public:

        /// \brief Underlying handle type used to represent the entity internally.
        using Handle = flecs::entity;

    public:

        /// \brief Constructs an invalid entity.
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

        /// \brief Constructs an entity from an existing handle.
        ///
        /// \param Handle The handle of this entity.
        ZYPHRYON_INLINE Entity(flecs::entity_t Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Constructs an entity from an identifier.
        ///
        /// \param Id The identifier that defines this entity.
        ZYPHRYON_INLINE Entity(flecs::id Id)
            : mHandle { Id.world(), Id.raw_id() }
        {
        }

        /// \brief Returns the unique numeric identifier of this entity.
        ///
        /// \return 64-bit entity identifier.
        ZYPHRYON_INLINE UInt64 GetID() const
        {
            return mHandle.id();
        }

        /// \brief Returns the internal handle representing this entity.
        ///
        /// \return The entity handle.
        ZYPHRYON_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Checks if this entity is valid (exists in the world).
        ///
        /// \return `true` if the entity is valid, false if not.
        ZYPHRYON_INLINE Bool IsValid() const
        {
            return mHandle.is_valid();
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

        /// \brief Checks if this entity represents a pair (relation-target).
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

        /// \brief Adds a tag component to this entity.
        ///
        /// \tparam Tag The tag type to add.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag>
        ZYPHRYON_INLINE ConstRef<Entity> Add() const
        {
            mHandle.add<Tag>();
            return (* this);
        }

        /// \brief Adds a component identified by an entity.
        ///
        /// \param Component The entity that represents the component.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Add(Entity Component) const
        {
            mHandle.add(Component.GetID());
            return (* this);
        }

        /// \brief Adds a pair consisting of a tag type and a target entity.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag>
        ZYPHRYON_INLINE ConstRef<Entity> Add(Entity Target) const
        {
            mHandle.add<Tag>(Target.GetID());
            return (* this);
        }

        /// \brief Adds a pair consisting of two component types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE ConstRef<Entity> Add() const
        {
            mHandle.add<Tag, Target>();
            return (* this);
        }

        /// \brief Adds a pair consisting of two entities.
        ///
        /// \param Tag       The entity used as the pair's first element.
        /// \param Component The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Add(Entity Tag, Entity Component)
        {
            mHandle.add(Tag.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Sets or replaces the data of a component on this entity.
        ///
        /// \param Data The component's data.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Set(AnyRef<Component> Data) const
        {
            mHandle.set<Component>(Move(Data));
            return (* this);
        }

        /// \brief Sets or replaces the data of a pair on this entity.
        ///
        /// \tparam Tag The tag type used as the pair's first element.
        /// \param Data The component data for the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Set(AnyRef<Component> Data) const
        {
            mHandle.set_second<Tag>(Move(Data));
            return (* this);
        }

        /// \brief Sets or replaces the data of a pair on this entity.
        ///
        /// \param Tag  The entity used as the pair's first element.
        /// \param Data The component data for the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Set(Entity Tag, AnyRef<Component> Data) const
        {
            mHandle.set_second(Tag.GetID(), Move(Data));
            return (* this);
        }

        /// \brief Constructs a component in-place on this entity.
        ///
        /// \param Parameters The component's constructor parameters.
        ///
        /// \return A reference to the updated entity.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE ConstRef<Entity> Emplace(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a pair in-place on this entity, with a tag type and component type.
        ///
        /// \tparam Tag       The tag type used as the pair's first element.
        /// \tparam Component The component type used as the pair's second element.
        /// \param Parameters The constructor parameters for the component.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Component, typename... Arguments>
        ZYPHRYON_INLINE ConstRef<Entity> Emplace(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<Tag, Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a pair in-place on this entity, with a tag entity and component type.
        ///
        /// \param Tag        The entity used as the pair's first element.
        /// \param Parameters The constructor parameters for the component.
        ///
        /// \return A reference to the updated entity.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE ConstRef<Entity> Emplace(Entity Tag, AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace_second<Component>(Tag.GetID(), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Ensures a component exists on this entity.
        ///
        /// \param Component The component entity to ensure.
        ///
        /// \return Pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return mHandle.ensure(Component.GetID());
        }

        /// \brief Ensures a pair exists on this entity, with a tag type and target entity.
        ///
        /// \tparam Tag  The tag type used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return Pointer to the component data.
        template<typename Tag>
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Target) const
        {
            return mHandle.ensure<Tag>(Target.GetID());
        }

        /// \brief Ensures a pair exists on this entity, with two entities.
        ///
        /// \param Tag       The entity used as the pair's first element.
        /// \param Component The entity used as the pair's second element.
        ///
        /// \return Pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Tag, Entity Component) const
        {
            return mHandle.ensure(Tag.GetID(), Component.GetID());
        }

        /// \brief Removes a component from this entity.
        ///
        /// \tparam Component The component type to remove.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Remove() const
        {
            mHandle.remove<Component>();
            return (* this);
        }

        /// \brief Removes a component or tag from this entity.
        ///
        /// \param Component The entity representing the component or tag.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Remove(Entity Component) const
        {
            mHandle.remove(Component.GetID());
            return (* this);
        }

        /// \brief Removes a pair from this entity, using two component types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE ConstRef<Entity> Remove() const
        {
            mHandle.remove<Tag, Target>();
            return (* this);
        }

        /// \brief Removes a pair from this entity, with a tag type and target entity.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag>
        ZYPHRYON_INLINE ConstRef<Entity> Remove(Entity Target) const
        {
            mHandle.remove<Tag>(Target.GetID());
            return (* this);
        }

        /// \brief Removes a pair from this entity, with two entities.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Remove(Entity Tag, Entity Target) const
        {
            mHandle.remove(Tag.GetID(), Target.GetID());
            return (* this);
        }

        /// \brief Checks whether this entity has a given component.
        ///
        /// \tparam Component The component type to check for.
        ///
        /// \return `true` if the entity has the component, `false` otherwise.
        template<typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Component>();
        }

        /// \brief Checks whether this entity has a given component or tag entity.
        ///
        /// \param Component The entity representing the component or tag.
        ///
        /// \return `true` if the entity has the component or tag, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Component) const
        {
            return mHandle.has(Component.GetID());
        }

        /// \brief Checks whether this entity has a specific pair of types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return `true` if the entity has the specified pair, `false` otherwise.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Tag, Target>();
        }

        /// \brief Checks whether this entity has a specific pair of entities.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return `true` if the entity has the specified pair, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Tag, Entity Target) const
        {
            return mHandle.has(Tag.GetID(), Target.GetID());
        }

        /// \brief Retrieves a pointer to a component on this entity if it exists.
        ///
        /// \tparam Component The component type to retrieve.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<typename Component>
        ZYPHRYON_INLINE Ptr<Component> Get() const
        {
            if constexpr (IsMutable<Component>)
            {
                return mHandle.try_get_mut<Component>();
            }
            else
            {
                return mHandle.try_get<Component>();
            }
        }

        /// \brief Retrieves a reference to a component on this entity.
        ///
        /// \tparam Component The component type to retrieve.
        ///
        /// \return A reference to the component data.
        template<typename Component>
        ZYPHRYON_INLINE Ref<Component> GetRef() const
        {
            if constexpr (IsMutable<Component>)
            {
                return mHandle.get_mut<Component>();
            }
            else
            {
                return mHandle.get<Component>();
            }
        }

        /// \brief Retrieves a pointer to a component represented by an entity.
        ///
        /// \param Component The entity representing the component.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        ZYPHRYON_INLINE Ptr<void> Get(Entity Component) const
        {
            return mHandle.try_get_mut(Component.GetID());
        }

        /// \brief Retrieves a pointer to a pair, where the first element is a tag type and the second is an entity.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<typename Tag>
        ZYPHRYON_INLINE Ptr<void> GetPair(Entity Target) const
        {
            return mHandle.try_get_mut<Tag>(Target.GetID());
        }

        /// \brief Retrieves a pointer to a pair, where both elements are entities.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        ZYPHRYON_INLINE Ptr<void> GetPair(Entity Tag, Entity Target) const
        {
            return mHandle.try_get_mut(Tag.GetID(), Target.GetID());
        }

        /// \brief Retrieves a pointer to a pair, where both elements are types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Ptr<Target> GetPair() const
        {
            if constexpr (IsMutable<Target>)
            {
                return mHandle.try_get_mut<Tag, Target>();
            }
            else
            {
                return mHandle.try_get<Tag, Target>();
            }
        }

        /// \brief Retrieves a reference to a pair, where both elements are types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the component data.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Ref<Target> GetPairRef() const
        {
            if constexpr (IsMutable<Target>)
            {
                return mHandle.get_mut<Tag, Target>();
            }
            else
            {
                return mHandle.get<Tag, Target>();
            }
        }

        /// \brief Marks a component as modified on this entity.
        ///
        /// \tparam Component The component type to notify.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Notify() const
        {
            mHandle.modified<Component>();
            return (* this);
        }

        /// \brief Marks a component as modified on this entity.
        ///
        /// \param Component The entity representing the component.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Notify(Entity Component) const
        {
            mHandle.modified(Component.GetID());
            return (* this);
        }

        /// \brief Marks a pair as modified on this entity, using two types.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE ConstRef<Entity> Notify() const
        {
            mHandle.modified<Tag, Target>();
            return (* this);
        }

        /// \brief Marks a pair as modified on this entity, using two entities.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Notify(Entity Tag, Entity Target) const
        {
            mHandle.modified(Tag.GetID(), Target.GetID());
            return (* this);
        }

        /// \brief Enables a component on this entity.
        ///
        /// \tparam Component The component type to enable.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Enable() const
        {
            mHandle.enable<Component>();
            return (* this);
        }

        /// \brief Enables a component or tag on this entity.
        ///
        /// \param Component The entity representing the component or tag.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Enable(Entity Component) const
        {
            mHandle.enable(Component.GetID());
            return (* this);
        }

        /// \brief Enables a pair of types on this entity.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE ConstRef<Entity> Enable() const
        {
            mHandle.enable<Tag, Target>();
            return (* this);
        }

        /// \brief Enables a pair of entities on this entity.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Enable(Entity Tag, Entity Target) const
        {
            mHandle.enable(Tag.GetID(), Target.GetID());
            return (* this);
        }

        /// \brief Disables a component on this entity.
        ///
        /// \tparam Component The component type to disable.
        ///
        /// \return A reference to the updated entity.
        template<typename Component>
        ZYPHRYON_INLINE ConstRef<Entity> Disable() const
        {
            mHandle.disable<Component>();
            return (* this);
        }

        /// \brief Disables a component or tag on this entity.
        ///
        /// \param Component The entity representing the component or tag.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Disable(Entity Component) const
        {
            mHandle.disable(Component.GetID());
            return (* this);
        }

        /// \brief Disables a pair of types on this entity.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE ConstRef<Entity> Disable() const
        {
            mHandle.disable<Tag, Target>();
            return (* this);
        }

        /// \brief Disables a pair of entities on this entity.
        ///
        /// \param Tag    The entity used as the pair's first element.
        /// \param Target The entity used as the pair's second element.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> Disable(Entity Tag, Entity Target) const
        {
            mHandle.disable(Tag.GetID(), Target.GetID());
            return (* this);
        }

        /// \brief Sets the parent of this entity to a type.
        ///
        /// \tparam Type The parent type.
        ///
        /// \return A reference to the updated entity.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Entity> SetParent() const
        {
            mHandle.child_of<Type>();
            return (* this);
        }

        /// \brief Sets the parent of this entity to another entity.
        ///
        /// \param Parent The parent entity.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> SetParent(Entity Parent) const
        {
            mHandle.child_of(Parent.GetHandle());
            return (* this);
        }

        /// \brief Retrieves the parent of this entity.
        ///
        /// \return The parent entity.
        ZYPHRYON_INLINE Entity GetParent() const
        {
            return Entity(mHandle.parent());
        }

        /// \brief Sets the archetype of this entity to a type.
        ///
        /// \tparam Type The archetype type.
        ///
        /// \return A reference to the updated entity.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Entity> SetArchetype() const
        {
            mHandle.is_a<Type>();
            return (* this);
        }

        /// \brief Sets the archetype of this entity to another entity.
        ///
        /// \param Archetype The archetype entity.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> SetArchetype(Entity Archetype) const
        {
            mHandle.is_a(Archetype.GetHandle());
            return (* this);
        }

        /// \brief Retrieves the archetype of this entity.
        ///
        /// \return The archetype entity.
        ZYPHRYON_INLINE Entity GetArchetype() const
        {
            return Entity(mHandle.target(flecs::IsA));
        }

        /// \brief Iterates over all children of this entity.
        ///
        /// \param Callback The function to call for each child entity.
        template<typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children(Callback);
        }

        /// \brief Iterates over all children of this entity that match a specific tag.
        ///
        /// \tparam Tag     The tag type to filter children by.
        /// \param Callback The function to call for each child entity.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children<Tag>(Callback);
        }

        /// \brief Iterates over all components of this entity.
        ///
        /// \param Callback The function to call for each component.
        template<typename Function>
        ZYPHRYON_INLINE void Iterate(AnyRef<Function> Callback) const
        {
            mHandle.each(Callback);
        }

        /// \brief Iterates over all components of this entity that match a specific tag.
        ///
        /// \tparam Tag     The tag type to filter components by.
        /// \param Callback The function to call for each component.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void Iterate(AnyRef<Function> Callback) const
        {
            mHandle.each<Tag>(Callback);
        }

        /// \brief Sets the internal name of this entity.
        ///
        /// \param Name The new entity name.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> SetName(ConstStr8 Name) const
        {
            mHandle.set_name(Str8(Name).c_str()); // TODO: Remove heap allocation (Flecs)
            return (* this);
        }

        /// \brief Retrieves the internal name of this entity.
        ///
        /// \return The entity's name as a string view.
        ZYPHRYON_INLINE ConstStr8 GetName() const
        {
            const flecs::string_view Name = mHandle.name();
            return ConstStr8(Name.c_str(), Name.size());
        }

        /// \brief Sets the display name of this entity.
        ///
        /// \param Name The new display name.
        ///
        /// \return A reference to the updated entity.
        ZYPHRYON_INLINE ConstRef<Entity> SetDisplayName(ConstStr8 Name) const
        {
            mHandle.set_doc_name(Str8(Name).c_str()); // TODO: Remove heap allocation (Flecs)
            return (* this);
        }

        /// \brief Retrieves the display name of this entity.
        ///
        /// \return The entity's display name.
        ZYPHRYON_INLINE ConstStr8 GetDisplayName() const
        {
            const ConstPtr<Char> Name = mHandle.doc_name();
            return ConstStr8(Name ? Name : "");
        }

        /// \brief Retrieves the first element of a pair associated with this entity.
        ///
        /// \return The first element of the pair as an entity.
        ZYPHRYON_INLINE Entity GetFirst() const
        {
            return Entity(mHandle.first());
        }

        /// \brief Retrieves the second element of a pair associated with this entity.
        ///
        /// \return The second element of the pair as an entity.
        ZYPHRYON_INLINE Entity GetSecond() const
        {
            return Entity(mHandle.second());
        }

        /// \brief Checks if this entity is equal to another entity.
        ///
        /// \param Other The entity to compare to.
        /// \return `true` if the entities are the same, `false` otherwise.
        ZYPHRYON_INLINE Bool operator==(ConstRef<Entity> Other) const
        {
            return GetHandle() == Other.GetHandle();
        }

        /// \brief Checks if this entity is not equal to another entity.
        ///
        /// \param Other The entity to compare to.
        /// \return `true` if the entities are not the same, `false` otherwise.
        ZYPHRYON_INLINE Bool operator!=(ConstRef<Entity> Other) const = default;

        /// \brief Computes a hash value for the object.
        ///
        /// \return A hash value uniquely representing the current state of the object.
        ZYPHRYON_INLINE UInt64 Hash() const
        {
            return GetID();
        }

    public:

        /// \brief Conditionally enables or disables a component on the specified entity.
        ///
        /// \tparam Component The type of component to toggle.
        /// \tparam Enable    If `true`, the component will be enabled. If `false`, the component will be disabled.
        /// \param Actor The target entity on which to toggle the component state.
        template<typename Component, Bool Enable>
        ZYPHRYON_INLINE static void ToggleComponent(Entity Actor)
        {
            if constexpr(Enable)
            {
                Actor.Enable<Component>();
            }
            else
            {
                Actor.Disable<Component>();
            }
        }

        /// \brief Recursively enables or disables a component on the specified entity and all its children.
        ///
        /// \tparam Component The type of component to toggle throughout the hierarchy.
        /// \tparam Enable    If `true`, the component will be enabled. If `false`, the component will be disabled.
        /// \param Actor The root entity of the hierarchy on which to start the operation.
        template<typename Component, Bool Enable>
        ZYPHRYON_INLINE static void ToggleComponentInHierarchy(Entity Actor)
        {
            if constexpr(Enable)
            {
                Actor.Enable<Component>();
            }
            else
            {
                Actor.Disable<Component>();
            }
            Actor.Children(&ToggleComponentInHierarchy<Component, Enable>);
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}
