// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
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
#include "Zyphryon.Math/Geometry/Circle.hpp"
#include "Zyphryon.Math/Geometry/Line.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Collision
{
    /// \brief Tests intersection between two circles.
    ///
    /// \param P0        The first circle.
    /// \param P1        The second circle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the circles intersect, `false` otherwise.
    Bool Intersects(ConstRef<Circle> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a circle and an axis-aligned rectangle.
    ///
    /// \param P0        The circle.
    /// \param P1        The rectangle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the circle and rectangle intersect, `false` otherwise.
    Bool Intersects(ConstRef<Circle> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a circle and a line segment.
    ///
    /// \param P0        The circle.
    /// \param P1        The line segment.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the circle and line intersect, `false` otherwise.
    Bool Intersects(ConstRef<Circle> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between two axis-aligned rectangles.
    ///
    /// \param P0        The first rectangle.
    /// \param P1        The second rectangle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the rectangles intersect, `false` otherwise.
    Bool Intersects(ConstRef<Rect> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a rectangle and a circle.
    ///
    /// \param P0        The rectangle.
    /// \param P1        The circle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the rectangle and circle intersect, `false` otherwise.
    Bool Intersects(ConstRef<Rect> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a rectangle and a line segment.
    ///
    /// \param P0        The rectangle.
    /// \param P1        The line segment.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the rectangle and line intersect, `false` otherwise.
    Bool Intersects(ConstRef<Rect> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a line segment and a rectangle.
    ///
    /// \param P0        The line segment.
    /// \param P1        The rectangle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the line and rectangle intersect, `false` otherwise.
    Bool Intersects(ConstRef<Line> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between a line segment and a circle.
    ///
    /// \param P0        The line segment.
    /// \param P1        The circle.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the line and circle intersect, `false` otherwise.
    Bool Intersects(ConstRef<Line> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold);

    /// \brief Tests intersection between two line segments.
    ///
    /// \param P0        The first line segment.
    /// \param P1        The second line segment.
    /// \param Manifold  Optional manifold pointer to receive contact data if an intersection occurs.
    /// \return `true` if the line segments intersect, `false` otherwise.
    Bool Intersects(ConstRef<Line> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold);
}