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

#include "Behaviour.hpp"
#include "Host.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    /// \brief Represents the running state of one agent through a \ref Behaviour.
    class ZY_API Brain final
    {
    public:

        /// \brief Constructs a brain with no running state.
        Brain();

        /// \brief Ticks a behaviour from its root, resuming every running node.
        ///
        /// \note The running state is reset when the behaviour's node count differs from the last one ticked.
        ///
        /// \param Tree  The behaviour to tick.
        /// \param Delta The time since the last tick, in seconds.
        /// \param Host  The host that runs the leaves.
        /// \return The status of the root, or \ref Status::Failure when the behaviour is empty.
        Status Tick(ConstRef<Behaviour> Tree, Real32 Delta, Ref<Host> Host);

        /// \brief Stops every running node, halting each running leaf through the host.
        ///
        /// \param Tree The behaviour last ticked.
        /// \param Host The host that runs the leaves.
        void Halt(ConstRef<Behaviour> Tree, Ref<Host> Host);

        /// \brief Clears the running state without halting any leaf.
        ///
        /// \note The blackboard is kept.
        void Reset();

        /// \brief Gets the status a node was left in by the last tick.
        ///
        /// \param Index The index of the node within \ref Behaviour::GetNodes.
        /// \return The status, or \ref Status::Idle when the node was never ticked.
        ZY_INLINE Status GetStatus(UInt32 Index) const
        {
            return Index < mMemory.GetSize() ? mMemory[Index].Last : Status::Idle;
        }

        /// \brief Gets the values the brain's leaves share.
        ///
        /// \return The blackboard of the brain.
        ZY_INLINE Ref<Blackboard> GetBlackboard()
        {
            return mBoard;
        }

        /// \brief Gets the values the brain's leaves share.
        ///
        /// \return The blackboard of the brain.
        ZY_INLINE ConstRef<Blackboard> GetBlackboard() const
        {
            return mBoard;
        }

        /// \brief Gets the time the brain has been ticked for.
        ///
        /// \return The sum of every tick's delta, in seconds.
        ZY_INLINE Real64 GetClock() const
        {
            return mClock;
        }

    private:

        /// \brief Represents the state kept for one node between ticks.
        struct Memory final
        {
            /// The status of the last tick.
            Status Last   = Status::Idle;

            /// The node index of the child a sequence or selector resumes from.
            UInt16 Cursor = 0;

            /// The runs a repeat has completed.
            UInt16 Count  = 0;

            /// The time a cooldown ends, or a timeout or leaf started, on the brain's clock.
            Real64 Mark   = 0.0;
        };

        /// \brief Ticks a node and the children it runs.
        ///
        /// \param Nodes The nodes of the behaviour.
        /// \param Index The index of the node.
        /// \param Host  The host that runs the leaves.
        /// \return The status of the node.
        Status Visit(ConstSpan<Behaviour::Node> Nodes, UInt32 Index, Ref<Host> Host);

        /// \brief Stops every running node of a subtree, halting each running leaf through the host.
        ///
        /// \param Nodes The nodes of the behaviour.
        /// \param Index The index of the subtree's root.
        /// \param Host  The host that runs the leaves.
        void Stop(ConstSpan<Behaviour::Node> Nodes, UInt32 Index, Ref<Host> Host);

        /// \brief Applies a compare node's test to the blackboard.
        ///
        /// \param Node The compare node.
        /// \return `true` when the entry passes the test, otherwise `false`.
        Bool Compare(ConstRef<Behaviour::Node> Node) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Memory> mMemory;
        Blackboard       mBoard;
        Real64           mClock;
    };
}