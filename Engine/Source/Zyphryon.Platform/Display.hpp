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

namespace ZyPlatform
{
    /// \brief Represents a display system that manages monitors.
    class ZY_API Display final
    {
    public:

        /// \brief Poll the display system, updating the state of all monitors.
        void Poll();

        /// \brief Gets a monitor by its name.
        ///
        /// \note A display server may report no primary monitor, in which case the first one stands in for it.
        ///
        /// \param Name The name of the monitor to retrieve, or empty for the primary monitor.
        /// \return A reference to the monitor if found, otherwise the primary monitor.
        ZY_INLINE ConstRef<Monitor> GetMonitor(Text Name) const
        {
            ZY_ASSERT(!mMonitors.IsEmpty(), "The display has no monitor to choose from");

            ConstPtr<Monitor> Primary = AddressOf(mMonitors.GetFront());

            for (ConstRef<Monitor> Monitor : mMonitors)
            {
                if (!Name.IsEmpty() && Name == Monitor.GetName())
                {
                    return Monitor;
                }

                if (Monitor.IsPrimary())
                {
                    Primary = AddressOf(Monitor);
                }
            }
            return (* Primary);
        }

        /// \brief Gets a monitor by its position.
        ///
        /// \param X The X coordinate of the position.
        /// \param Y The Y coordinate of the position.
        /// \return A reference to the monitor if found, otherwise `nullptr`.
        ZY_INLINE ConstPtr<Monitor> GetMonitor(SInt32 X, SInt32 Y) const
        {
            for (ConstRef<Monitor> Monitor : mMonitors)
            {
                const SInt32 Right  = Monitor.GetX() + static_cast<SInt32>(Monitor.GetWidth());
                const SInt32 Bottom = Monitor.GetY() + static_cast<SInt32>(Monitor.GetHeight());

                if (X >= Monitor.GetX() && X < Right && Y >= Monitor.GetY() && Y < Bottom)
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