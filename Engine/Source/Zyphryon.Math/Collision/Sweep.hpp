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
    /// \brief Finds where a moving volume first meets a standing one.
    class Sweep final
    {
    public:

        /// \brief The shortest length worth treating as a direction; below it a motion stands still.
        static constexpr Real32 kMinimumReach = 0.0001f;

    public:

        /// \brief Finds where a moving volume first meets a standing one.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  How far the first volume travels.
        /// \param Contact Receives where the two meet, unchanged when they never do.
        /// \return `true` if the two meet within the motion, `false` otherwise.
        static Bool Test(ConstRef<Box> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold> Contact);

        /// \brief Finds where a moving volume first meets a standing one.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  How far the first volume travels.
        /// \param Contact Receives where the two meet, unchanged when they never do.
        /// \return `true` if the two meet within the motion, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold> Contact);

        /// \brief Finds where a moving volume first meets a standing one.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  How far the first volume travels.
        /// \param Contact Receives where the two meet, unchanged when they never do.
        /// \return `true` if the two meet within the motion, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold> Contact);

        /// \brief Finds where a moving volume first meets a standing one.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  How far the first volume travels.
        /// \param Contact Receives where the two meet, unchanged when they never do.
        /// \return `true` if the two meet within the motion, `false` otherwise.
        static Bool Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold> Contact);

    private:

        /// \brief Holds the stretch of the motion that is still in play, and the face that last narrowed it.
        struct Interval final
        {
            /// The fraction of the motion the two first touch at.
            Real32  Entry  = 0.0f;

            /// The fraction of the motion the two last part at.
            Real32  Leave  = 1.0f;

            /// The way out of the face the entry was last narrowed by.
            Vector3 Normal = Vector3::Zero();
        };

        /// \brief Narrows the interval to the stretch of the motion that lies between one pair of faces.
        ///
        /// \param Apart  How far apart the two volumes stand along the axis.
        /// \param Reach  How far apart along the axis the two still touch at.
        /// \param Motion How far the first volume travels along the axis.
        /// \param Facing The way out of the faces this axis stands for.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowSlab(Real32 Apart, Real32 Reach, Real32 Motion, Vector3 Facing, Ref<Interval> Result);

        /// \brief Narrows the interval to the stretch of the motion that lies inside one circle on the ground.
        ///
        /// \param Apart  How far apart the two volumes stand across the ground.
        /// \param Reach  How far apart across the ground the two still touch at.
        /// \param Motion How far the first volume travels across the ground.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowRound(Vector2 Apart, Real32 Reach, Vector2 Motion, Ref<Interval> Result);

        /// \brief Narrows the interval against a rectangle grown by a radius, which is two slabs and four corners.
        ///
        /// \param Apart  How far apart the two volumes stand across the ground.
        /// \param Extent The half-size of the rectangle across the ground.
        /// \param Reach  The radius the rectangle is grown by.
        /// \param Motion How far the first volume travels across the ground.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowGrown(Vector2 Apart, Vector2 Extent, Real32 Reach, Vector2 Motion, Ref<Interval> Result);

        /// \brief Settles a narrowed interval into a contact.
        ///
        /// \param Result  The interval every narrowing left behind.
        /// \param Met     `true` when each narrowing in turn kept some of the motion.
        /// \param Contact Receives where the two meet, unchanged when they never do.
        /// \return `true` if the two meet within the motion, `false` otherwise.
        static Bool Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold> Contact);
    };
}