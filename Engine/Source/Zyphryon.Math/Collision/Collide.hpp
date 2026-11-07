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
#include "Shape.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Provides the tests for whether two shapes standing where they are meet, and the shortest way out.
    ///
    /// \note This is the case \ref Sweep declines, as a pair already overlapping has no face the motion entered by.
    class ZY_API Collide final
    {
    public:

        /// \brief Checks whether two boxes overlap, and the axis one leaves the other by most shallowly.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two overlap on every axis, `false` when they are apart or only touch.
        static Bool Test(ConstRef<Box> First, ConstRef<Box> Second, Ref<Manifold3D> Contact);

        /// \brief Checks whether two cylinders meet, and whether one leaves the other across the ground or upward.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two share some height and touch across the ground, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Ref<Manifold3D> Contact);

        /// \brief Checks whether a cylinder meets a box, and whether it leaves across the ground or upward.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two share some height and touch across the ground, `false` otherwise.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Ref<Manifold3D> Contact);

        /// \brief Checks whether a box meets a cylinder, and whether it leaves across the ground or upward.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two share some height and touch across the ground, `false` otherwise.
        static Bool Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Ref<Manifold3D> Contact);

        /// \brief Checks whether two volumes of any kind meet, by the test their pair has.
        ///
        /// \param First   The volume to be pushed clear.
        /// \param Second  The volume it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two meet as the test of their pair counts it, `false` otherwise.
        static Bool Test(ConstRef<Shape3D> First, ConstRef<Shape3D> Second, Ref<Manifold3D> Contact);

        /// \brief Checks whether two rectangles overlap, and the axis one leaves the other by most shallowly.
        ///
        /// \param First   The shape to be pushed clear.
        /// \param Second  The shape it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two overlap on both axes, `false` when they are apart or only touch.
        static Bool Test(ConstRef<Rect> First, ConstRef<Rect> Second, Ref<Manifold2D> Contact);

        /// \brief Checks whether two circles meet, and the way one leaves the other between their centres.
        ///
        /// \param First   The shape to be pushed clear.
        /// \param Second  The shape it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two touch or overlap, `false` otherwise.
        static Bool Test(ConstRef<Circle> First, ConstRef<Circle> Second, Ref<Manifold2D> Contact);

        /// \brief Checks whether a circle meets a rectangle, and the way it leaves from the nearest point on it.
        ///
        /// \param First   The shape to be pushed clear.
        /// \param Second  The shape it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two touch or overlap, `false` otherwise.
        static Bool Test(ConstRef<Circle> First, ConstRef<Rect> Second, Ref<Manifold2D> Contact);

        /// \brief Checks whether a rectangle meets a circle, and the way it leaves from the nearest point on it.
        ///
        /// \param First   The shape to be pushed clear.
        /// \param Second  The shape it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two touch or overlap, `false` otherwise.
        static Bool Test(ConstRef<Rect> First, ConstRef<Circle> Second, Ref<Manifold2D> Contact);

        /// \brief Checks whether two flat shapes of any kind meet, by the test their pair has.
        ///
        /// \param First   The shape to be pushed clear.
        /// \param Second  The shape it is stuck in.
        /// \param Contact Receives the way out and the depth with no time, untouched when the two are clear.
        /// \return `true` if the two meet as the test of their pair counts it, `false` otherwise.
        static Bool Test(ConstRef<Shape2D> First, ConstRef<Shape2D> Second, Ref<Manifold2D> Contact);

    private:

        /// \brief Measures how much of their height two spans hold in common.
        ///
        /// \param FirstBottom  The underside of the first span.
        /// \param FirstTop     The top of the first span.
        /// \param SecondBottom The underside of the second span.
        /// \param SecondTop    The top of the second span.
        /// \return The height the two share, which is zero or less when they only touch or stand apart.
        static Real32 Shared(Real32 FirstBottom, Real32 FirstTop, Real32 SecondBottom, Real32 SecondTop);

        /// \brief Settles which way out of a volume is shorter, the one the flat test found or the one up the height.
        ///
        /// \param Ground   The way out across the ground and the depth along it, as the flat test found them.
        /// \param Standing The depth one has sunk into the other up the height.
        /// \param Under    `true` when the first volume is centred below the second, `false` level with it or above.
        /// \param Contact  Receives the way out and the depth along it.
        /// \return `true` always, so a test may hand the answer straight back.
        static Bool Escapes(ConstRef<Manifold2D> Ground, Real32 Standing, Bool Under, Ref<Manifold3D> Contact);
    };
}