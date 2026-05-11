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
#include "Factory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a typed component within the ECS (Entity-Component System).
    ///
    /// \tparam Type The C++ type that defines this component.
    template<typename Type>
    class Component final : public Entity
    {
    public:

        /// \brief Constructs an invalid component with no associated world or type.
        ZYPHRYON_INLINE Component() = default;

        /// \brief Constructs a component from an existing entity handle.
        ///
        /// \param Handle The handle of this component.
        ZYPHRYON_INLINE Component(Handle Handle)
            : Entity { Handle }
        {
        }

        /// \brief Constructs a component from an identifier.
        ///
        /// \param Id The identifier that defines this component.
        ZYPHRYON_INLINE Component(flecs::id Id)
            : Entity { Id }
        {
        }

        /// \brief Gets the size, in bytes, of the component type.
        ///
        /// \return The size of the component in bytes.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return Get<flecs::Component>().size;
        }

        /// \brief Gets the alignment requirement of the component type.
        ///
        /// \return The alignment of the component in bytes.
        ZYPHRYON_INLINE UInt32 GetAlignment() const
        {
            return Get<flecs::Component>().alignment;
        }

        /// \brief Grants one or more behavioral traits to this component.
        ///
        /// \param Traits The traits to grant.
        /// \return This component, allowing for method chaining.
        template<typename... Arguments>
        ZYPHRYON_INLINE Component Grant(Arguments... Traits) const
        {
            (ApplyTrait(Traits), ...);
            return (* this);
        }

        /// \brief Removes one or more behavioral traits from this component.
        ///
        /// \param Traits The traits to remove.
        /// \return This component, allowing for method chaining.
        template<typename... Arguments>
        ZYPHRYON_INLINE Component Revoke(Arguments... Traits) const
        {
            (EraseTrait(Traits), ...);
            return (* this);
        }

        /// \brief Establishes a "with" context between this component and another component type.
        ///
        /// When set, entities created while this component is in scope will also
        /// automatically include the specified component type.
        ///
        /// \tparam Target The component type to associate with this component.
        /// \return This component, allowing for method chaining.
        template<typename Target>
        ZYPHRYON_INLINE Component With() const
        {
            mHandle.add(EcsWith, mHandle.world().template component<Target>());
            return (* this);
        }

        /// \brief Establishes a "with" context between this component and another component entity.
        ///
        /// \param Target The component entity to associate with this component.
        /// \return This component, allowing for method chaining.
        ZYPHRYON_INLINE Component With(Component Target) const
        {
            mHandle.add(EcsWith, Target.GetHandle());
            return (* this);
        }

        /// \brief Establishes a dependency relationship between this component and another component type.
        ///
        /// \param Target The component entity that this component depends on.
        /// \return This component, allowing for method chaining.
        ZYPHRYON_INLINE Component DependsOn(Entity Target) const
        {
            mHandle.add(EcsDependsOn, Target.GetHandle());
            return (* this);
        }

        /// \brief Overrides the add behavior of this component with a custom function.
        ///
        /// \param Action The function to execute when this component is added to an entity.
        /// \return This component, allowing for method chaining.
        template<typename Function>
        ZYPHRYON_INLINE Component OnAdd(AnyRef<Function> Action) const
        {
            flecs::component<Type> Handle(mHandle.world(), mHandle.name(), true, mHandle.id());

            Handle.on_add(Forward<Function>(Action));

            return (* this);
        }

        /// \brief Overrides the set behavior of this component with a custom function.
        ///
        /// \param Action The function to execute when this component is set on an entity.
        /// \return This component, allowing for method chaining.
        template<typename Function>
        ZYPHRYON_INLINE Component OnSet(AnyRef<Function> Action) const
        {
            flecs::component<Type> Handle(mHandle.world(), mHandle.name(), true, mHandle.id());

            Handle.on_set(Forward<Function>(Action));

            return (* this);
        }

        /// \brief Overrides the remove behavior of this component with a custom function.
        ///
        /// \param Action The function to execute when this component is removed from an entity.
        /// \return This component, allowing for method chaining.
        template<typename Function>
        ZYPHRYON_INLINE Component OnRemove(AnyRef<Function> Action) const
        {
            flecs::component<Type> Handle(mHandle.world(), mHandle.name(), true, mHandle.id());

            Handle.on_remove(Forward<Function>(Action));

            return (* this);
        }

        /// \brief Overrides the replace behavior of this component with a custom function.
        ///
        /// \param Action The function to execute when this component is replaced on an entity.
        /// \return This component, allowing for method chaining.
        template<typename Function>
        ZYPHRYON_INLINE Component OnReplace(AnyRef<Function> Action) const
        {
            flecs::component<Type> Handle(mHandle.world(), mHandle.name(), true, mHandle.id());

            Handle.on_replace(Forward<Function>(Action));

            return (* this);
        }

    private:

        /// \brief Grants a specific trait to this component.
        ///
        /// \param Trait The trait to grant.
        ZYPHRYON_INLINE void ApplyTrait(Trait Trait) const
        {
            switch (Trait)
            {
            case Trait::Serializable:
                mHandle.set<Factory>(Factory::Create<Type>());
                break;
            case Trait::Inheritable:
                mHandle.add(flecs::OnInstantiate, flecs::Inherit);
                break;
            case Trait::Toggleable:
                mHandle.add(flecs::CanToggle);
                break;
            case Trait::Sparse:
                mHandle.add(flecs::DontFragment);
                break;
            case Trait::Associative:
                mHandle.add(flecs::PairIsTag);
                break;
            case Trait::Singleton:
                mHandle.add(flecs::Singleton);
                break;
            case Trait::Final:
                mHandle.add(flecs::Final);
                break;
            case Trait::Symmetric:
                mHandle.add(flecs::Symmetric);
                break;
            }
        }

        /// \brief Revokes a specific trait from this component.
        ///
        /// \param Trait The trait to revoke.
        ZYPHRYON_INLINE void EraseTrait(Trait Trait) const
        {
            switch (Trait)
            {
            case Trait::Serializable:
                mHandle.remove<Factory>();
                break;
            case Trait::Inheritable:
                mHandle.remove(flecs::OnInstantiate, flecs::Inherit);
                break;
            case Trait::Toggleable:
                mHandle.remove(flecs::CanToggle);
                break;
            case Trait::Sparse:
                mHandle.remove(flecs::Sparse);
                break;
            case Trait::Associative:
                mHandle.remove(flecs::PairIsTag);
                break;
            case Trait::Singleton:
                mHandle.remove(flecs::Singleton);
                break;
            case Trait::Final:
                mHandle.remove(flecs::Final);
                break;
            case Trait::Symmetric:
                mHandle.remove(flecs::Symmetric);
                break;
            }
        }
    };
}