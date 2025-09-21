// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2025 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Collision.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Collision
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Circle> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Delta      = P1.GetCenter() - P0.GetCenter();
        const Real32  SqDistance = Delta.GetLengthSquared();
        const Real32  SumRadius  = P0.GetRadius() + P1.GetRadius();

        const Bool Result = (SqDistance <= SumRadius * SumRadius);

        if (Result && Manifold)
        {
            const Real32 Distance = Sqrt(SqDistance);
            const Vector2 Normal  = (IsAlmostZero(Distance) ? Vector2::UnitX() : Delta / Distance);

            Manifold->SetPenetration(SumRadius - Distance);
            Manifold->SetNormal(Normal);
            Manifold->AddPoint(P0.GetCenter() + Normal * P0.GetRadius());
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Circle> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Closest   = P1.GetNearest(P0.GetCenter());
        const Vector2 Delta     = Closest - P0.GetCenter();
        const Real32 SqDistance = Delta.GetLengthSquared();
        const Real32 Radius     = P0.GetRadius();

        const Bool Result = (SqDistance <= Radius * Radius);

        if (Result && Manifold)
        {
            const Real32 Distance = Sqrt(SqDistance);
            const Vector2 Normal  = (Distance > kEpsilon<Real32> ? Delta / Distance : Vector2::UnitX());

            Manifold->SetPenetration(Radius - Distance);
            Manifold->SetNormal(Normal);
            Manifold->AddPoint(Closest);
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Circle> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold)
    {
        const Vector2 AB = P1.GetEnd()    - P1.GetStart();
        const Vector2 AC = P0.GetCenter() - P1.GetStart();

        const Vector2 Closest   = P1.GetStart() + AB * Clamp(Vector2::Dot(AB, AC) / AB.GetLengthSquared(), 0.0f, 1.0f);
        const Vector2 Delta     = Closest - P0.GetCenter();
        const Real32 SqDistance = Delta.GetLengthSquared();

        const Bool Result = (SqDistance <= P0.GetRadius() * P0.GetRadius());

        if (Result && Manifold)
        {
            const Real32 Distance = Sqrt(SqDistance);

            Manifold->SetPenetration(P0.GetRadius() - Distance);

            if (IsAlmostZero(Distance))
            {
                Manifold->SetNormal(Vector2::Normalize(Vector2(AB.GetY(), -AB.GetX())));
            }
            else
            {
                Manifold->SetNormal(Delta / Distance);
            }
            Manifold->AddPoint(Closest);
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Rect> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold)
    {
        const Rect Intersection = Rect::Intersection(P0, P1);

        const Bool Result = (Intersection.IsValid());

        if (Result && Manifold)
        {
            const Real32 X1 = Intersection.GetMinimumX();
            const Real32 X2 = Intersection.GetMaximumX();
            const Real32 Y1 = Intersection.GetMinimumY();
            const Real32 Y2 = Intersection.GetMaximumY();

            const Real32 OverlapX = Intersection.GetWidth();
            const Real32 OverlapY = Intersection.GetHeight();

            if (OverlapX < OverlapY)
            {
                Manifold->SetPenetration(OverlapX);
                Manifold->SetNormal(Vector2((P0.GetCenter().GetX() < P1.GetCenter().GetX()) ? 1 : -1, 0));
            }
            else
            {
                Manifold->SetPenetration(OverlapY);
                Manifold->SetNormal(Vector2(0, (P0.GetCenter().GetY() < P1.GetCenter().GetY()) ? 1 : -1));
            }

            if (IsAlmostZero(OverlapX))
            {
                Manifold->AddPoint(Vector2(X1, (Y1 + Y2) * 0.5));
            }
            else if (IsAlmostZero(OverlapY))
            {
                Manifold->AddPoint(Vector2((X1 + X2) * 0.5, Y1));
            }
            else
            {
                if (X1 > P0.GetMinimumX() && X1 > P1.GetMinimumX())
                {
                    Manifold->AddPoint(Vector2(X1, (Y1 + Y2) * 0.5));
                }
                if (X2 < P0.GetMaximumX() && X2 < P1.GetMaximumX())
                {
                    Manifold->AddPoint(Vector2(X2, (Y1 + Y2) * 0.5));
                }
                if (Y1 > P0.GetMinimumY() && Y1 > P1.GetMinimumY())
                {
                    Manifold->AddPoint(Vector2((X1 + X2) * 0.5, Y1));
                }
                if (Y2 < P0.GetMaximumY() && Y2 < P1.GetMaximumY())
                {
                    Manifold->AddPoint(Vector2((X1 + X2) * 0.5, Y2));
                }

                if (Manifold->GetPoints().empty())
                {
                    Manifold->AddPoint(Vector2((X1 + X2) * 0.5, (Y1 + Y2) * 0.5));
                }
            }
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Rect> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold)
    {
        const Bool Result = Intersects(P1, P0, Manifold);

        if (Result && Manifold)
        {
            Manifold->SetNormal(-Manifold->GetNormal());
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Rect> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Delta = P1.GetEnd() - P1.GetStart();

        Real32 T0 = 0.0;
        Real32 T1 = 1.0;

        if (Abs(Delta.GetX()) < kEpsilon<Real32>)
        {
            if (P1.GetStart().GetX() < P0.GetMinimumX() || P1.GetStart().GetX() > P0.GetMaximumX())
            {
                return false;
            }
        }
        else
        {
            const Real32 InvDelta = 1.0 / Delta.GetX();
            Real32 Near           = (P0.GetMinimumX()  - P1.GetStart().GetX()) * InvDelta;
            Real32 Far            = (P0.GetMaximumX() - P1.GetStart().GetX()) * InvDelta;

            if (Near > Far)
            {
                Swap(Near, Far);
            }

            T0 = Max(Near, T0);
            T1 = Min(Far,  T1);
        }

        if (Abs(Delta.GetY()) < kEpsilon<Real32>)
        {
            if (P1.GetStart().GetY() < P0.GetMinimumY() || P1.GetStart().GetY() > P0.GetMaximumY())
            {
                return false;
            }
        }
        else
        {
            const Real32 InvDelta = 1.0 / Delta.GetY();
            Real32 Near           = (P0.GetMinimumY()    - P1.GetStart().GetY()) * InvDelta;
            Real32 Far            = (P0.GetMaximumY() - P1.GetStart().GetY()) * InvDelta;

            if (Near > Far)
            {
                Swap(Near, Far);
            }

            T0 = Max(Near, T0);
            T1 = Min(Far,  T1);
        }

        const Bool Result = (T0 <= T1);

        if (Result && Manifold)
        {
            const Vector2 Contact = P1.GetStart() + Delta * T0;
            Vector2 Normal;

            const Real32 DistLeft   = Abs(Contact.GetX() - P0.GetMinimumX());
            const Real32 DistRight  = Abs(Contact.GetX() - P0.GetMaximumX());
            const Real32 DistTop    = Abs(Contact.GetY() - P0.GetMinimumY());
            const Real32 DistBottom = Abs(Contact.GetY() - P0.GetMaximumY());
            const Real32 MinDist    = Min(DistLeft, DistRight, DistTop, DistBottom);

            if (Abs(MinDist - DistLeft) < kEpsilon<Real32>)
            {
                Normal = Vector2(-1.0f, 0.0f);
            }
            else if (Abs(MinDist - DistRight) < kEpsilon<Real32>)
            {
                Normal = Vector2(1.0f, 0.0f);
            }
            else if (Abs(MinDist - DistTop) < kEpsilon<Real32>)
            {
                Normal = Vector2(0.0f, -1.0f);
            }
            else if (Abs(MinDist - DistBottom) < kEpsilon<Real32>)
            {
                Normal = Vector2(0.0f, 1.0f);
            }
            else
            {
                Normal = Vector2::Normalize(P0.GetCenter() - Contact);
            }

            Manifold->SetPenetration((T1 - T0) * Delta.GetLength());
            Manifold->SetNormal(Normal);
            Manifold->AddPoint(Contact);
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Line> P0, ConstRef<Rect> P1, Ptr<Manifold> Manifold)
    {
        const Bool Result = Intersects(P1, P0, Manifold);

        if (Result && Manifold)
        {
            Manifold->SetNormal(-Manifold->GetNormal());
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Line> P0, ConstRef<Circle> P1, Ptr<Manifold> Manifold)
    {
        const Bool Result = Intersects(P1, P0, Manifold);

        if (Result && Manifold)
        {
            Manifold->SetNormal(-Manifold->GetNormal());
        }
        return Result;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Intersects(ConstRef<Line> P0, ConstRef<Line> P1, Ptr<Manifold> Manifold)
    {
        const Vector2 Dir0 = P0.GetEnd() - P0.GetStart();
        const Vector2 Dir1 = P1.GetEnd() - P1.GetStart();

        const Real32 Cross = Vector2::Cross(Dir0, Dir1);
        if (IsAlmostZero(Cross))
        {
            return false; // TODO: Implement Collinear
        }

        const Vector2 Difference = P1.GetStart() - P0.GetStart();
        const Real32 Alpha0      = Vector2::Cross(Difference, Dir1) / Cross;
        const Real32 Alpha1      = Vector2::Cross(Difference, Dir0) / Cross;

        const Bool Result = (Alpha0 >= 0.0f && Alpha0 <= 1.0f && Alpha1 >= 0.0f && Alpha1 <= 1.0f);

        if (Result && Manifold)
        {
            Manifold->SetPenetration(kEpsilon<Real32>);
            Manifold->SetNormal(Vector2::Normalize(Vector2(-Dir0.GetY(), Dir0.GetX())));
            Manifold->AddPoint(P0.GetStart() + Dir0 * Alpha0);
        }
        return Result;
    }
}