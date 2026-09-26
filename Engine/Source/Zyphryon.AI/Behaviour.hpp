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

#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyAI
{
    /// \brief Represents a behaviour tree, stored as its nodes in depth-first order.
    class ZY_API Behaviour final : public ZyContent::AbstractResource<Behaviour>
    {
    public:

        /// \brief Enumerates the kinds of behaviour node.
        enum class Kind : UInt8
        {
            Sequence, ///< Runs its children in order, and fails on the first child that fails.
            Selector, ///< Runs its children in order, and succeeds on the first child that succeeds.
            Parallel, ///< Runs every child each tick, and succeeds once `Count` children succeed, or all at `0`.
            Invert,   ///< Runs its child and swaps success with failure.
            Repeat,   ///< Runs its child again after each success, `Count` times, or forever at `0`.
            Cooldown, ///< Fails without running its child until `Value` seconds after the child last finished.
            Timeout,  ///< Runs its child, and stops it and fails once it has run for `Value` seconds.
            Chance,   ///< Runs its child with a probability of `Value`, from `0` to `1`, and fails otherwise.
            Leaf,     ///< Runs the host action named by `Name`.
            Compare,  ///< Succeeds when the blackboard entry `Name` passes `Check` against `Value`, and fails otherwise.
        };

        /// \brief Enumerates the tests a compare node applies to a blackboard entry.
        enum class Test : UInt8
        {
            Exists,       ///< The entry exists, whatever it holds.
            Equal,        ///< The entry equals `Value`.
            NotEqual,     ///< The entry differs from `Value`.
            Less,         ///< The entry is less than `Value`.
            LessEqual,    ///< The entry is less than or equal to `Value`.
            Greater,      ///< The entry is greater than `Value`.
            GreaterEqual, ///< The entry is greater than or equal to `Value`.
        };

        /// \brief Represents one node of the tree.
        struct Node final
        {
            /// The kind of the node.
            Kind      Type      = Kind::Leaf;

            /// The number of direct children.
            UInt16    Children  = 0;

            /// The number of nodes in its subtree, itself included, which is the offset to its next sibling.
            UInt16    Span      = 1;

            /// The count a parallel or repeat reads.
            UInt16    Count     = 0;

            /// The seconds, probability or operand a cooldown, timeout, chance or compare reads.
            Real64    Value     = 0.0;

            /// The slot of a leaf's arguments in the argument list, counted from `1`, or `0` when it has none.
            UInt16    Arguments = 0;

            /// The test a compare node applies.
            Test      Check     = Test::Exists;

            /// The name of the host action a leaf runs, or of the blackboard entry a compare node reads.
            Str32     Name;
        };

        /// \brief The maximum number of nodes in a behaviour.
        static constexpr UInt32 kMaxNodes = kMaximum<UInt16>;

    public:

        /// \brief Constructs a behaviour resource with the given content key.
        ///
        /// \param Key The unique content key identifying this behaviour.
        explicit Behaviour(AnyRef<ZyContent::Uri> Key);

        /// \brief Replaces the nodes of the tree and the arguments its leaves point into.
        ///
        /// \param Nodes     The nodes in depth-first order, root first.
        /// \param Arguments The arguments of the leaves that have any, each a JSON object, in slot order.
        ZY_INLINE void SetNodes(AnyRef<Sequence<Node>> Nodes, AnyRef<Sequence<JsonValue>> Arguments)
        {
            mNodes     = Move(Nodes);
            mArguments = Move(Arguments);
        }

        /// \brief Gets the nodes of the tree.
        ///
        /// \return The nodes in depth-first order, root first, or none when the tree is empty.
        ZY_INLINE ConstSpan<Node> GetNodes() const
        {
            return mNodes;
        }

        /// \brief Gets the arguments of a leaf.
        ///
        /// \param Index The index of the node within \ref GetNodes.
        /// \return The arguments as a JSON object, or `nullptr` when the node has none.
        ZY_INLINE ConstPtr<JsonValue> GetArguments(UInt32 Index) const
        {
            const UInt16 Slot = mNodes[Index].Arguments;
            return Slot ? AddressOf(mArguments[Slot - 1]) : nullptr;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Node>      mNodes;
        Sequence<JsonValue> mArguments;
    };
}