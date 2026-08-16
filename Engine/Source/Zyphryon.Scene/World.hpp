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

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents the world entity within the ECS, acting as a bridge to singleton components.
    ///
    /// A singleton is stored on the entity that represents the component itself, which is what every
    /// operation here resolves to before delegating to the entity API.
    class World final
    {
    public:

        /// \brief Underlying handle type used to represent the world internally.
        using Handle = Ptr<ecs_world_t>;

    public:

        /// \brief Constructs a world from an existing handle.
        ///
        /// \param Handle The handle of this world.
        ZY_INLINE World(Handle Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Gets the internal handle representing this world.
        ///
        /// \return The world internal handle.
        ZY_INLINE Handle GetHandle() const
        {
            return mHandle;
        }

        /// \brief Attaches a singleton component or tag to the world.
        ///
        /// \tparam Component The component or tag type to attach.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Add() const
        {
            Singleton<Component>().template Add<Component>();
            return (* this);
        }

        /// \brief Attaches a component or tag to the world using a runtime entity.
        ///
        /// \param Component The component or tag entity to attach.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Add(Entity Component) const
        {
            Component.Add(Component);
            return (* this);
        }

        /// \brief Attaches a singleton relation pair to the world using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE World Add() const
        {
            Singleton<Relation>().template Add<Relation, Component>();
            return (* this);
        }

        /// \brief Attaches a singleton relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE World Add(Entity Component) const
        {
            Singleton<Relation>().template Add<Relation>(Component);
            return (* this);
        }

        /// \brief Attaches a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Add(Entity Relation, Entity Component) const
        {
            Relation.Add(Relation, Component);
            return (* this);
        }

        /// \brief Sets the value of a singleton component on the world.
        ///
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Set(AnyRef<Component> Data) const
        {
            Singleton<Component>().Set(Forward<Component>(Data));
            return (* this);
        }

        /// \brief Sets the value of a singleton component on a relation pair using a compile-time relation.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE World Set(AnyRef<Component> Data) const
        {
            Singleton<Relation>().template Set<Relation, Component>(Forward<Component>(Data));
            return (* this);
        }

        /// \brief Sets the value of a singleton component on a relation pair using a runtime relation entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Relation  The relation entity.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Set(Entity Relation, AnyRef<Component> Data) const
        {
            Singleton<Component>().template Set<Component>(Relation, Forward<Component>(Data));
            return (* this);
        }

        /// \brief Constructs a singleton component directly on the world, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZY_INLINE World Emplace(AnyRef<Arguments>... Parameters) const
            requires (!IsAnyOf<StripAll<Arguments>, Entity> && ...)
        {
            Singleton<Component>().template Emplace<Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a singleton component on a relation pair, forwarding arguments to its constructor.
        ///
        /// \tparam Relation   The relation type.
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component, typename... Arguments>
        ZY_INLINE World Emplace(AnyRef<Arguments>... Parameters) const
        {
            Singleton<Component>().template Emplace<Relation, Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a singleton component using a runtime relation, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Relation   The relation entity.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZY_INLINE World Emplace(Entity Relation, AnyRef<Arguments>... Parameters) const
        {
            Singleton<Component>().template Emplace<Component>(Relation, Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Gets a writable pointer to a singleton component, creating it if it does not exist.
        ///
        /// \tparam Component The component type to retrieve or create.
        /// \return A pointer to the component data.
        template<typename Component>
        ZY_INLINE Ptr<void> Ensure() const
        {
            return Singleton<Component>().template Ensure<Component>();
        }

        /// \brief Gets a writable pointer to a singleton component by runtime entity, creating it if it does not exist.
        ///
        /// \param Component The component entity to retrieve or create.
        /// \return A pointer to the component data.
        ZY_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return Component.Ensure(Component);
        }

        /// \brief Gets a writable pointer to a singleton component on a relation pair using a runtime relation.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return A pointer to the component data.
        template<typename Relation>
        ZY_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return Component.template Ensure<Relation>(Component);
        }

        /// \brief Gets a writable pointer to a singleton component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data.
        ZY_INLINE Ptr<void> Ensure(Entity Relation, Entity Component) const
        {
            return Component.Ensure(Relation, Component);
        }

        /// \brief Checks if the world has a singleton component or tag.
        ///
        /// \tparam Component The component or tag type to check.
        /// \return `true` if it exists, `false` otherwise.
        template<typename Component>
        ZY_INLINE Bool Has() const
        {
            return Singleton<Component>().template Has<Component>();
        }

        /// \brief Checks if the world has a given singleton component using a runtime entity.
        ///
        /// \param Component The component entity to check.
        /// \return `true` if it exists, `false` otherwise.
        ZY_INLINE Bool Has(Entity Component) const
        {
            return Component.Has(Component);
        }

        /// \brief Checks if the world has a singleton relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return `true` if the pair exists, `false` otherwise.
        template<typename Relation, typename Component>
        ZY_INLINE Bool Has() const
        {
            return Singleton<Relation>().template Has<Relation, Component>();
        }

        /// \brief Checks if the world has a singleton relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to check.
        /// \return `true` if the pair exists, `false` otherwise.
        template<typename Relation>
        ZY_INLINE Bool Has(Entity Component) const
        {
            return Singleton<Relation>().template Has<Relation>(Component);
        }

        /// \brief Checks if the world has a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return `true` if the pair exists, `false` otherwise.
        ZY_INLINE Bool Has(Entity Relation, Entity Component) const
        {
            return Relation.Has(Relation, Component);
        }

        /// \brief Removes a singleton component or tag from the world.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Remove() const
        {
            Singleton<Component>().template Remove<Component>();
            return (* this);
        }

        /// \brief Removes a singleton component or tag from the world using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Remove(Entity Component) const
        {
            Component.Remove(Component);
            return (* this);
        }

        /// \brief Removes a singleton relation pair from the world using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE World Remove() const
        {
            Singleton<Relation>().template Remove<Relation, Component>();
            return (* this);
        }

        /// \brief Removes a singleton relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE World Remove(Entity Component) const
        {
            Singleton<Relation>().template Remove<Relation>(Component);
            return (* this);
        }

        /// \brief Removes a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Remove(Entity Relation, Entity Component) const
        {
            Relation.Remove(Relation, Component);
            return (* this);
        }

        /// \brief Removes all occurrences of a component.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Purge() const
        {
            ecs_remove_all(mHandle, _::Identify<Component>());
            return (* this);
        }

        /// \brief Removes all occurrences of a component using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Purge(Entity Component) const
        {
            ecs_remove_all(mHandle, Component.GetID());
            return (* this);
        }

        /// \brief Removes all occurrences of a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE World Purge() const
        {
            ecs_remove_all(mHandle, (_::Identify<Relation, Component>()));
            return (* this);
        }

        /// \brief Removes all occurrences of a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE World Purge(Entity Component) const
        {
            ecs_remove_all(mHandle, _::Identify<Relation>(Component.GetID()));
            return (* this);
        }

        /// \brief Removes all occurrences of a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Purge(Entity Relation, Entity Component) const
        {
            ecs_remove_all(mHandle, ecs_pair(Relation.GetID(), Component.GetID()));
            return (* this);
        }

        /// \brief Gets a reference to a singleton component on the world.
        ///
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Component>
        ZY_INLINE Ref<Component> Get() const
        {
            return Singleton<Component>().template Get<Component>();
        }

        /// \brief Gets a reference to a singleton component on a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to retrieve.
        /// \return A reference to the component data.
        template<typename Relation, typename Component>
        ZY_INLINE Ref<Component> Get() const
        {
            return Singleton<Relation>().template Get<Relation, Component>();
        }

        /// \brief Gets a pointer to a singleton component, or null if it does not exist.
        ///
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Component>
        ZY_INLINE Ptr<Component> TryGet() const
        {
            return Singleton<Component>().template TryGet<Component>();
        }

        /// \brief Gets a raw pointer to a singleton component by runtime entity, or null if not found.
        ///
        /// \param Component The component entity to look up.
        /// \return A pointer to the component data, or null if not found.
        ZY_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return Component.TryGet(Component);
        }

        /// \brief Gets a pointer to a singleton component on a relation pair, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation, typename Component>
        ZY_INLINE Ptr<Component> TryGet() const
        {
            return Singleton<Relation>().template TryGet<Relation, Component>();
        }

        /// \brief Gets a raw pointer to a singleton component on a relation pair using a runtime target, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation>
        ZY_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return Singleton<Relation>().template TryGet<Relation>(Component);
        }

        /// \brief Gets a raw pointer to a singleton component on a relation pair using two runtime entities, or null if not found.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data, or null if not found.
        ZY_INLINE Ptr<void> TryGet(Entity Relation, Entity Component) const
        {
            return Relation.TryGet(Relation, Component);
        }

        /// \brief Notifies systems that a singleton component on the world has changed.
        ///
        /// \tparam Component The component type that was modified.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZY_INLINE World Notify() const
        {
            Singleton<Component>().template Notify<Component>();
            return (* this);
        }

        /// \brief Notifies systems that a singleton component has changed using a runtime entity.
        ///
        /// \param Component The component entity that was modified.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Notify(Entity Component) const
        {
            Component.Notify(Component);
            return (* this);
        }

        /// \brief Notifies systems that a singleton component on a relation pair has changed.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type that was modified.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZY_INLINE World Notify() const
        {
            Singleton<Relation>().template Notify<Relation, Component>();
            return (* this);
        }

        /// \brief Notifies systems that a singleton component on a relation pair has changed using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity that was modified.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZY_INLINE World Notify(Entity Component) const
        {
            Singleton<Relation>().template Notify<Relation>(Component);
            return (* this);
        }

        /// \brief Notifies systems that a singleton component on a relation pair has changed using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity that was modified.
        /// \return This world, allowing for method chaining.
        ZY_INLINE World Notify(Entity Relation, Entity Component) const
        {
            Component.Notify(Relation, Component);
            return (* this);
        }

        /// \brief Iterates over all child entities of the world and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each child entity.
        template<typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            Root().Children(Forward<Callable>(Callback));
        }

        /// \brief Iterates over all child entities related via a specific relation and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to filter children by.
        /// \param  Callback The function to call for each matching child entity.
        template<typename Relation, typename Callable>
        ZY_INLINE void Children(AnyRef<Callable> Callback) const
        {
            Root().template Children<Relation>(Forward<Callable>(Callback));
        }

        /// \brief Iterates over all singleton components and tags on the world and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each component or tag.
        template<typename Callable>
        ZY_INLINE void Each(AnyRef<Callable> Callback) const
        {
            for (ecs_iter_t Iterator = ecs_each_id(mHandle, EcsSingleton); ecs_each_next(& Iterator);)
            {
                for (SInt32 Element = 0; Element < Iterator.count; ++Element)
                {
                    // A component may declare itself a singleton without the world ever storing a value for it.
                    if (const Entity Component(mHandle, Iterator.entities[Element]); Has(Component))
                    {
                        Callback(Component);
                    }
                }
            }
        }

    private:

        /// \brief Gets the entity a singleton component of a given type is stored on.
        ///
        /// \return The entity that represents the component.
        template<typename Component>
        ZY_INLINE Entity Singleton() const
        {
            return Entity(mHandle, _::Identify<Component>());
        }

        /// \brief Gets the entity that stands for the root of the hierarchy.
        ///
        /// \return The root entity.
        ZY_INLINE Entity Root() const
        {
            return Entity(mHandle, 0);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}