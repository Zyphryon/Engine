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

#include "Chart.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    /// \brief Represents a search for ways across a chart, keeping its buffers from one search to the next.
    class ZY_API Finder final
    {
    public:

        /// \brief Enumerates how far a search got toward its goal.
        enum class Outcome : UInt8
        {
            None,    ///< Found no way, because an end lies off the floor or no polygon nearer the goal was reached.
            Partial, ///< Found a way to the point nearest the goal, which is cut off or past the limit.
            Reached, ///< Found a way to the goal.
        };

        /// \brief The corners of a way from its start to its end, in cells with heights in height steps.
        using Way = Sequence<Vector3>;

    public:

        /// \brief Constructs a finder that has run no search yet.
        ZY_INLINE Finder()
            : mEpoch  { 0 },
              mAnchor { IntVector2::Zero() }
        {
        }

        /// \brief Finds a way between two points of a chart, as the corners to walk straight between.
        ///
        /// \param Map    The chart to search.
        /// \param Anchor The tile both points and the corners found are counted from, best the one set out from.
        /// \param From   The point to start from, in cells from the anchor with its height in height steps.
        /// \param Into   The point to reach, in cells from the anchor with its height in height steps.
        /// \param Reach  The farthest either point may lie off the floor and still be snapped onto it, in cells.
        /// \param Limit  The most polygons to expand before settling for the one nearest the goal, or zero for none.
        /// \param Way    Receives the corners of the way from the anchor at floor height, or nothing when none is.
        /// \return The part of the way found, from none through the point nearest the goal to the goal itself.
        Outcome Find(
            ConstRef<Chart> Map,
            IntVector2      Anchor,
            Vector3         From,
            Vector3         Into,
            Real32          Reach,
            UInt32          Limit,
            Ref<Way>        Way);

        /// \brief Moves a point toward another across the floor, sliding along any wall in the way.
        ///
        /// \param Map     The chart to move over.
        /// \param Anchor  The tile both points are counted from, best the one the move starts on.
        /// \param Polygon The polygon the point starts on.
        /// \param From    The point to move, in cells from the anchor.
        /// \param Into    The point to move toward, in cells from the anchor.
        /// \param Stop    Receives the point reached at floor height, the target itself when nothing blocks the move.
        /// \return The polygon under the point reached.
        UInt32 Glide(
            ConstRef<Chart> Map,
            IntVector2      Anchor,
            UInt32          Polygon,
            Vector2         From,
            Vector2         Into,
            Ref<Vector3>    Stop);

    private:

        /// \brief The weight of the estimate to the goal, just under one so a tie goes to the polygon closer to it.
        static constexpr Real32 kGuess  = 0.999f;

        /// \brief The most polygons a glide crosses in a straight line before it searches the walls nearby.
        static constexpr UInt32 kStride = 64;

        /// \brief Represents the search state of one polygon.
        struct Node final
        {
            /// The cost of the way to the entry point, in cells of ground that costs one.
            Real32  Cost   = 0.0f;

            /// The point the way enters the polygon at, in cells.
            Vector2 Entry;

            /// The left end of the side the way enters across, in cells.
            Vector2 Left;

            /// The right end of the side the way enters across, in cells.
            Vector2 Right;

            /// The polygon the way enters from, followed back to gather the way.
            UInt32  Parent = Chart::kNowhere;

            /// The stamp of the last search that reached the polygon, which tells a stale node from a live one.
            UInt32  Epoch  = 0;

            /// `true` once the polygon was expanded, `false` while a cheaper way to it may still turn up.
            Bool    Closed = false;
        };

        /// \brief Represents a polygon waiting in the frontier to be expanded.
        struct Candidate final
        {
            /// The cost of the way there plus the estimate of what is left, in cells of ground that costs one.
            Real32 Rank    = 0.0f;

            /// The polygon to expand.
            UInt32 Polygon = 0;

            /// \brief Orders candidates so the lowest rank is expanded first.
            ///
            /// \param Other The candidate to compare against.
            /// \return `true` when this one is expanded first, `false` otherwise.
            ZY_INLINE Bool operator<(ConstRef<Candidate> Other) const
            {
                return Rank < Other.Rank;
            }
        };

        /// \brief Represents a side the way crosses from one polygon into the next.
        struct Side final
        {
            /// The end of the side on the left of the way, in cells.
            Vector2 Left;

            /// The end of the side on the right of the way, in cells.
            Vector2 Right;

            /// The polygon past the side, whose floor gives the height of a corner on the side.
            UInt32  Into = Chart::kNowhere;
        };

        /// \brief Starts a search over a chart, leaving the nodes of earlier searches stale.
        ///
        /// \param Map The chart to search.
        void Begin(ConstRef<Chart> Map);

        /// \brief Pulls the way taut through the sides it crosses, keeping only the corners it turns at.
        ///
        /// \param Map  The chart the way was found over.
        /// \param From The start of the way, in cells with its height in height steps.
        /// \param Into The end of the way, in cells with its height in height steps.
        /// \param Way  Receives the corners of the way.
        void Pull(ConstRef<Chart> Map, Vector3 From, Vector3 Into, Ref<Way> Way);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt32           mEpoch;
        IntVector2       mAnchor;
        Sequence<Node>   mNodes;
        Heap<Candidate>  mFrontier;
        Sequence<Side>   mSides;
        Sequence<UInt32> mQueue;
    };
}