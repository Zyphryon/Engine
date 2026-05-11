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

#include "Intersect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Physics
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(Circle First, Circle Second, Ptr<Manifold> Manifold)
    {
        const Vector2 Delta           = Second.GetCenter() - First.GetCenter();
        const Real32  DistanceSquared = Delta.GetLengthSquared();
        const Real32  SumRadius       = First.GetRadius() + Second.GetRadius();

        if (DistanceSquared > SumRadius * SumRadius)
        {
            return false;
        }

        if (Manifold)
        {
            const Real32  Distance = Sqrt(DistanceSquared);
            const Vector2 Normal   = (IsAlmostZero(Distance) ? Vector2::UnitY() : Delta / Distance);

            Manifold->SetPenetration(SumRadius - Distance);
            Manifold->SetNormal(Normal);
            Manifold->AddContact(First.GetCenter() + Normal * First.GetRadius());
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(Circle First, Rect Second, Ptr<Manifold> Manifold)
    {
        const Vector2 Center          = First.GetCenter();
        const Vector2 Closest         = Second.GetNearest(Center);
        const Vector2 Delta           = Center - Closest;
        const Real32  DistanceSquared = Delta.GetLengthSquared();
        const Real32  Radius          = First.GetRadius();

        if (DistanceSquared > Radius * Radius)
        {
            return false;
        }

        if (Manifold)
        {
            Vector2 Normal;
            Real32  Penetration;

            if (const Real32 Distance = Sqrt(DistanceSquared); !IsAlmostZero(Distance))
            {
                Normal      = Delta / Distance;
                Penetration = Radius - Distance;
            }
            else
            {
                Real32 Side = Closest.GetX() - Second.GetMinimumX();
                Normal      = Vector2(-1.0f, 0.0f);

                if (const Real32 Right = Second.GetMaximumX() - Closest.GetX(); Right < Side)
                {
                    Side   = Right;
                    Normal = Vector2::UnitX();
                }
                if (const Real32 Bottom = Closest.GetY() - Second.GetMinimumY(); Bottom < Side)
                {
                    Side   = Bottom;
                    Normal = Vector2(0.0f, -1.0f);
                }
                if (const Real32 Top = Second.GetMaximumY() - Closest.GetY(); Top < Side)
                {
                    Side   = Top;
                    Normal = Vector2::UnitY();
                }
                Penetration = Radius + Side;
            }

            Manifold->SetPenetration(Penetration);
            Manifold->SetNormal(Normal);
            Manifold->AddContact(Center - Normal * Radius);
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(Rect First, Rect Second, Ptr<Manifold> Manifold)
    {
        if (!First.Test(Second))
        {
            return false;
        }

        if (Manifold)
        {
            const Rect Intersection = Rect::Intersection(First, Second);

            const Real32 OverlapX = Intersection.GetWidth();
            const Real32 OverlapY = Intersection.GetHeight();

            const Vector2 Center0 = First.GetCenter();
            const Vector2 Center1 = Second.GetCenter();

            Vector2 Normal;

            if (OverlapX < OverlapY)
            {
                Normal = (Center1.GetX() >= Center0.GetX()) ? Vector2::UnitX() : Vector2(-1.0f, 0.0f);

                const Real32 XFace = (Normal.GetX() > 0.0f) ? Second.GetMinimumX() : Second.GetMaximumX();

                Manifold->SetPenetration(OverlapX);
                Manifold->SetNormal(Normal);
                Manifold->AddContact(Vector2(XFace, Intersection.GetMinimumY()));

                if (!IsAlmostEqual(Intersection.GetMinimumY(), Intersection.GetMaximumY()))
                {
                    Manifold->AddContact(Vector2(XFace, Intersection.GetMaximumY()));
                }
            }
            else
            {
                Normal = (Center1.GetY() >= Center0.GetY()) ? Vector2::UnitY() : Vector2(0.0f, -1.0f);

                const Real32 YFace = (Normal.GetY() > 0.0f) ? Second.GetMinimumY() : Second.GetMaximumY();

                Manifold->SetPenetration(OverlapY);
                Manifold->SetNormal(Normal);
                Manifold->AddContact(Vector2(Intersection.GetMinimumX(), YFace));

                if (!IsAlmostEqual(Intersection.GetMinimumX(), Intersection.GetMaximumX()))
                {
                    Manifold->AddContact(Vector2(Intersection.GetMaximumX(), YFace));
                }
            }
        }
        return true;
    }
}