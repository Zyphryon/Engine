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
    /// \brief Represents a component type named at runtime.
    class Component final
    {
    public:

        /// \brief Constructs a component that names no type.
        ZY_INLINE constexpr Component()
            : mIdentifier { 0 }
        {
        }

        /// \brief Constructs a component from the identifier its type answers to.
        ///
        /// \param Identifier The identifier, or zero for none.
        ZY_INLINE constexpr explicit Component(UInt32 Identifier)
            : mIdentifier { Identifier }
        {
        }

        /// \brief Checks whether the component names a type at all.
        ///
        /// \return `true` if it names one, `false` otherwise.
        ZY_INLINE constexpr Bool IsValid() const
        {
            return mIdentifier != 0;
        }

        /// \brief Gets the identifier the type answers to.
        ///
        /// \return The identifier, or zero for none.
        ZY_INLINE constexpr UInt32 GetID() const
        {
            return mIdentifier;
        }

        /// \brief Gets how the type is stored, saved and shown.
        ///
        /// \return The description, which stays where it is for the life of the process.
        ZY_INLINE ConstRef<Metatype> GetInfo() const
        {
            ZY_ASSERT(IsValid(), "The component names no type");
            return Registry::Get().GetMetatype(mIdentifier);
        }

        /// \brief Gets the name saves know the type by.
        ///
        /// \return The name.
        ZY_INLINE Text GetName() const
        {
            return GetInfo().Name;
        }

        /// \brief Gets the size, in bytes, of one instance.
        ///
        /// \return The size, which is zero for a tag.
        ZY_INLINE UInt32 GetSize() const
        {
            return GetInfo().GetSize();
        }

        /// \brief Gets the alignment one instance needs.
        ///
        /// \return The alignment in bytes.
        ZY_INLINE UInt32 GetAlignment() const
        {
            return GetInfo().GetAlignment();
        }

        /// \brief Checks whether the type carries no data.
        ///
        /// \return `true` for a tag, `false` otherwise.
        ZY_INLINE Bool IsTag() const
        {
            return GetInfo().IsTag();
        }

        /// \brief Checks whether saves carry the type.
        ///
        /// \return `true` if they do, `false` otherwise.
        ZY_INLINE Bool IsSerializable() const
        {
            return GetInfo().Has(Trait::Serializable);
        }

        /// \brief Checks whether instances read the type from their archetype.
        ///
        /// \return `true` if they do, `false` otherwise.
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

        /// \brief Checks whether the world holds the one instance of the type.
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
            return Info.Presentation.Label.IsEmpty() ? nullptr : AddressOf(Info.Presentation);
        }

        /// \brief Gets the fields a tool reads and writes of the type.
        ///
        /// \return The fields, or `nullptr` while nothing reflected them.
        ZY_INLINE ConstPtr<ZyReflection::Schema> GetSchema() const
        {
            ConstRef<Metatype> Info = GetInfo();
            return Info.Layout.IsEmpty() ? nullptr : AddressOf(Info.Layout);
        }

        /// \brief Gets a hash value for the component based on its identifier.
        ///
        /// \return The identifier, used as its hash.
        ZY_INLINE UInt64 Hash(UInt64) const
        {
            return mIdentifier;
        }

        /// \brief Checks whether two components name the same type.
        ///
        /// \param Other The component to compare against.
        /// \return `true` if both name the same type, `false` otherwise.
        ZY_INLINE constexpr Bool operator==(ConstRef<Component> Other) const = default;

    public:

        /// \brief Gets the component a type answers to, registering it the first time it is asked about.
        ///
        /// \return The component.
        template<typename Type>
        ZY_INLINE static Component Of()
        {
            return Component(IdentifierOf<Type>());
        }

        /// \brief Gets the component registered under a name.
        ///
        /// \param Name The name saves know the type by, or its fully qualified one.
        /// \return The component, or one naming no type when nothing registered under the name.
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

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32 mIdentifier;
    };
}