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

namespace ZyScene
{
    /// \brief Represents a component type named at runtime, as tools and commands name one.
    class Component final
    {
    public:

        /// \brief Constructs a component that names no type.
        ZY_INLINE Component()
            : mIdentifier { 0 }
        {
        }

        /// \brief Constructs a component from the identifier its type answers to.
        ///
        /// \param Identifier The identifier, or zero for none.
        ZY_INLINE explicit Component(UInt32 Identifier)
            : mIdentifier { Identifier }
        {
        }

        /// \brief Checks whether the component names a type.
        ///
        /// \return `true` if it names one, `false` otherwise.
        ZY_INLINE Bool IsValid() const
        {
            return mIdentifier != 0;
        }

        /// \brief Gets the identifier the type answers to.
        ///
        /// \return The identifier, or zero for none.
        ZY_INLINE UInt32 GetID() const
        {
            return mIdentifier;
        }

        /// \brief Gets the name saves know the type by.
        ///
        /// \return The name, the compiler's unless the type was declared under another.
        ZY_INLINE Text GetName() const
        {
            return GetInfo().Name;
        }

        /// \brief Checks whether instances read the type from their archetype rather than holding a copy.
        ///
        /// \return `true` if they read it, `false` if they copy it or it never reaches them.
        ZY_INLINE Bool IsInheritable() const
        {
            return GetInfo().Has(Trait::Inheritable);
        }

        /// \brief Checks whether the type never reaches an instance made from an archetype.
        ///
        /// \return `true` if it stays on the archetype, `false` otherwise.
        ZY_INLINE Bool IsLocal() const
        {
            return GetInfo().Has(Trait::Local);
        }

        /// \brief Checks whether the world entity holds the one instance of the type.
        ///
        /// \return `true` for a singleton, `false` otherwise.
        ZY_INLINE Bool IsSingleton() const
        {
            return GetInfo().Has(Trait::Singleton);
        }

        /// \brief Gets how a tool shows the type.
        ///
        /// \return The description, or `nullptr` while nothing described the type.
        ZY_INLINE ConstPtr<Description> GetDescription() const
        {
            ConstRef<Metatype> Info = GetInfo();
            return Info.Presentation.Group.IsEmpty() ? nullptr : AddressOf(Info.Presentation);
        }

        /// \brief Gets the fields a tool reads and writes of the type.
        ///
        /// \return The fields, or `nullptr` while nothing reflected them.
        ZY_INLINE ConstPtr<ZyReflection::Schema> GetSchema() const
        {
            ConstRef<Metatype> Info = GetInfo();
            return Info.Layout.IsEmpty() ? nullptr : AddressOf(Info.Layout);
        }

        /// \brief Checks whether two components name the same type.
        ///
        /// \param Other The component to compare against.
        /// \return `true` if both name the same type, `false` otherwise.
        ZY_INLINE Bool operator==(ConstRef<Component> Other) const = default;

    public:

        /// \brief Gets the component a type answers to, registering it the first time it is asked about.
        ///
        /// \return The component, which always names a type.
        template<typename Type>
        ZY_INLINE static Component Of()
        {
            return Component(IdentifierOf<Type>());
        }

        /// \brief Finds the component registered under a name.
        ///
        /// \param Name The name saves know the type by, or its compiler name.
        /// \return The component, or one naming no type when nothing answers to the name.
        ZY_INLINE static Component Find(Text Name)
        {
            return Component(Registry::Get().Find(Name));
        }

        /// \brief Hands every registered component to a callback, in the order they were registered.
        ///
        /// \param Callback The callable, taking each component.
        template<typename Callable>
        ZY_INLINE static void ForEach(AnyRef<Callable> Callback)
        {
            for (UInt32 Identifier = 1, Count = Registry::Get().GetCount(); Identifier <= Count; ++Identifier)
            {
                Callback(Component(Identifier));
            }
        }

    private:

        /// \brief Gets how the type is stored, saved and shown.
        ///
        /// \return The description, which never moves for the life of the process.
        ZY_INLINE ConstRef<Metatype> GetInfo() const
        {
            ZY_ASSERT(IsValid(), "The component names no type");
            return Registry::Get().GetMetatype(mIdentifier);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32 mIdentifier;
    };
}