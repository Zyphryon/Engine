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
    };
}