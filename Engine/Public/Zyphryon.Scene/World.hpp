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
    /// \brief Represents the world or scene context within the ECS (Entity-Component System).
    class World
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

        /// \brief Adds a singleton tag component to this world.
        ///
        /// \tparam Tag The tag type to add.
        ///
        /// \return The updated world.
        template<typename Tag>
        ZYPHRYON_INLINE World Add() const
        {
            mHandle.add<Tag>();
            return (* this);
        }

        /// \brief Adds a value pair component to this world.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return The updated world.
        template<typename Tag>
        ZYPHRYON_INLINE World Add(Entity Target) const
        {
            mHandle.add<Tag>(Target.GetID());
            return (* this);
        }

        /// \brief Adds a relationship pair component to this world.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return The updated world.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE World Add() const
        {
            mHandle.add<Tag, Target>();
            return (* this);
        }

        /// \brief Adds a relationship pair component to this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return The updated world.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE World Add() const
        {
            return Add<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Adds a pair component to this world.
        ///
        /// \param Tag       The world used as the pair's first element.
        /// \param Component The world used as the pair's second element.
        ///
        /// \return The updated world.
        ZYPHRYON_INLINE World Add(Entity Tag, Entity Component) const
        {
            mHandle.add(Tag.GetID(), Component.GetID());
            return (* this);
        }

        /// \brief Sets or replaces the data of a component on this world.
        ///
        /// \param Data The component's data.
        ///
        /// \return The updated world.
        template<typename Component>
        ZYPHRYON_INLINE World Set(AnyRef<Component> Data) const
        {
            mHandle.set<Component>(Move(Data));
            return (* this);
        }

        /// \brief Constructs a component in-place on this world.
        ///
        /// \param Parameters The component's constructor parameters.
        ///
        /// \return The updated world.
        template<typename Component, typename... Arguments>
        ZYPHRYON_INLINE World Emplace(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a pair component in-place on this world.
        ///
        /// \tparam Tag       The tag type used as the pair's first element.
        /// \tparam Component The component type used as the pair's second element.
        /// \param Parameters The constructor parameters for the component.
        ///
        /// \return The updated world.
        template<typename Tag, typename Component, typename... Arguments>
        ZYPHRYON_INLINE World EmplacePair(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<Tag, Component>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Constructs a pair component in-place on this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return The updated world.
        template<IsPairDSL Pair, typename... Arguments>
        ZYPHRYON_INLINE World EmplacePair(AnyRef<Arguments>... Parameters) const
        {
            mHandle.emplace<typename Pair::First, typename Pair::Second>(Forward<Arguments>(Parameters)...);
            return (* this);
        }

        /// \brief Removes a component or tag from this world.
        ///
        /// \tparam Component The component type to remove.
        ///
        /// \return The updated world.
        template<typename Component>
        ZYPHRYON_INLINE World Remove() const
        {
            mHandle.remove<Component>();
            return (* this);
        }

        /// \brief Removes a pair component from this world.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return The updated world.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE World Remove() const
        {
            mHandle.remove<Tag, Target>();
            return (* this);
        }

        /// \brief Removes a pair component from this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return The updated world.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE World Remove() const
        {
            return Remove<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Removes a pair component from this world.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return The updated world.
        template<typename Tag>
        ZYPHRYON_INLINE World Remove(Entity Target) const
        {
            mHandle.remove<Tag>(Target.GetID());
            return (* this);
        }

        /// \brief Removes a pair component from this world.
        ///
        /// \param Tag    The world used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return The updated world.
        ZYPHRYON_INLINE World Remove(Entity Tag, Entity Target) const
        {
            mHandle.remove(Tag.GetID(), Target.GetID());
            return (* this);
        }

        /// \brief Checks whether this world has a specific component.
        ///
        /// \tparam Component The component type to check for.
        ///
        /// \return `true` if the world has the component, `false` otherwise.
        template<typename Component>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Component>();
        }

        /// \brief Checks whether this world has a specific pair component.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return `true` if the world has the specified pair, `false` otherwise.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Bool Has() const
        {
            return mHandle.has<Tag, Target>();
        }

        /// \brief Checks whether this world has a specific pair component.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return `true` if the world has the specified pair, `false` otherwise.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE Bool Has() const
        {
            return Has<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Checks whether this world has a specific pair component.
        ///
        /// \param Tag    The world used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return `true` if the world has the specified pair, `false` otherwise.
        ZYPHRYON_INLINE Bool Has(Entity Tag, Entity Target) const
        {
            return mHandle.has(Tag.GetID(), Target.GetID());
        }

        /// \brief Retrieves a pointer to a component on this world.
        ///
        /// \param Component The world representing the component.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        ZYPHRYON_INLINE Ptr<void> TryGet(Entity Component) const
        {
            return mHandle.try_get_mut(Component.GetID());
        }

        /// \brief Retrieves a pointer to a component on this world.
        ///
        /// \tparam Component The component type to retrieve.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
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

        /// \brief Retrieves a pointer to a pair component on this world.
        ///
        /// \tparam Tag   The tag type used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<typename Tag>
        ZYPHRYON_INLINE Ptr<void> TryGetPair(Entity Target) const
        {
            return mHandle.try_get_mut<Tag>(Target.GetID());
        }

        /// \brief Retrieves a pointer to a pair component on this world.
        ///
        /// \param Tag    The world used as the pair's first element.
        /// \param Target The world used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        ZYPHRYON_INLINE Ptr<void> TryGetPair(Entity Tag, Entity Target) const
        {
            return mHandle.try_get_mut(Tag.GetID(), Target.GetID());
        }

        /// \brief Retrieves a pointer to a pair component on this world.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Ptr<Target> TryGetPair() const
        {
            if constexpr (IsImmutable<Target>)
            {
                return mHandle.try_get<Tag, Target>();
            }
            else
            {
                return mHandle.try_get_mut<Tag, Target>();
            }
        }

        /// \brief Retrieves a pointer to a pair component on this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return A pointer to the component data, or nullptr if not present.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE decltype(auto) TryGetPair() const
        {
            return TryGetPair<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Retrieves a reference to a component on this world.
        ///
        /// \tparam Component The component type to retrieve.
        ///
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

        /// \brief Retrieves a reference to a pair component on this world.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return A reference to the component data.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE Ref<Target> GetPair() const
        {
            if constexpr (IsImmutable<Target>)
            {
                return mHandle.get<Tag, Target>();
            }
            else
            {
                return mHandle.get_mut<Tag, Target>();
            }
        }

        /// \brief Retrieves a reference to a pair component on this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return A reference to the component data.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE decltype(auto) GetPair() const
        {
            return GetPair<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Marks a component as modified on this world.
        ///
        /// \tparam Component The component type to notify.
        ///
        /// \return The updated world.
        template<typename Component>
        ZYPHRYON_INLINE World Notify() const
        {
            mHandle.modified<Component>();
            return (* this);
        }

        /// \brief Marks a pair component as modified on this world.
        ///
        /// \tparam Tag    The tag type used as the pair's first element.
        /// \tparam Target The component type used as the pair's second element.
        ///
        /// \return The updated world.
        template<typename Tag, typename Target>
        ZYPHRYON_INLINE World Notify() const
        {
            mHandle.modified<Tag, Target>();
            return (* this);
        }

        /// \brief Marks a pair component as modified on this world.
        ///
        /// \tparam Pair The pair type, which must satisfy the `IsPairDSL` concept.
        ///
        /// \return The updated world.
        template<IsPairDSL Pair>
        ZYPHRYON_INLINE World Notify() const
        {
            return Notify<typename Pair::First, typename Pair::Second>();
        }

        /// \brief Iterates over all children of this world.
        ///
        /// \param Callback The function to call for each child world.
        template<typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children(Callback);
        }

        /// \brief Iterates over all children of this world that match a specific tag.
        ///
        /// \tparam Tag     The tag type to filter children by.
        /// \param Callback The function to call for each child world.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void Children(AnyRef<Function> Callback) const
        {
            mHandle.children<Tag>(Callback);
        }

        /// \brief Iterates over all components of this world.
        ///
        /// \param Callback The function to call for each component.
        template<typename Function>
        ZYPHRYON_INLINE void Iterate(AnyRef<Function> Callback) const
        {
            mHandle.each(Callback);
        }

        /// \brief Iterates over all components of this world that match a specific tag.
        ///
        /// \tparam Tag     The tag type to filter components by.
        /// \param Callback The function to call for each component.
        template<typename Tag, typename Function>
        ZYPHRYON_INLINE void Iterate(AnyRef<Function> Callback) const
        {
            mHandle.each<Tag>(Callback);
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Handle> mHandle;
    };
}