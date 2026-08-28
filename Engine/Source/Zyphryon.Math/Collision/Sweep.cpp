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

#include "Sweep.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace Math
{
    namespace
    {
        struct Interval final
        {
            Real32  Entry  = 0.0f;
            Real32  Leave  = 1.0f;
            Vector3 Normal = Vector3::Zero();
        };
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool NarrowSlab(Real32 Apart, Real32 Reach, Real32 Motion, Vector3 Facing, Ref<Interval> Result)
    {
        // A motion that never travels along the axis either lies between the faces the whole way or never does.
        if (Abs(Motion) < Sweep::kMinimumReach)
        {
            return Abs(Apart) <= Reach;
        }

        const Real32 First = (-Reach - Apart) / Motion;
        const Real32 Last  = ( Reach - Apart) / Motion;
        const Real32 Enter = Min(First, Last);
        const Real32 Exit  = Max(First, Last);

        if (Exit < Result.Entry || Enter > Result.Leave)
        {
            return false;
        }

        // Only the last face reached decides the contact, so the facing follows whichever narrowed the interval.
        if (Enter > Result.Entry)
        {
            Result.Entry  = Enter;
            Result.Normal = (Motion > 0.0f ? -Facing : Facing);
        }
        Result.Leave = Min(Result.Leave, Exit);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool NarrowRound(Vector2 Apart, Real32 Reach, Vector2 Motion, Ref<Interval> Result)
    {
        const Real32 Square = Vector2::Dot(Motion, Motion);
        const Real32 Along  = Vector2::Dot(Apart, Motion);
        const Real32 Beyond = Vector2::Dot(Apart, Apart) - Reach * Reach;

        // A motion that never travels on the ground either lies inside the circle the whole way or never does.
        if (Square < Sweep::kMinimumReach)
        {
            return Beyond <= 0.0f;
        }

        const Real32 Root = Along * Along - Square * Beyond;

        if (Root < 0.0f)
        {
            return false;
        }

        const Real32 Reached = Sqrt(Root);
        const Real32 Enter   = (-Along - Reached) / Square;
        const Real32 Exit    = (-Along + Reached) / Square;

        if (Exit < Result.Entry || Enter > Result.Leave)
        {
            return false;
        }

        if (Enter > Result.Entry)
        {
            Result.Entry  = Enter;
            Result.Normal = Vector3::FromXZ(Vector2::Normalize(Apart + Motion * Enter));
        }
        Result.Leave = Min(Result.Leave, Exit);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool NarrowGrown(Vector2 Apart, Vector2 Extent, Real32 Reach, Vector2 Motion, Ref<Interval> Result)
    {
        Real32  Entry  = Result.Leave;
        Real32  Leave  = Result.Entry;
        Vector3 Facing = Vector3::Zero();
        Bool    Found  = false;

        const auto OnPiece = [&](ConstRef<Interval> Piece)
        {
            if (Piece.Entry < Entry)
            {
                Entry  = Piece.Entry;
                Facing = Piece.Normal;
            }
            Leave = Max(Leave, Piece.Leave);
            Found = true;
        };

        for (const Vector2 Grown : {
            Vector2(Extent.GetX() + Reach, Extent.GetY()),
            Vector2(Extent.GetX(), Extent.GetY() + Reach) })
        {
            Interval Piece = Result;

            if (NarrowSlab(Apart.GetX(), Grown.GetX(), Motion.GetX(), Vector3::UnitX(), Piece)
             && NarrowSlab(Apart.GetY(), Grown.GetY(), Motion.GetY(), Vector3::UnitZ(), Piece))
            {
                OnPiece(Piece);
            }
        }

        for (const Real32 SideX : { -Extent.GetX(), Extent.GetX() })
        {
            for (const Real32 SideY : { -Extent.GetY(), Extent.GetY() })
            {
                Interval Piece = Result;

                if (NarrowRound(Apart - Vector2(SideX, SideY), Reach, Motion, Piece))
                {
                    OnPiece(Piece);
                }
            }
        }

        if (!Found)
        {
            return false;
        }

        if (Entry > Result.Entry)
        {
            Result.Entry  = Entry;
            Result.Normal = Facing;
        }
        Result.Leave = Min(Result.Leave, Leave);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold> Contact)
    {
        // A pair that already shares a spot has no face to have come in through, which is separating's business.
        if (!Met || Result.Normal.IsAlmostZero())
        {
            return false;
        }

        Contact.SetNormal(Result.Normal);
        Contact.SetTime(Max(Result.Entry, 0.0f));

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Box> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold> Contact)
    {
        const Vector3 Apart = First.GetCenter() - Second.GetCenter();
        const Vector3 Reach = First.GetExtents() + Second.GetExtents();

        Interval Result;

        const Bool Met = NarrowSlab(Apart.GetX(), Reach.GetX(), Motion.GetX(), Vector3::UnitX(), Result)
                      && NarrowSlab(Apart.GetY(), Reach.GetY(), Motion.GetY(), Vector3::UnitY(), Result)
                      && NarrowSlab(Apart.GetZ(), Reach.GetZ(), Motion.GetZ(), Vector3::UnitZ(), Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold> Contact)
    {
        const Vector3 Apart = First.GetCenter() - Second.GetCenter();

        Interval Result;

        const Bool Met =
            NarrowSlab(Apart.GetY(), First.GetExtent() + Second.GetExtent(), Motion.GetY(), Vector3::UnitY(), Result)
         && NarrowRound(Apart.GetXZ(), First.GetRadius() + Second.GetRadius(), Motion.GetXZ(), Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold> Contact)
    {
        const Vector3 Apart   = First.GetCenter() - Second.GetCenter();
        const Vector3 Extents = Second.GetExtents();

        Interval Result;

        // The box is the one that grows, and it grows by the cylinder's radius.
        const Bool Met =
            NarrowSlab(Apart.GetY(), First.GetExtent() + Extents.GetY(), Motion.GetY(), Vector3::UnitY(), Result)
         && NarrowGrown(Apart.GetXZ(), Extents.GetXZ(), First.GetRadius(), Motion.GetXZ(), Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold> Contact)
    {
        const Vector3 Apart   = First.GetCenter() - Second.GetCenter();
        const Vector3 Extents = First.GetExtents();

        Interval Result;

        // The pair grows into the same shape either way round, so it is the box's extents that are grown again.
        const Bool Met =
            NarrowSlab(Apart.GetY(), Extents.GetY() + Second.GetExtent(), Motion.GetY(), Vector3::UnitY(), Result)
         && NarrowGrown(Apart.GetXZ(), Extents.GetXZ(), Second.GetRadius(), Motion.GetXZ(), Result);

        return Close(Result, Met, Contact);
    }
}