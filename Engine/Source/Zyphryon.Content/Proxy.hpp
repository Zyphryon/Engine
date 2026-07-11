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

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Uri            mPath;
        Retainer<Type> mResource;
    };
}