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

#include "Blackboard.hpp"
#include "Status.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    /// \brief Interface a \ref Brain calls to run the leaves of a \ref Behaviour.
    class Host
    {
    public:

        /// \brief Destroys the host.
        ZY_INLINE virtual ~Host() = default;

        /// \brief Ticks a leaf.
        ///
        /// \param Index   The index of the leaf within \ref Behaviour::GetNodes.
        /// \param Fresh   `true` when the leaf starts, `false` when it resumes after returning \ref Status::Running.
        /// \param Elapsed The time since the leaf started, in seconds.
        /// \param Board   The blackboard of the brain being ticked.
        /// \return The status of the leaf.
        virtual Status Tick(UInt32 Index, Bool Fresh, Real64 Elapsed, Ref<Blackboard> Board) = 0;

        /// \brief Stops a running leaf that will not be resumed.
        ///
        /// \param Index The index of the leaf within \ref Behaviour::GetNodes.
        virtual void Halt(UInt32 Index) = 0;

        /// \brief Rolls a random chance for a \ref Behaviour::Kind::Chance node.
        ///
        /// \param Chance The probability of passing, from `0` to `1`.
        /// \return `true` when the roll passes, otherwise `false`.
        virtual Bool Roll(Real32 Chance) = 0;
    };
}