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

#include "Monitor.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Platform
{
    /// \brief Represents a display system that manages monitors.
    class Display final
    {
    public:

        /// \brief Poll the display system, updating the state of all monitors.
        void Poll();

        /// \brief Gets a monitor by its name.
        ///
        /// \param Name The name of the monitor to retrieve.
        /// \return A reference to the monitor if found, otherwise the primary monitor.
        ZY_INLINE ConstRef<Monitor> GetMonitor(Text Name) const
        {
            ConstPtr<Monitor> Primary = nullptr;

            for (ConstRef<Monitor> Monitor : mMonitors)
            {
                if (Monitor.IsPrimary())
                {
                    Primary = AddressOf(Monitor);

                    if (Name.IsEmpty() || Name == Primary->GetName())
                    {
                        return (* Primary);
                    }
                }
                else
                {
                    if (Name == Monitor.GetName())
                    {
                        return (* AddressOf(Monitor));
                    }
                }
            }
            return (* Primary);
        }

        /// \brief Gets a monitor by its position.
        ///
        /// \param X The X coordinate of the position.
        /// \param Y The Y coordinate of the position.
        /// \return A reference to the monitor if found, otherwise `nullptr`.
        ZY_INLINE ConstPtr<Monitor> GetMonitor(UInt32 X, UInt32 Y) const
        {
            for (ConstRef<Monitor> Monitor : mMonitors)
            {
                if (X >= Monitor.GetX() && X < Monitor.GetX() + Monitor.GetWidth() &&
                    Y >= Monitor.GetY() && Y < Monitor.GetY() + Monitor.GetHeight())
                {
                    return AddressOf(Monitor);
                }
            }
            return nullptr;
        }

        /// \brief Gets all available monitors attached to the system.
        ///
        /// \return The collection of all detected monitors.
        ZY_INLINE ConstSpan<Monitor> GetMonitors() const
        {
            return mMonitors;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Monitor> mMonitors;
    };
}