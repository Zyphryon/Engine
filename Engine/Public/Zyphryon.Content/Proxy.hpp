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
        ZYPHRYON_INLINE Proxy() = default;

        /// \brief Creates a proxy from a resource path.
        ///
        /// \param Path The URI path to the resource to create a proxy for.
        ZYPHRYON_INLINE Proxy(AnyRef<Uri> Path)
            : mPath     { Move(Path) },
              mResource { nullptr }
        {
        }

        /// \brief Creates a proxy from an existing resource.
        ///
        /// \param Resource The resource to create a proxy for.
        ZYPHRYON_INLINE Proxy(ConstTracker<Type> Resource)
            : mPath     { Resource->GetKey() },
              mResource { Resource }
        {
        }

        /// \brief Gets the resource managed by the proxy.
        ///
        /// \return The managed resource.
        ZYPHRYON_INLINE ConstTracker<Type> GetResource() const
        {
            return mResource;
        }

        /// \brief Resolves the proxy's resource using the specified service.
        ///
        /// \param Service The content service to use for loading the resource.
        ZYPHRYON_INLINE void Resolve(Ref<Service> Service)
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
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mPath);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Uri           mPath;        // TODO: Reusable IDs (Uri Table?)
        Tracker<Type> mResource;
    };
}