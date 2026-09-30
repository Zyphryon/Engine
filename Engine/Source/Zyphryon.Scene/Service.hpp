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

#include "Clock.hpp"
#include "World.hpp"
#include "Zyphryon.Engine/Subsystem.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyScene
{
    /// \brief Represents the scene service, which is the world itself and ticks the world's \ref Clock in place.
    class ZY_API Service final : public ZyEngine::Subsystem, public World
    {
    public:

        /// \brief Constructs the scene service and registers it with the system host.
        ///
        /// \param Host The system context that owns and manages this service.
        explicit Service(Ref<Host> Host);

        /// \brief Advances the world's clock, then runs every phase in order.
        ///
        /// \param Delta The elapsed time since the last tick.
        void OnTick(Real64 Delta) override;

        /// \brief Gets the world's time, scaled by the timescale.
        ///
        /// \return The absolute time of the world's clock, in seconds.
        ZY_INLINE Real64 GetTime() const
        {
            return GetWorld().Get<const Clock>()->GetAbsolute();
        }
        
        /// \brief Sets how fast the world's clock runs against real time, where zero holds it still.
        ///
        /// \param Timescale The multiplier applied to each tick.
        ZY_INLINE void SetTimescale(Real32 Timescale)
        {
            GetWorld().Get<Clock>()->SetMultiplier(Timescale);
        }

        /// \brief Gets how fast the world's clock runs against real time.
        ///
        /// \return The multiplier applied to each tick, zero while it is held still.
        ZY_INLINE Real32 GetTimescale() const
        {
            return GetWorld().Get<const Clock>()->GetMultiplier();
        }
    };
}