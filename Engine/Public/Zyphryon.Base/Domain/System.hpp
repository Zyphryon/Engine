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

#include "Zyphryon.Base/Collection.hpp"
#include "Zyphryon.Base/Hash.hpp"
#include "Zyphryon.Base/Time.hpp"
#include "Zyphryon.Base/Memory/Tracker.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A generic system class template for defining subsystems within the engine.
    template<typename Unit>
    class System
    {
    public:

        /// \brief Defines the operational mode of the system.
        enum class Mode : UInt8
        {
            Client, ///< Active only in client mode.
            Server, ///< Active only in server mode.
            Both,   ///< Active in both client and server modes.
        };

    public:

        /// \brief Default constructor that initializes the system in both client and server modes.
        ZYPHRYON_INLINE System()
            : mMode { Mode::Both }
        {
        }

        /// \brief Sets the operational mode of the system.
        ///
        /// \param Mode The mode to set for the system.
        ZYPHRYON_INLINE void SetMode(Mode Mode)
        {
            mMode = Mode;
        }

        /// \brief Checks if the system is in client mode.
        ///
        /// \return `true` if the system is in client mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsClientMode() const
        {
            return mMode != Mode::Server;
        }

        /// \brief Checks if the system is in server mode.
        ///
        /// \return `true` if the system is in server mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsServerMode() const
        {
            return mMode != Mode::Client;
        }

        /// \brief Advances the system by the specified time delta.
        ///
        /// \param Time The time elapsed since last update.
        ZYPHRYON_INLINE void Tick(Time Time) const
        {
            for (ConstTracker<Unit> Service : mRegistry)
            {
                Service->OnTick(Time);  // TODO: Multicast Delegate?
            }
        }

        /// \brief Adds a service of the specified type to the system.
        ///
        /// \param Parameters The constructor arguments for the service.
        /// \return A retainer to the newly added service.
        template<typename Type, typename ... Arguments>
        Tracker<Type> AddService(AnyRef<Arguments>... Parameters)
        {
            const Tracker<Type> Instance = Tracker<Type>::Create(* this, Forward<Arguments>(Parameters)...);
            mRegistry.push_back(Instance);
            return Instance;
        }

        /// \brief Retrieves a service of the specified type from the system.
        ///
        /// \return A retainer to the requested service, or `nullptr` if not found.
        template<typename Type>
        Tracker<Type> GetService()
        {
            for (ConstTracker<Unit> Service : mRegistry)
            {
                if (Service->GetID() == HashFNV1<Type>())
                {
                    return Tracker<Type>::Cast(Service);
                }
            }
            return nullptr;
        }

        /// \brief Removes a service of the specified type from the system.
        template<typename Type>
        void RemoveService()
        {
            std::erase_if(mRegistry, [](ConstTracker<Unit> Service)
            {
                return Service->GetID() == HashFNV1<Type>();
            });
        }

    protected:

        /// \brief Performs subsystem-level teardown in reverse registration order.
        void Teardown()
        {
            for (SInt32 Index = mRegistry.size() - 1; Index >= 0; --Index)
            {
                mRegistry[Index]->OnTeardown();
            }
            mRegistry.clear();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Mode                  mMode;
        Vector<Tracker<Unit>> mRegistry;
    };
}