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
        ZY_INLINE Component() = default;

        /// \brief Constructs a component from the entity that carries it.
        ///
        /// \param Actor The entity that represents this component.
        ZY_INLINE Component(Entity Actor)
            : Entity { Actor }
        {
        }

        /// \brief Constructs a component from an identifier bound to the world that issued it.
        ///
        /// \param World  The world the component belongs to.
        /// \param Handle The identifier that defines this component.
        ZY_INLINE Component(Ptr<ecs_world_t> World, Handle Handle)
            : Entity { World, Handle }
        {
        }

        /// \brief Gets the size, in bytes, of the component type.
        ///
        /// \return The size of the component in bytes.
        ZY_INLINE UInt32 GetSize() const
        {
            return static_cast<UInt32>(Describe().size);
        }

        /// \brief Gets the alignment requirement of the component type.
        ///
        /// \return The alignment of the component in bytes.
        ZY_INLINE UInt32 GetAlignment() const
        {
            return static_cast<UInt32>(Describe().alignment);
        }

        /// \brief Grants one or more behavioral traits to this component.
        ///
        /// \param Traits The traits to grant.
        /// \return This component, allowing for method chaining.
        template<typename... Arguments>
        ZY_INLINE Component Grant(Arguments... Traits) const
        {
            (ApplyTrait(Traits), ...);
            return (* this);
        }

        /// \brief Removes one or more behavioral traits from this component.
        ///
        /// \param Traits The traits to remove.
        /// \return This component, allowing for method chaining.
        template<typename... Arguments>
        ZY_INLINE Component Revoke(Arguments... Traits) const
        {
            (EraseTrait(Traits), ...);
            return (* this);
        }

        /// \brief Establishes a "with" context between this component and another component type.
        ///
        /// \tparam Target The component type to associate with this component.
        /// \return This component, allowing for method chaining.
        template<typename Target>
        ZY_INLINE Component With() const
        {
            ecs_add_id(mWorld, mHandle, ecs_pair(EcsWith, _::Identify<Target>()));
            return (* this);
        }

        /// \brief Establishes a "with" context between this component and another component entity.
        ///
        /// \param Target The component entity to associate with this component.
        /// \return This component, allowing for method chaining.
        ZY_INLINE Component With(Component Target) const
        {
            ecs_add_id(mWorld, mHandle, ecs_pair(EcsWith, Target.GetHandle()));
            return (* this);
        }

        /// \brief Establishes a dependency relationship between this component and another component type.
        ///
        /// \param Target The component entity that this component depends on.
        /// \return This component, allowing for method chaining.
        ZY_INLINE Component DependsOn(Entity Target) const
        {
            ecs_add_id(mWorld, mHandle, ecs_pair(EcsDependsOn, Target.GetHandle()));
            return (* this);
        }

    private:

        /// \brief Gets the record describing how much storage one instance of this component occupies.
        ///
        /// \return The storage record of this component.
        ZY_INLINE ConstRef<EcsComponent> Describe() const
        {
            const ConstPtr<EcsComponent> Record
                = static_cast<ConstPtr<EcsComponent>>(ecs_get_id(mWorld, mHandle, ecs_id(EcsComponent)));

            ZY_ASSERT(Record, "Entity does not describe a component");

            return (* Record);
        }

        /// \brief Grants a specific trait to this component.
        ///
        /// \param Trait The trait to grant.
        ZY_INLINE void ApplyTrait(Trait Trait) const
        {
            switch (Trait)
            {
            case Trait::Serializable:
                Set(Factory::Create<Type>());
                break;
            case Trait::Inheritable:
                ecs_add_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsInherit));
                break;
            case Trait::Local:
                ecs_add_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsDontInherit));
                break;
            case Trait::Toggleable:
                ecs_add_id(mWorld, mHandle, EcsCanToggle);
                break;
            case Trait::Sparse:
                ecs_add_id(mWorld, mHandle, EcsSparse);
                break;
            case Trait::Associative:
                ecs_add_id(mWorld, mHandle, EcsPairIsTag);
                break;
            case Trait::Singleton:
                ecs_add_id(mWorld, mHandle, EcsSingleton);
                break;
            case Trait::Final:
                ecs_add_id(mWorld, mHandle, EcsFinal);
                break;
            case Trait::Symmetric:
                ecs_add_id(mWorld, mHandle, EcsSymmetric);
                break;
            case Trait::Exclusive:
                ecs_add_id(mWorld, mHandle, EcsExclusive);
                break;
            }
        }

        /// \brief Revokes a specific trait from this component.
        ///
        /// \param Trait The trait to revoke.
        ZY_INLINE void EraseTrait(Trait Trait) const
        {
            switch (Trait)
            {
            case Trait::Serializable:
                Remove<Factory>();
                break;
            case Trait::Inheritable:
                ecs_remove_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsInherit));
                break;
            case Trait::Local:
                ecs_remove_id(mWorld, mHandle, ecs_pair(EcsOnInstantiate, EcsDontInherit));
                break;
            case Trait::Toggleable:
                ecs_remove_id(mWorld, mHandle, EcsCanToggle);
                break;
            case Trait::Sparse:
                ecs_remove_id(mWorld, mHandle, EcsSparse);
                break;
            case Trait::Associative:
                ecs_remove_id(mWorld, mHandle, EcsPairIsTag);
                break;
            case Trait::Singleton:
                ecs_remove_id(mWorld, mHandle, EcsSingleton);
                break;
            case Trait::Final:
                ecs_remove_id(mWorld, mHandle, EcsFinal);
                break;
            case Trait::Symmetric:
                ecs_remove_id(mWorld, mHandle, EcsSymmetric);
                break;
            case Trait::Exclusive:
                ecs_remove_id(mWorld, mHandle, EcsExclusive);
                break;
            }
        }
    };
}