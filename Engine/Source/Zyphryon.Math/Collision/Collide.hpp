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

#include "Manifold.hpp"
#include "Zyphryon.Math/Geometry/Cylinder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    /// \brief Finds whether two standing volumes share a spot, and the shortest way one has to move to be clear.
    ///
    /// \note This is the case \ref Sweep declines: a pair already overlapping has no face the motion came in through.
    class Collide final
    {
    public:

        /// \brief Checks whether two boxes share a spot, and how one leaves the other by the shortest way.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and how far one has sunk in, untouched when the two are clear.
        /// \return `true` if the two overlap, `false` otherwise.
        static Bool Test(ConstRef<Box> First, ConstRef<Box> Second, Ref<Manifold> Contact);

        /// \brief Checks whether two cylinders share a spot, and how one leaves the other by the shortest way.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and how far one has sunk in, untouched when the two are clear.
        /// \return `true` if the two overlap, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Ref<Manifold> Contact);

        /// \brief Checks whether a cylinder shares a spot with a box, and how it leaves by the shortest way.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and how far one has sunk in, untouched when the two are clear.
        /// \return `true` if the two overlap, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Ref<Manifold> Contact);

        /// \brief Checks whether a box shares a spot with a cylinder, and how it leaves by the shortest way.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and how far one has sunk in, untouched when the two are clear.
        /// \return `true` if the two overlap, `false` otherwise.
        static Bool Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Ref<Manifold> Contact);

    private:

        /// \brief Measures how much of their height two spans hold in common.
        ///
        /// \param FirstBottom  The underside of the first span.
        /// \param FirstTop     The top of the first span.
        /// \param SecondBottom The underside of the second span.
        /// \param SecondTop    The top of the second span.
        /// \return How much of the height the two share, which is zero or less when they never meet.
        static Real32 Shared(Real32 FirstBottom, Real32 FirstTop, Real32 SecondBottom, Real32 SecondTop);

        /// \brief Settles which way out of a contact is shorter, the one across the ground or the one up the height.
        ///
        /// \param Apart    The way out across the ground, which need not be normalized.
        /// \param Span     The length of \p Apart, so it is measured once.
        /// \param Ground   How far one has sunk into the other across the ground.
        /// \param Standing How far one has sunk into the other up the height.
        /// \param Under    `true` when the first volume stands below the second.
        /// \param Contact  Receives the way out and the depth along it.
        /// \return `true` always, so a caller may hand the answer straight back.
        static Bool Escapes(Vector2 Apart, Real32 Span, Real32 Ground, Real32 Standing, Bool Under, Ref<Manifold> Contact);
    };
}