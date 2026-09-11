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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Represents a proxy reference to a content resource, allowing for lazy loading.
    template<typename Type>
    class Proxy final
    {
    public:

        /// \brief Creates a proxy without an associated resource.
        ZY_INLINE Proxy() = default;

        /// \brief Creates a proxy from a resource path.
        ///
        /// \param Path The URI path to the resource to create a proxy for.
        ZY_INLINE explicit Proxy(AnyRef<Uri> Path)
            : mPath     { Move(Path) },
              mResource { nullptr }
        {
        }

        /// \brief Creates a proxy from an existing resource.
        ///
        /// \param Resource The resource to create a proxy for.
        ZY_INLINE explicit Proxy(ConstRetainer<Type> Resource)
            : mPath     { Resource->GetKey() },
              mResource { Resource }
        {
        }

        /// \brief Sets the name of the resource the proxy stands for, letting go of whatever it held.
        ///
        /// \param Path The path the resource is loaded from.
        ZY_INLINE void SetKey(AnyRef<Uri> Path)
        {
            mPath     = Move(Path);
            mResource = nullptr;
        }

        /// \brief Gets the name of the resource the proxy stands for.
        ///
        /// \return The path the resource is loaded from.
        ZY_INLINE ConstRef<Uri> GetKey() const
        {
            return mPath;
        }

        /// \brief Gets the resource managed by the proxy.
        ///
        /// \return The managed resource.
        ZY_INLINE ConstRetainer<Type> GetResource() const
        {
            return mResource;
        }

        /// \brief Resolves the proxy's resource using the specified service.
        ///
        /// \param Service The content service to use for loading the resource.
        ZY_INLINE void Resolve(Ref<Service> Service)
        {
            if (mPath.IsValid())
            {
                mResource = Service.Load<Type>(mPath);
            }
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mPath);
        }

    public:

        /// \brief Provides the name this type is registered under in the reflection system.
        ///
        /// \return The fully qualified reflection name of the type, and how it is shown.
        ZY_INLINE static constexpr auto OnClassify()
        {
            return Reflection::Presentation { .Name = "Content.Asset", .Flat = true };
        }

        /// \brief Provides the reflected members of this type.
        ///
        /// \return The fields of the type, in the order they are shown.
        ZY_INLINE static constexpr auto OnDescribe()
        {
            return Array(Reflection::Field::Property<&Proxy::GetKey, &Proxy::SetKey>("Key"));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Uri            mPath;
        Retainer<Type> mResource;
    };
}