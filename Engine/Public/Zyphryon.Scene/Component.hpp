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

#include "Entity.hpp"
#include "Factory.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a typed component within the ECS (Entity-Component System).
    ///
    /// \tparam Class The C++ type that defines this component.
    template<typename Class>
    class Component : public Entity
    {
    public:

        /// \brief Constructs an empty component.
        ZYPHRYON_INLINE Component() = default;

        /// \brief Constructs a component from an existing entity handle.
        ///
        /// \param Handle The handle of this component.
        ZYPHRYON_INLINE Component(Handle Handle)
            : Entity(Handle)
        {
        }

        /// \brief Constructs a component from an identifier.
        ///
        /// \param Id The identifier that defines this component.
        ZYPHRYON_INLINE Component(flecs::id Id)
            : Entity(Id)
        {
        }

        /// \brief Retrieves the size, in bytes, of the component type.
        ///
        /// \return The size of the component in bytes.
        ZYPHRYON_INLINE UInt32 GetSize() const
        {
            return Get<flecs::Component>().size;
        }

        /// \brief Retrieves the alignment requirement of the component type.
        ///
        /// \return The alignment of the component in bytes.
        ZYPHRYON_INLINE UInt32 GetAlignment() const
        {
            return Get<flecs::Component>().alignment;
        }

        /// \brief Adds one or more behavioral traits to this component.
        ///
        /// \param Traits The traits to apply.
        ///
        /// \return A reference to this component.
        template<typename... Arguments>
        ZYPHRYON_INLINE ConstRef<Component> AddTrait(Arguments... Traits) const
        {
            (ApplyTrait(Traits), ...);
            return (* this);
        }

        /// \brief Removes one or more behavioral traits from this component.
        ///
        /// \param Traits The traits to remove.
        ///
        /// \return A reference to this component.
        template<typename... Arguments>
        ZYPHRYON_INLINE ConstRef<Component> RemoveTrait(Arguments... Traits) const
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
        ///
        /// \return A reference to this component.
        template<typename Target>
        ZYPHRYON_INLINE ConstRef<Component> With() const
        {
            mHandle.add(EcsWith, mHandle.world().template component<Target>());
            return (* this);
        }

        /// \brief Establishes a "with" context between this component and another component entity.
        ///
        /// \param Target The component entity to associate with this component.
        ///
        /// \return A reference to this component.
        ZYPHRYON_INLINE ConstRef<Component> With(Component Target) const
        {
            mHandle.add(EcsWith, Target.GetHandle());
            return (* this);
        }

        /// \brief Establishes a dependency relationship between this component and another component type.
        ///
        /// \param Target The component entity that this component depends on.
        /// \return A reference to this component.
        ZYPHRYON_INLINE ConstRef<Component> DependsOn(Entity Target) const
        {
            mHandle.add(EcsDependsOn, Target.GetHandle());
            return (* this);
        }

    private:

        /// \brief Applies a specific trait to this component.
        ///
        /// \param Trait The trait to apply.
        ZYPHRYON_INLINE void ApplyTrait(Trait Trait) const
        {
            switch (Trait)
            {
                case Trait::Serializable:
                    mHandle.set<Factory>(Factory::Create<Class>());
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
                case Trait::Phase:
                    mHandle.add(flecs::Phase);
                    break;
            }
        }

        /// \brief Removes a specific trait from this component.
        ///
        /// \param Trait The trait to remove.
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
                case Trait::Phase:
                    mHandle.remove(flecs::Phase);
                    break;
            }
        }
    };
}