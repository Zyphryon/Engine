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

#include <flecs.h>

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
        using Handle = flecs::system;

    public:

        /// \brief Constructs an empty system with no associated handle.
        ZYPHRYON_INLINE System() = default;

        /// \brief Constructs a system from an existing handle.
        ///
        /// \param Handle The handle of this system.
        ZYPHRYON_INLINE System(Handle Handle)
            : mHandle { Handle }
        {
        }

        /// \brief Destroys the system and releases its underlying resources.
        ///
        /// \note The system becomes invalid after destruction.
        ZYPHRYON_INLINE void Destruct()
        {
            if (mHandle)
            {
                mHandle.destruct();
                mHandle = Handle();
            }
        }

        /// \brief Checks if the system is currently enabled.
        ///
        /// \return `true` if the system is enabled and can be executed, `false` otherwise.
        ZYPHRYON_INLINE Bool IsEnabled() const
        {
            return !mHandle.has(EcsDisabled);
        }

        /// \brief Executes the system logic.
        ///
        /// \param Delta The time step to pass to the system.
        ZYPHRYON_INLINE void Run(Real32 Delta) const
        {
            mHandle.run(Delta);
        }

        /// \brief Enables the system, allowing it to be executed.
        ///
        /// \return The updated system.
        ZYPHRYON_INLINE System Enable() const
        {
            mHandle.enable();
            return (* this);
        }

        /// \brief Disables the system, preventing it from being executed.
        ///
        /// \return The updated system.
        ZYPHRYON_INLINE System Disable() const
        {
            mHandle.disable();
            return (* this);
        }

        /// \brief Toggles the enabled state of the system based on a boolean mask.
        ///
        /// \param Mask If `true`, the system will be enabled; if `false`, it will be disabled.
        /// @return The updated system.
        ZYPHRYON_INLINE System Toggle(Bool Mask) const
        {
            if (Mask)
            {
                return Enable();
            }
            return Disable();
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Handle mHandle;
    };
}