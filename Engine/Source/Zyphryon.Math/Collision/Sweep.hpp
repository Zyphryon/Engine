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
    /// \brief Provides the tests for where a moving shape first meets one standing still, and how far it may go.
    class ZY_API Sweep final
    {
    public:

        /// \brief The shortest motion along an axis worth treating as one, below which the axis stands still.
        static constexpr Real32 kMinimumReach = 0.0001f;

    public:

        /// \brief Finds where a moving box first meets a standing box.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  The whole displacement of the first volume, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Box> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold3D> Contact);

        /// \brief Finds where a moving cylinder first meets a standing cylinder.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  The whole displacement of the first volume, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold3D> Contact);

        /// \brief Finds where a moving cylinder first meets a standing box.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  The whole displacement of the first volume, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold3D> Contact);

        /// \brief Finds where a moving box first meets a standing cylinder.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  The whole displacement of the first volume, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold3D> Contact);

        /// \brief Finds where a moving volume of any kind first meets a standing one, by the test their pair has.
        ///
        /// \param First   The volume being moved.
        /// \param Second  The volume standing in its way.
        /// \param Motion  The whole displacement of the first volume, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Shape3D> First, ConstRef<Shape3D> Second, Vector3 Motion, Ref<Manifold3D> Contact);

        /// \brief Finds how much of a motion a volume may take before it comes within a skin of a standing one.
        ///
        /// \param First  The volume being moved.
        /// \param Second The volume standing in its way.
        /// \param Motion The whole displacement of the first volume, in world units.
        /// \param Skin   The gap the motion stops short of the contact by, in world units.
        /// \param Normal Receives the way out of the face met, unchanged when nothing is.
        /// \return The fraction of the motion that is free, from 0 to 1, which is 0 for one sinking further in.
        static Real32 Cast(
            ConstRef<Shape3D> First,
            ConstRef<Shape3D> Second,
            Vector3           Motion,
            Real32            Skin,
            Ref<Vector3>      Normal);

        /// \brief Finds where a moving rectangle first meets a standing rectangle.
        ///
        /// \param First   The shape being moved.
        /// \param Second  The shape standing in its way.
        /// \param Motion  The whole displacement of the first shape, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Rect> First, ConstRef<Rect> Second, Vector2 Motion, Ref<Manifold2D> Contact);

        /// \brief Finds where a moving circle first meets a standing circle.
        ///
        /// \param First   The shape being moved.
        /// \param Second  The shape standing in its way.
        /// \param Motion  The whole displacement of the first shape, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Circle> First, ConstRef<Circle> Second, Vector2 Motion, Ref<Manifold2D> Contact);

        /// \brief Finds where a moving circle first meets a standing rectangle.
        ///
        /// \param First   The shape being moved.
        /// \param Second  The shape standing in its way.
        /// \param Motion  The whole displacement of the first shape, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Circle> First, ConstRef<Rect> Second, Vector2 Motion, Ref<Manifold2D> Contact);

        /// \brief Finds where a moving rectangle first meets a standing circle.
        ///
        /// \param First   The shape being moved.
        /// \param Second  The shape standing in its way.
        /// \param Motion  The whole displacement of the first shape, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Rect> First, ConstRef<Circle> Second, Vector2 Motion, Ref<Manifold2D> Contact);

        /// \brief Finds where a moving flat shape of any kind first meets a standing one, by the test their pair has.
        ///
        /// \param First   The shape being moved.
        /// \param Second  The shape standing in its way.
        /// \param Motion  The whole displacement of the first shape, in world units.
        /// \param Contact Receives the face met and the time, its depth untouched, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Test(ConstRef<Shape2D> First, ConstRef<Shape2D> Second, Vector2 Motion, Ref<Manifold2D> Contact);

        /// \brief Finds how much of a motion a flat shape may take before it comes within a skin of a standing one.
        ///
        /// \param First  The shape being moved.
        /// \param Second The shape standing in its way.
        /// \param Motion The whole displacement of the first shape, in world units.
        /// \param Skin   The gap the motion stops short of the contact by, in world units.
        /// \param Normal Receives the way out of the face met, unchanged when nothing is.
        /// \return The fraction of the motion that is free, from 0 to 1, which is 0 for one sinking further in.
        static Real32 Cast(
            ConstRef<Shape2D> First,
            ConstRef<Shape2D> Second,
            Vector2           Motion,
            Real32            Skin,
            Ref<Vector2>      Normal);

    private:

        /// \brief Holds the stretch of the motion the two may meet within, and the face that last narrowed it.
        struct Interval final
        {
            /// The fraction of the motion the two first touch at.
            Real32  Entry  = 0.0f;

            /// The fraction of the motion the two last part at.
            Real32  Leave  = 1.0f;

            /// The way out of the face the entry was last narrowed by, which a flat pair keeps on x and z.
            Vector3 Normal = Vector3::Zero();
        };

        /// \brief Narrows the interval to the stretch of the motion that lies between one pair of faces.
        ///
        /// \param Apart  The distance between the two shapes along the axis.
        /// \param Reach  The largest distance along the axis at which the two still touch.
        /// \param Motion The distance the first shape travels along the axis.
        /// \param Facing The axis, of unit length, whose face against the motion becomes the way out.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowSlab(Real32 Apart, Real32 Reach, Real32 Motion, Vector3 Facing, Ref<Interval> Result);

        /// \brief Narrows the interval to the stretch of the motion that lies inside one circle on the ground.
        ///
        /// \param Apart  The offset between the two shapes across the ground.
        /// \param Reach  The largest distance across the ground at which the two still touch.
        /// \param Motion The displacement of the first shape across the ground.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowRound(Vector2 Apart, Real32 Reach, Vector2 Motion, Ref<Interval> Result);

        /// \brief Narrows the interval against a rectangle grown by a radius, which is two slabs and four corners.
        ///
        /// \param Apart  The offset between the two shapes across the ground.
        /// \param Extent The half-size of the rectangle across the ground.
        /// \param Reach  The radius the rectangle is grown by.
        /// \param Motion The displacement of the first shape across the ground.
        /// \param Result Receives the narrowed interval, left as it was when nothing survives.
        /// \return `true` if any of the motion survives the narrowing, `false` otherwise.
        static Bool NarrowGrown(Vector2 Apart, Vector2 Extent, Real32 Reach, Vector2 Motion, Ref<Interval> Result);

        /// \brief Settles a narrowed interval into the contact two volumes meet by.
        ///
        /// \param Result  The interval every narrowing left behind.
        /// \param Met     `true` when each narrowing in turn kept some of the motion, `false` once one emptied it.
        /// \param Contact Receives the face met and the time, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold3D> Contact);

        /// \brief Settles a narrowed interval into the contact two flat shapes meet by, its way out read off x and z.
        ///
        /// \param Result  The interval every narrowing left behind.
        /// \param Met     `true` when each narrowing in turn kept some of the motion, `false` once one emptied it.
        /// \param Contact Receives the face met and the time, unchanged when they never meet.
        /// \return `true` if the two meet within the motion, `false` when they never do or already overlap.
        static Bool Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold2D> Contact);

        /// \brief Finds how much of a motion a shape may take before it comes within a skin, which both casts share.
        ///
        /// \param First  The shape being moved.
        /// \param Second The shape standing in its way.
        /// \param Motion The whole displacement of the first shape, in world units.
        /// \param Skin   The gap the motion stops short of the contact by, in world units.
        /// \param Normal Receives the way out of the face met, unchanged when nothing is.
        /// \return The fraction of the motion that is free, from 0 to 1, which is 0 for one sinking further in.
        template<typename Form, typename Vector>
        static Real32 Free(ConstRef<Form> First, ConstRef<Form> Second, Vector Motion, Real32 Skin, Ref<Vector> Normal);
    };
}