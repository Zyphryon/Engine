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

#include "Common.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Scene
{
    /// \brief Represents a system that operates on entities matching queries within the ECS (Entity-Component System).
    ///
    /// Systems define logic that runs automatically or can be executed manually.
    class System
    {
    public:

        /// \brief Underlying handle type for the ECS system.
        using Handle = ecs_entity_t;

    public:

        /// \brief Constructs an empty system with no associated handle.
        ZY_INLINE System()
            : mWorld  { nullptr },
              mHandle { 0 }
        {
        }

        /// \brief Constructs a system from an existing handle.
        ///
        /// \param World  The world the system belongs to.
        /// \param Handle The handle of this system.
        ZY_INLINE System(Ptr<ecs_world_t> World, Handle Handle)
            : mWorld  { World },
              mHandle { Handle }
        {
        }

        /// \brief Destroys the system and releases its underlying resources.
        ///
        /// \note The system becomes invalid after destruction.
        ZY_INLINE void Destruct()
        {
            if (mHandle)
            {
                ecs_delete(mWorld, mHandle);
                mHandle = 0;
            }
        }

        /// \brief Checks if the system is currently enabled.
        ///
        /// \return `true` if the system is enabled and can be executed, `false` otherwise.
        ZY_INLINE Bool IsEnabled() const
        {
            return !ecs_has_id(mWorld, mHandle, EcsDisabled);
        }

        /// \brief Executes the system logic.
        ///
        /// \param Delta The time step to pass to the system.
        ZY_INLINE void Run(Real32 Delta) const
        {
            ecs_run(mWorld, mHandle, Delta, nullptr);
        }

        /// \brief Enables the system, allowing it to be executed.
        ///
        /// \return This system, allowing for method chaining.
        ZY_INLINE System Enable() const
        {
            ecs_enable(mWorld, mHandle, true);
            return (* this);
        }

        /// \brief Disables the system, preventing it from being executed.
        ///
        /// \return This system, allowing for method chaining.
        ZY_INLINE System Disable() const
        {
            ecs_enable(mWorld, mHandle, false);
            return (* this);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<ecs_world_t> mWorld;
        Handle           mHandle;
    };
}