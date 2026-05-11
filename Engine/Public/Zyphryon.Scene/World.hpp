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
    class World final
    {
    public:

        /// \brief Underlying handle type used to represent the world internally.
        using Handle = flecs::world;

    public:

        /// \brief Constructs a world from an existing handle.
        ///
        /// \param Handle The handle of this world.
        ZYPHRYON_INLINE World(Ref<Handle> Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Attaches a singleton component or tag to the world.
        ///
        /// \tparam Component The component or tag type to attach.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Add() const
        {
            mHandle.add<Component>();
            return (* this);
        }

        /// \brief Attaches a component or tag to the world using a runtime entity.
        ///
        /// \param Component The component or tag entity to attach.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Add(Entity Component) const
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
        ZYPHRYON_INLINE World Add() const
        {
            mHandle.add<Relation, Component>();
            return (* this);
        }

        /// \brief Attaches a singleton relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE World Add(Entity Component) const
        {
            mHandle.add<Relation>(Component.GetHandle());
            return (* this);
        }

        /// \brief Attaches a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Add(Entity Relation, Entity Component) const
        {
            mHandle.add(Relation.GetHandle(), Component.GetHandle());
            return (* this);
        }

        /// \brief Sets the value of a singleton component on the world.
        ///
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Set(AnyRef<Component> Data) const
        {
            mHandle.set<Component>(Move(Data));
            return (* this);
        }

        /// \brief Sets the value of a singleton component on a relation pair using a compile-time relation.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The component type to set.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE World Set(AnyRef<Component> Data) const
        {
            mHandle.set<Relation, Component>(Move(Data));
            return (* this);
        }

        /// \brief Sets the value of a singleton component on a relation pair using a runtime relation entity.
        ///
        /// \tparam Component The component type to set.
        /// \param  Relation  The relation entity.
        /// \param  Data      The data to assign.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Set(Entity Relation, AnyRef<Component> Data) const
        {
            mHandle.set<Component>(Relation.GetHandle(), Move(Data));
            return (* this);
        }

        /// \brief Constructs a singleton component directly on the world, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE World Emplace(AnyRef<Arguments>... Parameters) const
            requires (!IsEqual<Decay<Arguments>, Entity> && ...)
        {
            mHandle.emplace<Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a singleton component on a relation pair, forwarding arguments to its constructor.
        ///
        /// \tparam Relation   The relation type.
        /// \tparam Component  The component type to construct.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component, typename... Arguments>
        ZYPHRYON_INLINE World Emplace(AnyRef<Arguments>... Parameters) const
        {
            mHandle.component<Component>().template emplace<Relation, Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a singleton component using a runtime relation, forwarding arguments to its constructor.
        ///
        /// \tparam Component  The component type to construct.
        /// \param  Relation   The relation entity.
        /// \param  Parameters Arguments forwarded to the component constructor.
        /// \return This world, allowing for method chaining.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE World Emplace(Entity Relation, AnyRef<Arguments>... Parameters) const
        {
            mHandle.component<Component>().template emplace_second<Component>(Relation.GetHandle(), Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Gets a writable pointer to a singleton component, creating it if it does not exist.
        ///
        /// \tparam Component The component type to retrieve or create.
        /// \return A pointer to the component data.
        template<typename Component>
        ZYPHRYON_INLINE Ptr<void> Ensure() const
        {
            return mHandle.ensure<Component>();
        }

        /// \brief Gets a writable pointer to a singleton component by runtime entity, creating it if it does not exist.
        ///
        /// \param Component The component entity to retrieve or create.
        /// \return A pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return Component.Ensure(Component);
        }

        /// \brief Gets a writable pointer to a singleton component on a relation pair using a runtime relation.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity of the relation.
        /// \return A pointer to the component data.
        template<typename Relation>
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Component) const
        {
            return Component.Ensure<Relation>(Component);
        }

        /// \brief Gets a writable pointer to a singleton component on a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data.
        ZYPHRYON_INLINE Ptr<void> Ensure(Entity Relation, Entity Component) const
        {
            return Component.Ensure(Relation, Component);
        }

        /// \brief Checks if the world has a singleton component or tag.
        ///
        /// \tparam Component The component or tag type to check.
        /// \return `true` if it exists, `false` otherwise.
        template<typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Component>();
        }

        /// \brief Checks if the world has a given singleton component using a runtime entity.
        ///
        /// \param Component The component entity to check.
        /// \return `true` if it exists, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Component) const
        {
            return Component.Has(Component);
        }

        /// \brief Checks if the world has a singleton relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return `true` if the pair exists, `false` otherwise.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Relation, Component>();
        }

        /// \brief Checks if the world has a singleton relation pair using a compile-time relation and a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to check.
        /// \return `true` if the pair exists, `false` otherwise.
        template<typename Relation>
        ZYPHRYON_INLINE Bool Has(Entity Component) const
        {
            return mHandle.has<Relation>(Component.GetHandle());
        }

        /// \brief Checks if the world has a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return `true` if the pair exists, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Relation, Entity Component) const
        {
            return mHandle.has(Relation.GetHandle(), Component.GetHandle());
        }

        /// \brief Removes a singleton component or tag from the world.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Remove() const
        {
            mHandle.remove<Component>();
            return (* this);
        }

        /// \brief Removes a singleton component or tag from the world using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Remove(Entity Component) const
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
        ZYPHRYON_INLINE World Remove() const
        {
            mHandle.remove<Relation, Component>();
            return (* this);
        }

        /// \brief Removes a singleton relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE World Remove(Entity Component) const
        {
            mHandle.remove<Relation>(Component.GetHandle());
            return (* this);
        }

        /// \brief Removes a singleton relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Remove(Entity Relation, Entity Component) const
        {
            mHandle.remove(Relation.GetHandle(), Component.GetHandle());
            return (* this);
        }

        /// \brief Remove all occurrences instances of a component.
        ///
        /// \tparam Component The component or tag type to remove.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Purge() const
        {
            mHandle.remove_all<Component>();
            return (* this);
        }

        /// \brief Remove all occurrences of a component using a runtime entity.
        ///
        /// \param Component The component or tag entity to remove.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Purge(Entity Component) const
        {
            mHandle.remove_all(Component.GetID());
            return (* this);
        }

        /// \brief Remove all occurrences of a relation pair using two compile-time types.
        ///
        /// \tparam Relation  The relation type.
        /// \tparam Component The target type of the relation.
        /// \return This world, allowing for method chaining.
        template<typename Relation, typename Component>
        ZYPHRYON_INLINE World Purge() const
        {
            mHandle.remove_all<Relation, Component>();
            return (* this);
        }

        /// \brief Remove all occurrences of a relation pair using a compile-time relation and a runtime target entity.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to remove.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE World Purge(Entity Component) const
        {
            mHandle.remove_all<Relation>(Component.GetHandle());
            return (* this);
        }

        /// \brief Remove all occurrences of a relation pair using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Purge(Entity Relation, Entity Component) const
        {
            mHandle.remove_all(Relation.GetHandle(), Component.GetHandle());
            return (* this);
        }

        /// \brief Gets a reference to a singleton component on the world.
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

        /// \brief Gets a reference to a singleton component on a relation pair using two compile-time types.
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

        /// \brief Gets a pointer to a singleton component, or null if it does not exist.
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

        /// \brief Gets a raw pointer to a singleton component by runtime entity, or null if not found.
        ///
        /// \param Component The component entity to look up.
        /// \return A pointer to the component data, or null if not found.
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return mHandle.try_get_mut(Component.GetID());
        }

        /// \brief Gets a pointer to a singleton component on a relation pair, or null if not found.
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

        /// \brief Gets a raw pointer to a singleton component on a relation pair using a runtime target, or null if not found.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity to look up.
        /// \return A pointer to the component data, or null if not found.
        template<typename Relation>
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return mHandle.try_get_mut<Relation>(Component.GetID());
        }

        /// \brief Gets a raw pointer to a singleton component on a relation pair using two runtime entities, or null if not found.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity.
        /// \return A pointer to the component data, or null if not found.
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Relation, Entity Component) const
        {
            return mHandle.try_get_mut(Relation.GetID(), Component.GetID());
        }

        /// \brief Notifies systems that a singleton component on the world has changed.
        ///
        /// \tparam Component The component type that was modified.
        /// \return This world, allowing for method chaining.
        template<typename Component>
        ZYPHRYON_INLINE World Notify() const
        {
            mHandle.modified<Component>();
            return (* this);
        }

        /// \brief Notifies systems that a singleton component has changed using a runtime entity.
        ///
        /// \param Component The component entity that was modified.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Notify(Entity Component) const
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
        ZYPHRYON_INLINE World Notify() const
        {
            mHandle.modified<Relation, Component>();
            return (* this);
        }

        /// \brief Notifies systems that a singleton component on a relation pair has changed using a runtime target.
        ///
        /// \tparam Relation  The relation type.
        /// \param  Component The target entity that was modified.
        /// \return This world, allowing for method chaining.
        template<typename Relation>
        ZYPHRYON_INLINE World Notify(Entity Component) const
        {
            mHandle.modified<Relation>(Component.GetID());
            return (* this);
        }

        /// \brief Notifies systems that a singleton component on a relation pair has changed using two runtime entities.
        ///
        /// \param Relation  The relation entity.
        /// \param Component The target entity that was modified.
        /// \return This world, allowing for method chaining.
        ZYPHRYON_INLINE World Notify(Entity Relation, Entity Component) const
        {
            Component.Notify(Relation, Component);
            return (* this);
        }

        /// \brief Iterates over all child entities of the world and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each child entity.
        template<typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children(Forward<Function>(Callback));
        }

        /// \brief Iterates over all child entities related via a specific relation and invokes a callback for each one.
        ///
        /// \tparam Relation The relation type to filter children by.
        /// \param  Callback The function to call for each matching child entity.
        template<typename Relation, typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children<Relation>(Forward<Function>(Callback));
        }

        /// \brief Iterates over all singleton components and tags on the world and invokes a callback for each one.
        ///
        /// \param Callback The function to call for each component or tag.
        template<typename Function>
        ZYPHRYON_INLINE void Each(AnyRef<Function> Callback) const
        {
            const auto OnCallback = [&](Entity Component)
            {
                if (Has(Component))
                {
                    Callback(Component);
                }
            };
            mHandle.query_builder<>().with(flecs::Singleton).each(OnCallback);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Handle> mHandle;
    };
}
