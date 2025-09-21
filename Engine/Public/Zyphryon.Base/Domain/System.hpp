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

#include "Zyphryon.Base/Collection.hpp"
#include "Zyphryon.Base/Time.hpp"
#include "Zyphryon.Base/Memory/Tracker.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Base
{
    /// \brief A container and lifecycle manager for service units.
    template<typename Unit>
    class System
    {
    public:

        /// \brief Specifies the execution mode of the system.
        enum class Mode : UInt8
        {
            Client, ///< Active only in client mode.
            Server, ///< Active only in server mode.
            Both,   ///< Active in both client and server modes.
        };

    public:

        /// \brief Default constructor initializing the system in both client and server modes.
        ZYPHRYON_INLINE System()
            : mMode { Mode::Both }
        {
        }

        /// \brief Virtual destructor to ensure proper cleanup through derived types.
        virtual ~System() = default;

        /// \brief Sets the execution mode of the system.
        /// 
        /// \param Mode The new execution mode to set.
        ZYPHRYON_INLINE void SetMode(Mode Mode)
        {
            mMode = Mode;
        }

        /// \brief Checks if the system should operate in client mode.
        /// 
        /// \return `true` if the system is not running in server-only mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsClientMode() const
        {
            return mMode != Mode::Server;
        }

        /// \brief Checks if the system should operate in server mode.
        /// 
        /// \return `true` if the system is not running in client-only mode, `false` otherwise.
        ZYPHRYON_INLINE Bool IsServerMode() const
        {
            return mMode != Mode::Client;
        }

        /// \brief Updates all registered services.
        /// 
        /// \param Time The current simulation time.
        ZYPHRYON_INLINE void Tick(ConstRef<Time> Time)
        {
            for (ConstTracker<Unit> Service : mRegistry)
            {
                Service->OnTick(Time);
            }
        }

        /// \brief Adds a new service instance of the specified type.
        /// 
        /// \tparam Type The concrete service type to create.
        /// \param Parameters The forwarded constructor arguments.
        /// \return A tracker managing the newly created service.
        template<typename Type, typename ... Arguments>
        Tracker<Type> AddService(AnyRef<Arguments>... Parameters)
        {
            const Tracker<Type> Instance = Tracker<Type>::Create(* this, Forward<Arguments>(Parameters)...);
            mRegistry.push_back(Instance);
            return Instance;
        }

        /// \brief Retrieves a previously added service of the specified type.
        /// 
        /// \tparam Type The service type to search for.
        /// \return A tracker to the service instance if found, or `nullptr` otherwise.
        template<typename Type>
        Tracker<Type> GetService()
        {
            for (ConstTracker<Unit> Service : mRegistry)
            {
                if (Service->GetGuid() == HashType<Type>())
                {
                    return Tracker<Type>::Cast(Service);
                }
            }
            return nullptr;
        }

        /// \brief Removes the service of the specified type, if present.
        /// 
        /// \tparam Type The service type to remove.
        template<typename Type>
        void RemoveService()
        {
            constexpr auto FindByID = [](ConstTracker<Unit> Service)
            {
                return Service->GetGuid() == HashType<Type>();
            };
            mRegistry.erase(std::remove_if(mRegistry.begin(), mRegistry.end(), FindByID), mRegistry.end());
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