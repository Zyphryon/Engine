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

#include "Box.hpp"
#include "Sphere.hpp"
#include "Zyphryon.Math/Matrix4x4.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Represents the six planes a view-projection closes the world off with, for asking what lies inside them.
    class ZY_API Frustum final
    {
    public:

        /// \brief Specifies which edge of the view a plane closes.
        enum class Side : UInt8
        {
            Left,   ///< Past the left edge of the view.
            Right,  ///< Past the right edge of the view.
            Bottom, ///< Past the bottom edge of the view.
            Top,    ///< Past the top edge of the view.
            Near,   ///< Nearer than the view reaches.
            Far,    ///< Farther than the view reaches.
        };

    public:

        /// \brief Constructs a frustum with no planes set, which nothing lies outside of.
        ZY_INLINE Frustum() = default;

        /// \brief Gets one of the planes, as an inward-facing normal and the distance along it.
        ///
        /// \param Plane The side to get.
        /// \return The plane, its normal in the first three components and its distance in the fourth.
        Vector4 GetPlane(Side Plane) const;

        /// \brief Checks whether a point lies inside every plane.
        ///
        /// \param Point The point to test.
        /// \return `true` when the point is inside, `false` otherwise.
        Bool Test(Vector3 Point) const;

        /// \brief Checks whether any of a sphere lies inside every plane.
        ///
        /// \param Volume The sphere to test.
        /// \return `true` when any of the sphere is inside, `false` otherwise.
        Bool Test(ConstRef<Sphere> Volume) const;

        /// \brief Checks whether any of a box lies inside every plane.
        ///
        /// \param Volume The box to test.
        /// \return `true` when any of the box is inside, `false` otherwise.
        Bool Test(ConstRef<Box> Volume) const;

    public:

        /// \brief Extracts the planes from a view-projection whose clip depth runs from zero to w.
        ///
        /// \param ViewProjection The matrix that takes world coordinates to clip coordinates.
        /// \return The frustum it closes.
        static Frustum FromMatrix(ConstRef<Matrix4x4> ViewProjection);

    private:

        /// \brief Four planes side by side, one register per coefficient, so all four are tested at once.
        ///
        /// \note The registers hold the X, Y and Z of the four normals, then the four distances.
        using Quartet = Array<Vector4, 4>;

        /// \brief Scales four planes so each normal has unit length, leaving a plane with no normal as it is.
        ///
        /// \param Group The four planes to normalize in place.
        static void Normalize(Ref<Quartet> Group);

        /// \brief Measures how far a point lies inside each of four planes at once.
        ///
        /// \param Group The four planes.
        /// \param X     The X coordinate of the point, in every lane.
        /// \param Y     The Y coordinate of the point, in every lane.
        /// \param Z     The Z coordinate of the point, in every lane.
        /// \return The signed distance to each plane, negative on the outside.
        static Vector4 Measure(ConstRef<Quartet> Group, Vector4 X, Vector4 Y, Vector4 Z);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Quartet, 2> mQuartets;
    };
}