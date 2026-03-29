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

    Bool Intersects(Circle P0, Circle P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Delta           = P1.GetCenter() - P0.GetCenter();
        const Real32  DistanceSquared = Delta.GetLengthSquared();
        const Real32  SumRadius       = P0.GetRadius() + P1.GetRadius();

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
            Manifold->AddContact(P0.GetCenter() + Normal * P0.GetRadius());
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(Circle P0, Rect P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Center          = P0.GetCenter();
        const Vector2 Closest         = P1.GetNearest(Center);
        const Vector2 Delta           = Center - Closest;
        const Real32  DistanceSquared = Delta.GetLengthSquared();
        const Real32  Radius          = P0.GetRadius();

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
                Real32 Side = Closest.GetX() - P1.GetMinimumX();
                Normal      = Vector2(-1.0f, 0.0f);

                if (const Real32 Right = P1.GetMaximumX() - Closest.GetX(); Right < Side)
                {
                    Side   = Right;
                    Normal = Vector2::UnitX();
                }
                if (const Real32 Bottom = Closest.GetY() - P1.GetMinimumY(); Bottom < Side)
                {
                    Side   = Bottom;
                    Normal = Vector2(0.0f, -1.0f);
                }
                if (const Real32 Top = P1.GetMaximumY() - Closest.GetY(); Top < Side)
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

    Bool Intersects(Rect P0, Rect P1, Ptr<Manifold> Manifold)
    {
        const Rect Intersection = Rect::Intersection(P0, P1);

        if (Intersection.IsAlmostZero())
        {
            return false;
        }

        if (Manifold)
        {
            const Real32 OverlapX = Intersection.GetWidth();
            const Real32 OverlapY = Intersection.GetHeight();

            const Vector2 Center0 = P0.GetCenter();
            const Vector2 Center1 = P1.GetCenter();

            Vector2 Normal;

            if (OverlapX < OverlapY)
            {
                Normal = (Center1.GetX() >= Center0.GetX()) ? Vector2::UnitX() : Vector2(-1.0f, 0.0f);

                const Real32 XFace = (Normal.GetX() > 0.0f) ? P1.GetMinimumX() : P1.GetMaximumX();

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

                const Real32 YFace = (Normal.GetY() > 0.0f) ? P1.GetMinimumY() : P1.GetMaximumY();

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