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

#include "Zyphryon.Content/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Concept to check if a type supports resolution of deferred dependencies.
    template<typename Type>
    concept CanResolve = requires(Ref<Type> Object, Ref<Content::Service> Content)
    {
        Object.OnResolve(Content);
    };

    /// \brief Handles serialization and deserialization for a specific component type.
    class Factory final
    {
    public:

        /// \brief Type alias for resolver function.
        using Resolver   = void (*)(Ref<Content::Service>, Ptr<void>);

        /// \brief Type alias for serializer functions.
        template<typename Type>
        using Serializer = void (*)(Archive<Type>, Ptr<void>);

    public:

        /// \brief Constructs an empty factory with no serializers.
        ZYPHRYON_INLINE Factory() = default;

        /// \brief Constructs a factory with explicit serializer functions.
        ///
        /// \param Reader   Function pointer used for deserialization.
        /// \param Writer   Function pointer used for serialization.
        /// \param Resolver Function pointer used for resolving dependencies.
        ZYPHRYON_INLINE Factory(Serializer<Reader> Reader, Serializer<Writer> Writer, Resolver Resolver)
            : mReader   { Reader },
              mWriter   { Writer },
              mResolver { Resolver }
        {
        }

        /// \brief Deserializes a component from an archive.
        ///
        /// \param Reader    The archive instance performing the deserialization.
        /// \param Component Pointer to the raw memory of the component instance.
        ZYPHRYON_INLINE void Read(Ref<Reader> Reader, Ptr<void> Component) const
        {
            mReader(Archive(Reader), Component);
        }

        /// \brief Serializes a component into an archive.
        ///
        /// \param Writer    The archive instance performing the serialization.
        /// \param Component Pointer to the raw memory of the component instance.
        ZYPHRYON_INLINE void Write(Ref<Writer> Writer, Ptr<void> Component) const
        {
            mWriter(Archive(Writer), Component);
        }

        /// \brief Resolves deferred dependencies for a component.
        ///
        /// \param Service   The service used to load deferred resources.
        /// \param Component Pointer to the raw memory of the component instance.
        ZYPHRYON_INLINE void Resolve(Ref<Content::Service> Service, Ptr<void> Component) const
        {
            mResolver(Service, Component);
        }

    public:

        /// \brief Creates a Factory instance for a specific component type.
        ///
        /// \tparam Component The component type to generate serialization support for.
        ///
        /// \return Factory instance with type-specific serialization handlers.
        template<typename Component>
        ZYPHRYON_INLINE static Factory Create()
        {
            return Factory(&OnRead<Component>, &OnWrite<Component>, &OnResolve<Component>);
        }

    private:

        /// \brief Static deserialization callback for component data.
        ///
        /// \param Archive   The archive performing the deserialization.
        /// \param Component Pointer to the raw memory of the component instance.
        template<typename Type>
        ZYPHRYON_INLINE static void OnRead(Archive<Reader> Archive, Ptr<void> Component)
        {
            if constexpr (CanSerialize<Type, Base::Archive<Reader>>)
            {
                static_cast<Ptr<Type>>(Component)->OnSerialize(Archive);
            }
        }

        /// \brief Static serialization callback for component data.
        ///
        /// \param Archive   The archive performing the serialization.
        /// \param Component Pointer to the raw memory of the component instance.
        template<typename Type>
        ZYPHRYON_INLINE static void OnWrite(Archive<Writer> Archive, Ptr<void> Component)
        {
            if constexpr (CanSerialize<Type, Base::Archive<Writer>>)
            {
                static_cast<Ptr<Type>>(Component)->OnSerialize(Archive);
            }
        }

        /// \brief Static resolve callback for deferred component dependencies.
        ///
        /// \param Content   The content service used for resolving dependencies.
        /// \param Component Pointer to the raw memory of the component instance.
        template<typename Type>
        ZYPHRYON_INLINE static void OnResolve(Ref<Content::Service> Content, Ptr<void> Component)
        {
            if constexpr (CanResolve<Type>)
            {
                static_cast<Ptr<Type>>(Component)->OnResolve(Content);
            }
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Serializer<Reader> mReader;
        Serializer<Writer> mWriter;
        Resolver           mResolver;
    };
}