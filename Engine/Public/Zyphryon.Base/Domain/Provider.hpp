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

#include "Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief Provides automatic dependency injection of services into a derived class.
    ///
    /// Automatically retrieves a set of services from the system host at construction time, storing them internally.
    /// This allows derived classes  to access their dependencies directly without repeated lookups.
    ///
    /// \tparam Dependencies List of service types to inject.
    template<typename... Dependencies>
    class Provider
    {
    public:

        /// \brief Constructs the provider and fetches all specified service dependencies.
        ///
        /// \param Host The parent system host containing all registered services.
        ZYPHRYON_INLINE explicit Provider(Ref<Service::Host> Host)
            : mDependencies { Host.GetService<Dependencies>()... }
        {
            (VerifyDependency(Fetch<Tracker<Dependencies>>(mDependencies)), ...);
        }

        /// \brief Retrieves a reference to a specific injected service.
        ///
        /// \tparam Type The service type to retrieve.
        /// \return A reference to the requested service.
        template<typename Type>
        ZYPHRYON_INLINE Ref<Type> GetService()
        {
            return * Fetch<Tracker<Type>>(mDependencies);
        }

        /// \brief Retrieves a reference to a specific injected service.
        ///
        /// \tparam Type The service type to retrieve.
        /// \return A reference to the requested service.
        template<typename Type>
        ZYPHRYON_INLINE ConstRef<Type> GetService() const
        {
            return * Fetch<Tracker<Type>>(mDependencies);
        }

    private:

        /// \brief Verifies that a tracked service dependency is valid.
        ///
        /// \param Dependency A tracked reference to the service instance.
        template<typename Service>
        void VerifyDependency(ConstRef<Tracker<Service>> Dependency)
        {
            LOG_ASSERT(Dependency, "Missing service dependency: {}", typeid(Service).name());
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Tuple<Tracker<Dependencies>...> mDependencies;
    };
}