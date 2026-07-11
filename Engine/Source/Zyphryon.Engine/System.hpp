// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Engine
{
    /// \brief A system that manages a collection of subsystems and coordinates their lifecycle.
    template<typename Unit>
    class System
    {
    public:

        /// \brief Ticks all registered subsystems with the given time.
        ///
        /// \param Time The current time in seconds.
        ZY_INLINE void Tick(Real64 Time)
        {
            for (ConstRef<typename decltype(mServices)::Pair> Pair : mServices)
            {
                Pair.Second->OnTick(Time);
            }
        }

        /// \brief Tears down all registered subsystems and clears the service registry.
        ZY_INLINE void Teardown()
        {
            for (ConstRef<typename decltype(mServices)::Pair> Pair : mServices)
            {
                Pair.Second->OnTeardown();
            }
            mServices.Clear();
        }

        /// \brief Registers a new service instance with the system.
        ///
        /// \tparam Type       The concrete service type to register.
        /// \param  Parameters The constructor arguments to forward to the service.
        /// \return The registered service instance.
        template<typename Type, typename ... Arguments>
        ZY_INLINE Retainer<Type> Register(AnyRef<Arguments>... Parameters)
        {
            ZY_ASSERT(!GetService<Type>(), "Service is already registered in the system");

            const Retainer<Type> Instance = Retainer<Type>::Create(* this, Forward<Arguments>(Parameters)...);
            mServices.Assign(Hash<Type>(), Instance);
            return Instance;
        }

        /// \brief Unregisters a service from the system.
        ///
        /// \tparam Type The service type to unregister.
        template<typename Type>
        ZY_INLINE void Unregister()
        {
            mServices.Erase(Hash<Type>());
        }

        /// \brief Returns a handle to the registered service of the specified type.
        ///
        /// \tparam Type The service type to look up.
        /// \return A handle to the service, or an empty handle if not found.
        template<typename Type>
        ZY_INLINE Retainer<Type> GetService() const
        {
            const Retainer<Unit> Result = mServices.FindOrDefault(Hash<Type>());
            return (Result ? Retainer<Type>::Cast(Result) : nullptr);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Table<UInt64, Retainer<Unit>> mServices;
    };
}