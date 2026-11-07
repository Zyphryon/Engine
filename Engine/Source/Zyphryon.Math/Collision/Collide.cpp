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

inline namespace ZyMath
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Box> First, ConstRef<Box> Second, Ref<Manifold3D> Contact)
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
            Contact = Manifold3D(Vector3(Apart.GetX() < 0.0f ? -1.0f : 1.0f, 0.0f, 0.0f), 0.0f, AlongX);
        }
        else if (AlongY <= AlongZ)
        {
            Contact = Manifold3D(Vector3(0.0f, Apart.GetY() < 0.0f ? -1.0f : 1.0f, 0.0f), 0.0f, AlongY);
        }
        else
        {
            Contact = Manifold3D(Vector3(0.0f, 0.0f, Apart.GetZ() < 0.0f ? -1.0f : 1.0f), 0.0f, AlongZ);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Ref<Manifold3D> Contact)
    {
        const Real32 Standing = Shared(First.GetBottom(), First.GetTop(), Second.GetBottom(), Second.GetTop());

        if (Standing <= 0.0f)
        {
            return false;
        }

        Manifold2D Ground;

        if (!Test(First.GetXZ(), Second.GetXZ(), Ground))
        {
            return false;
        }
        return Escapes(Ground, Standing, First.GetCenter().GetY() < Second.GetCenter().GetY(), Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Ref<Manifold3D> Contact)
    {
        const Real32 Standing = Shared(First.GetBottom(), First.GetTop(), Second.GetMinimumY(), Second.GetMaximumY());

        if (Standing <= 0.0f)
        {
            return false;
        }

        Manifold2D Ground;

        if (!Test(First.GetXZ(), Second.GetXZ(), Ground))
        {
            return false;
        }
        return Escapes(Ground, Standing, First.GetCenter().GetY() < Second.GetCenter().GetY(), Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Ref<Manifold3D> Contact)
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

    Bool Collide::Test(ConstRef<Shape3D> First, ConstRef<Shape3D> Second, Ref<Manifold3D> Contact)
    {
        return First.Visit([&]<typename Left>(ConstRef<Left> P0)
        {
            return Second.Visit([&]<typename Right>(ConstRef<Right> P1)
            {
                return Test(P0, P1, Contact);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Rect> First, ConstRef<Rect> Second, Ref<Manifold2D> Contact)
    {
        const Vector2 Apart = First.GetCenter() - Second.GetCenter();
        const Vector2 Reach = (First.GetSize() + Second.GetSize()) * 0.5f;

        const Real32 AlongX = Reach.GetX() - Abs(Apart.GetX());
        const Real32 AlongY = Reach.GetY() - Abs(Apart.GetY());

        if (AlongX <= 0.0f || AlongY <= 0.0f)
        {
            return false;
        }

        // The way out is the axis it is least deeply into, so the push is the shortest one that frees it.
        if (AlongX <= AlongY)
        {
            Contact = Manifold2D(Vector2(Apart.GetX() < 0.0f ? -1.0f : 1.0f, 0.0f), 0.0f, AlongX);
        }
        else
        {
            Contact = Manifold2D(Vector2(0.0f, Apart.GetY() < 0.0f ? -1.0f : 1.0f), 0.0f, AlongY);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Circle> First, ConstRef<Circle> Second, Ref<Manifold2D> Contact)
    {
        const Vector2 Apart = First.GetCenter() - Second.GetCenter();
        const Real32  Reach = First.GetRadius() + Second.GetRadius();
        const Real32  Span  = Apart.GetLength();

        if (Span > Reach)
        {
            return false;
        }

        // Two centers on one spot have no way apart of their own, so they part along x.
        const Vector2 Facing = IsAlmostZero(Span) ? Vector2(1.0f, 0.0f) : Apart * (1.0f / Span);

        Contact = Manifold2D(Facing, 0.0f, Reach - Span);
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Circle> First, ConstRef<Rect> Second, Ref<Manifold2D> Contact)
    {
        const Vector2 Center = First.GetCenter();
        const Vector2 Apart  = Center - Second.GetNearest(Center);
        const Real32  Span   = Apart.GetLength();

        if (Span > First.GetRadius())
        {
            return false;
        }

        // A center standing inside the rectangle has no way of its own to leave by, so it takes the nearest side.
        if (IsAlmostZero(Span))
        {
            const Real32 Left  = Center.GetX() - Second.GetMinimumX();
            const Real32 Right = Second.GetMaximumX() - Center.GetX();
            const Real32 Near  = Center.GetY() - Second.GetMinimumY();
            const Real32 Far   = Second.GetMaximumY() - Center.GetY();
            const Real32 Least = Min(Min(Left, Right), Min(Near, Far));

            const Vector2 Facing = (Least == Left)  ? Vector2(-1.0f, 0.0f)
                                 : (Least == Right) ? Vector2( 1.0f, 0.0f)
                                 : (Least == Near)  ? Vector2( 0.0f, -1.0f)
                                                    : Vector2( 0.0f,  1.0f);

            Contact = Manifold2D(Facing, 0.0f, Least + First.GetRadius());
        }
        else
        {
            Contact = Manifold2D(Apart * (1.0f / Span), 0.0f, First.GetRadius() - Span);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Test(ConstRef<Rect> First, ConstRef<Circle> Second, Ref<Manifold2D> Contact)
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

    Bool Collide::Test(ConstRef<Shape2D> First, ConstRef<Shape2D> Second, Ref<Manifold2D> Contact)
    {
        return First.Visit([&]<typename Left>(ConstRef<Left> P0)
        {
            return Second.Visit([&]<typename Right>(ConstRef<Right> P1)
            {
                return Test(P0, P1, Contact);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Collide::Shared(Real32 FirstBottom, Real32 FirstTop, Real32 SecondBottom, Real32 SecondTop)
    {
        return Min(FirstTop, SecondTop) - Max(FirstBottom, SecondBottom);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Collide::Escapes(ConstRef<Manifold2D> Ground, Real32 Standing, Bool Under, Ref<Manifold3D> Contact)
    {
        if (Ground.GetDepth() <= Standing)
        {
            Contact = Manifold3D(Vector3::FromXZ(Ground.GetNormal()), 0.0f, Ground.GetDepth());
        }
        else
        {
            Contact = Manifold3D(Vector3(0.0f, Under ? -1.0f : 1.0f, 0.0f), 0.0f, Standing);
        }
        return true;
    }
}