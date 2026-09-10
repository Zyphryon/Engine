// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Collide.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Box> First, ConstRef<Box> Second, Ref<Manifold> Contact)
    {
        const Vector3 Apart = First.GetCenter() - Second.GetCenter();
        const Vector3 Reach = First.GetExtents() + Second.GetExtents();

        const Real32 AlongX = Reach.GetX() - Abs(Apart.GetX());
        const Real32 AlongY = Reach.GetY() - Abs(Apart.GetY());
        const Real32 AlongZ = Reach.GetZ() - Abs(Apart.GetZ());

        if (AlongX <= 0.0f || AlongY <= 0.0f || AlongZ <= 0.0f)
        {
            return false;
        }

        // The way out is the axis it is least deeply into, so the push is the shortest one that frees it.
        if (AlongX <= AlongY && AlongX <= AlongZ)
        {
            Contact = Manifold(Vector3(Apart.GetX() < 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f), 0.0f, AlongX);
        }
        else if (AlongY <= AlongZ)
        {
            Contact = Manifold(Vector3(0.0f, Apart.GetY() < 0.0f ? -1.0f : 1.0f, 0.0f), 0.0f, AlongY);
        }
        else
        {
            Contact = Manifold(Vector3(0.0f, 0.0f, Apart.GetZ() < 0.0f ? -1.0f : 1.0f), 0.0f, AlongZ);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Ref<Manifold> Contact)
    {
        const Real32 Standing = Shared(First.GetBottom(), First.GetTop(), Second.GetBottom(), Second.GetTop());

        if (Standing <= 0.0f)
        {
            return false;
        }

        const Vector2 Apart = First.GetCenter().GetXZ() - Second.GetCenter().GetXZ();
        const Real32  Reach = First.GetRadius() + Second.GetRadius();
        const Real32  Span  = Apart.GetLength();

        if (Span > Reach)
        {
            return false;
        }
        return Escapes(Apart, Span, Reach - Span, Standing,
            First.GetCenter().GetY() < Second.GetCenter().GetY(), Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Ref<Manifold> Contact)
    {
        const Real32 Standing = Shared(
            First.GetBottom(), First.GetTop(), Second.GetMinimumY(), Second.GetMaximumY());

        if (Standing <= 0.0f)
        {
            return false;
        }

        const Vector2 Center(First.GetCenter().GetX(), First.GetCenter().GetZ());
        const Vector2 Corner(
            Clamp(Center.GetX(), Second.GetMinimumX(), Second.GetMaximumX()),
            Clamp(Center.GetY(), Second.GetMinimumZ(), Second.GetMaximumZ()));

        const Vector2 Apart = Center - Corner;
        const Real32  Span  = Apart.GetLength();

        if (Span > First.GetRadius())
        {
            return false;
        }

        const Bool Under = First.GetCenter().GetY() < Second.GetCenter().GetY();

        // A centre standing inside the footprint has no way of its own to leave by, so it takes the nearest side.
        if (IsAlmostZero(Span))
        {
            const Real32 Left  = Center.GetX() - Second.GetMinimumX();
            const Real32 Right = Second.GetMaximumX() - Center.GetX();
            const Real32 Near  = Center.GetY() - Second.GetMinimumZ();
            const Real32 Far   = Second.GetMaximumZ() - Center.GetY();
            const Real32 Least = Min(Min(Left, Right), Min(Near, Far));

            const Vector2 Facing = (Least == Left)  ? Vector2(-1.0f, 0.0f)
                                 : (Least == Right) ? Vector2( 1.0f, 0.0f)
                                 : (Least == Near)  ? Vector2( 0.0f, -1.0f)
                                                    : Vector2( 0.0f,  1.0f);

            return Escapes(Facing, 1.0f, Least + First.GetRadius(), Standing, Under, Contact);
        }
        return Escapes(Apart, Span, First.GetRadius() - Span, Standing, Under, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Ref<Manifold> Contact)
    {
        if (Test(Second, First, Contact))
        {
            Contact.SetNormal(-Contact.GetNormal());
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Collide::Shared(Real32 FirstBottom, Real32 FirstTop, Real32 SecondBottom, Real32 SecondTop)
    {
        return Min(FirstTop, SecondTop) - Max(FirstBottom, SecondBottom);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Escapes(
        Vector2 Apart, Real32 Span, Real32 Ground, Real32 Standing, Bool Under, Ref<Manifold> Contact)
    {
        if (Ground <= Standing)
        {
            const Vector2 Facing = IsAlmostZero(Span) ? Vector2(1.0f, 0.0f) : Apart * (1.0f / Span);

            Contact = Manifold(Vector3(Facing.GetX(), 0.0f, Facing.GetY()), 0.0f, Ground);
        }
        else
        {
            Contact = Manifold(Vector3(0.0f, Under ? -1.0f : 1.0f, 0.0f), 0.0f, Standing);
        }
        return true;
    }
}