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

namespace Input
{
    /// \brief Represents a touch surface and every point resting on it.
    class Touch final
    {
    public:

        /// \brief The number of points the surface reports at once, beyond which further ones are turned away.
        constexpr static UInt32 kMaxPoints = 10;

        /// \brief Represents one point on the surface, from the moment it lands until the frame after it leaves.
        struct Point final
        {
            /// The identity the platform gave this point, held for as long as it rests on the surface.
            UInt32 ID     = 0;

            /// How far along its life the point is.
            Phase  Phase  = Phase::Began;

            /// The X coordinate of the point.
            Real32 X      = 0.0f;

            /// The Y coordinate of the point.
            Real32 Y      = 0.0f;

            /// How far the point moved along X since the frame opened.
            Real32 DeltaX = 0.0f;

            /// How far the point moved along Y since the frame opened.
            Real32 DeltaY = 0.0f;
        };

    public:

        /// \brief Begins a new frame, retiring the points that left during the last one.
        void Begin();

        /// \brief Processes a single input event to update the surface.
        ///
        /// \param Event The event to act on.
        void Process(ConstRef<Event> Event);

        /// \brief Clears every point resting on the surface.
        void Reset();

        /// \brief Cancels every point resting on the surface, so whoever polls learns each one went away.
        ///
        /// \param Output Receives one cancel event per point that was resting.
        void CancelAllPoints(Ref<Sequence<Event>> Output);

        /// \brief Gets the points the surface is carrying this frame.
        ///
        /// \return The points, in no particular order.
        ZY_INLINE ConstSpan<Point> GetPoints() const
        {
            return mPoints;
        }

        /// \brief Gets how many points the surface is carrying this frame.
        ///
        /// \return The number of points.
        ZY_INLINE UInt GetCount() const
        {
            return mPoints.GetSize();
        }

        /// \brief Checks whether nothing is resting on the surface.
        ///
        /// \return `true` if the surface carries no point, otherwise `false`.
        ZY_INLINE Bool IsEmpty() const
        {
            return mPoints.IsEmpty();
        }

        /// \brief Finds the point carrying the given identity.
        ///
        /// \param ID The identity to look for.
        /// \return The point, or `nullptr` when no point carries that identity.
        ZY_INLINE ConstPtr<Point> Find(UInt32 ID) const
        {
            for (ConstRef<Point> Entry : mPoints)
            {
                if (Entry.ID == ID)
                {
                    return AddressOf(Entry);
                }
            }
            return nullptr;
        }

    private:

        /// \brief Finds the point carrying the given identity.
        ///
        /// \param ID The identity to look for.
        /// \return The point, or `nullptr` when no point carries that identity.
        ZY_INLINE Ptr<Point> Locate(UInt32 ID)
        {
            for (Ref<Point> Entry : mPoints)
            {
                if (Entry.ID == ID)
                {
                    return AddressOf(Entry);
                }
            }
            return nullptr;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Point, kMaxPoints> mPoints;
    };
}