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

#include "System.hpp"
#include "Zyphryon.Base/Memory/Tracker.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A generic service class template for defining services within a system.
    ///
    /// Services are modular components that provide specific functionality within a system
    /// and can be added, retrieved, and managed by the host system.
    class Service : public Trackable<Service>
    {
    public:

        /// \brief Defines the host system type for this service.
        using Host = System<Service>;

    public:

        /// \brief Constructs a new service with the specified host and id.
        ///
        /// \param Host The host system that owns this service.
        /// \param ID   The unique identifier for this service.
        ZYPHRYON_INLINE Service(Ref<Host> Host, UInt64 ID)
            : mHost { Host },
              mID   { ID }
        {
        }

        /// \brief Virtual destructor for proper cleanup in derived classes.
        virtual ~Service() = default;

        /// \brief Retrieves the unique identifier of the service.
        ///
        /// \return The service ID.
        ZYPHRYON_INLINE UInt64 GetID() const
        {
            return mID;
        }

        /// \brief Called every tick to update the service.
        ///
        /// Override this method to implement custom update logic.
        ///
        /// \param Time The time delta since the last tick.
        virtual void OnTick(Time Time)
        {
        }

        /// \brief Called when the service is being destroyed.
        ///
        /// Override this method to implement custom teardown logic.
        virtual void OnTeardown()
        {
        }

    protected:

        /// \brief Retrieves the host system of the service.
        ///
        /// \return The host system.
        ZYPHRYON_INLINE Ref<Host> GetHost() const
        {
            return mHost;
        }

        /// \brief Checks if the system is in client mode.
        ///
        /// \return `true` if the system is in client mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsClientMode() const
        {
            return mHost.IsClientMode();
        }

        /// \brief Checks if the system is in server mode.
        ///
        /// \return `true` if the system is in server mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsServerMode() const
        {
            return mHost.IsServerMode();
        }

        /// \brief Adds a service of the specified type to the system.
        ///
        /// \param Parameters The constructor arguments for the service.
        /// \return A retainer to the newly added service.
        template<typename Type, typename ... Arguments>
        ZYPHRYON_INLINE Tracker<Type> AddService(AnyRef<Arguments>... Parameters)
        {
            return mHost.AddService<Type>(Parameters...);
        }

        /// \brief Retrieves a service of the specified type from the system.
        ///
        /// \return A retainer to the requested service, or `nullptr` if not found.
        template<typename Type>
        ZYPHRYON_INLINE Tracker<Type> GetService()
        {
            return mHost.GetService<Type>();
        }

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Host> mHost;
        UInt64    mID;
    };

    /// \brief A templated abstract service class for defining services of a specific type.
    template<typename Type>
    class AbstractService : public Service
    {
    public:

        /// \brief Constructs a new abstract service with the specified host.
        ///
        /// \param Host The host system that owns this service.
        ZYPHRYON_INLINE explicit AbstractService(Ref<Host> Host)
            : Service { Host, HashFNV1<Type>() }
        {
        }
    };
}