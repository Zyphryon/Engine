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
#include "Zyphryon.Math/Geometry/Circle.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Physics
{
    /// \brief Tests for intersection between two circles and generates a contact manifold if they intersect.
    ///
    /// \param First    The first geometric shape.
    /// \param Second   The second geometric shape.
    /// \param Manifold Output parameter to store the contact manifold if an intersection occurs.
    /// \return `true` if the shapes intersect, otherwise `false`.
    Bool Intersects(Circle First, Circle Second, Ptr<Manifold> Manifold);

    /// \brief Tests for intersection between a circle and a rectangle and generates a contact manifold if they intersect.
    ///
    /// \param First    The first geometric shape.
    /// \param Second   The second geometric shape.
    /// \param Manifold Output parameter to store the contact manifold if an intersection occurs.
    /// \return `true` if the shapes intersect, otherwise `false`.
    Bool Intersects(Circle First, Rect Second, Ptr<Manifold> Manifold);

    /// \brief Tests for intersection between two rectangles and generates a contact manifold if they intersect.
    ///
    /// \param First    The first geometric shape.
    /// \param Second   The second geometric shape.
    /// \param Manifold Output parameter to store the contact manifold if an intersection occurs.
    /// \return `true` if the shapes intersect, otherwise `false`.
    Bool Intersects(Rect First, Rect Second, Ptr<Manifold> Manifold);

    /// \brief Tests for intersection between a rectangle and a circle and generates a contact manifold if they intersect.
    ///
    /// \param First    The first geometric shape.
    /// \param Second   The second geometric shape.
    /// \param Manifold Output parameter to store the contact manifold if an intersection occurs.
    /// \return `true` if the shapes intersect, otherwise `false`.
    ZYPHRYON_INLINE Bool Intersects(Rect First, Circle Second, Ptr<Manifold> Manifold)
    {
        const Bool Result = Intersects(Second, First, Manifold);

        if (Result && Manifold)
        {
            Manifold->SetNormal(-Manifold->GetNormal());
        }
        return Result;
    }
}