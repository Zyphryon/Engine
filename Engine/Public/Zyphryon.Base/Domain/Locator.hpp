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

#include "Dependency.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A class template that provides access to multiple service dependencies within a system.
    template<typename... Dependencies>
    class Locator : Dependency<Dependencies>...
    {
    public:

        /// \brief Constructs a provider by initializing all specified dependencies from the host.
        ///
        /// \param Host The host system from which to retrieve the services.
        ZYPHRYON_INLINE explicit Locator(Ref<Service::Host> Host)
            : Dependency<Dependencies>(Host)...
        {
        }

        /// \brief Retrieves a reference to the specified service type.
        ///
        /// \tparam Type The type of the service to retrieve.
        /// \return A reference to the service instance.
        template<typename Type>
        ZYPHRYON_INLINE Ref<Type> GetService()
        {
            return Dependency<Type>::Fetch();
        }

        /// \brief Retrieves a constant reference to the specified service type.
        ///
        /// \tparam Type The type of the service to retrieve.
        /// \return A constant reference to the service instance.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Type> GetService() const
        {
            return Dependency<Type>::Fetch();
        }
    };
}