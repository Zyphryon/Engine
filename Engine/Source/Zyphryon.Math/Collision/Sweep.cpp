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
#include "Collide.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Box> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold3D> Contact)
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

    Bool Sweep::Test(ConstRef<Cylinder> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold3D> Contact)
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

    Bool Sweep::Test(ConstRef<Cylinder> First, ConstRef<Box> Second, Vector3 Motion, Ref<Manifold3D> Contact)
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

    Bool Sweep::Test(ConstRef<Box> First, ConstRef<Cylinder> Second, Vector3 Motion, Ref<Manifold3D> Contact)
    {
        // The swapped pair, moving the other way, meets at the same time on the opposite face.
        if (Test(Second, First, -Motion, Contact))
        {
            Contact.SetNormal(-Contact.GetNormal());
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Shape3D> First, ConstRef<Shape3D> Second, Vector3 Motion, Ref<Manifold3D> Contact)
    {
        return First.Visit([&]<typename Left>(ConstRef<Left> P0)
        {
            return Second.Visit([&]<typename Right>(ConstRef<Right> P1)
            {
                return Test(P0, P1, Motion, Contact);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Sweep::Cast(
        ConstRef<Shape3D> First,
        ConstRef<Shape3D> Second,
        Vector3           Motion,
        Real32            Skin,
        Ref<Vector3>      Normal)
    {
        return Free(First, Second, Motion, Skin, Normal);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Rect> First, ConstRef<Rect> Second, Vector2 Motion, Ref<Manifold2D> Contact)
    {
        const Vector2 Apart = First.GetCenter() - Second.GetCenter();
        const Vector2 Reach = (First.GetSize() + Second.GetSize()) * 0.5f;

        Interval Result;

        // The plane is narrowed as the ground is, its y riding on the ground's z.
        const Bool Met = NarrowSlab(Apart.GetX(), Reach.GetX(), Motion.GetX(), Vector3::UnitX(), Result)
                      && NarrowSlab(Apart.GetY(), Reach.GetY(), Motion.GetY(), Vector3::UnitZ(), Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Circle> First, ConstRef<Circle> Second, Vector2 Motion, Ref<Manifold2D> Contact)
    {
        Interval Result;

        const Bool Met = NarrowRound(
            First.GetCenter() - Second.GetCenter(), First.GetRadius() + Second.GetRadius(), Motion, Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Circle> First, ConstRef<Rect> Second, Vector2 Motion, Ref<Manifold2D> Contact)
    {
        Interval Result;

        // The rectangle is the one that grows, and it grows by the circle's radius.
        const Bool Met = NarrowGrown(
            First.GetCenter() - Second.GetCenter(), Second.GetSize() * 0.5f, First.GetRadius(), Motion, Result);

        return Close(Result, Met, Contact);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Rect> First, ConstRef<Circle> Second, Vector2 Motion, Ref<Manifold2D> Contact)
    {
        // The swapped pair, moving the other way, meets at the same time on the opposite face.
        if (Test(Second, First, -Motion, Contact))
        {
            Contact.SetNormal(-Contact.GetNormal());
            return true;
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Test(ConstRef<Shape2D> First, ConstRef<Shape2D> Second, Vector2 Motion, Ref<Manifold2D> Contact)
    {
        return First.Visit([&]<typename Left>(ConstRef<Left> P0)
        {
            return Second.Visit([&]<typename Right>(ConstRef<Right> P1)
            {
                return Test(P0, P1, Motion, Contact);
            });
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Sweep::Cast(
        ConstRef<Shape2D> First,
        ConstRef<Shape2D> Second,
        Vector2           Motion,
        Real32            Skin,
        Ref<Vector2>      Normal)
    {
        return Free(First, Second, Motion, Skin, Normal);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::NarrowSlab(Real32 Apart, Real32 Reach, Real32 Motion, Vector3 Facing, Ref<Interval> Result)
    {
        // A motion that never travels along the axis lies strictly between the faces the whole way or never does.
        if (Abs(Motion) < kMinimumReach)
        {
            return Abs(Apart) < Reach;
        }

        const Real32 First = (-Reach - Apart) / Motion;
        const Real32 Last  = ( Reach - Apart) / Motion;
        const Real32 Enter = Min(First, Last);
        const Real32 Exit  = Max(First, Last);

        if (Exit < Result.Entry || Enter > Result.Leave)
        {
            return false;
        }

        // Only the last face reached decides the contact, and a face met at the start counts for one resting on it.
        if (Enter >= Result.Entry)
        {
            Result.Entry  = Enter;
            Result.Normal = (Motion > 0.0f ? -Facing : Facing);
        }
        Result.Leave = Min(Result.Leave, Exit);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::NarrowRound(Vector2 Apart, Real32 Reach, Vector2 Motion, Ref<Interval> Result)
    {
        const Real32 Square = Vector2::Dot(Motion, Motion);
        const Real32 Along  = Vector2::Dot(Apart, Motion);
        const Real32 Beyond = Vector2::Dot(Apart, Apart) - Reach * Reach;

        // A motion that never travels on the ground either lies inside the circle the whole way or never does.
        if (Square < kMinimumReach * kMinimumReach)
        {
            return Beyond < 0.0f;
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

        if (Enter >= Result.Entry)
        {
            Result.Entry  = Enter;
            Result.Normal = Vector3::FromXZ(Vector2::Normalize(Apart + Motion * Enter));
        }
        Result.Leave = Min(Result.Leave, Exit);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::NarrowGrown(Vector2 Apart, Vector2 Extent, Real32 Reach, Vector2 Motion, Ref<Interval> Result)
    {
        Real32  Entry  = Result.Leave;
        Real32  Leave  = Result.Entry;
        Vector3 Facing = Vector3::Zero();
        Bool    Found  = false;

        // The motion is in the grown rectangle from the first piece it enters until the last piece it leaves.
        const auto Merge  = [&](ConstRef<Interval> Piece)
        {
            if (Piece.Entry < Entry)
            {
                Entry  = Piece.Entry;
                Facing = Piece.Normal;
            }
            Leave = Max(Leave, Piece.Leave);
            Found = true;
        };

        const auto Side   = [&](Real32 ReachX, Real32 ReachY)
        {
            Interval Piece = Result;

            if (NarrowSlab(Apart.GetX(), ReachX, Motion.GetX(), Vector3::UnitX(), Piece)
             && NarrowSlab(Apart.GetY(), ReachY, Motion.GetY(), Vector3::UnitZ(), Piece))
            {
                Merge(Piece);
            }
        };

        const auto Corner = [&](Real32 X, Real32 Y)
        {
            Interval Piece = Result;

            if (NarrowRound(Apart - Vector2(X, Y), Reach, Motion, Piece))
            {
                Merge(Piece);
            }
        };

        Side(Extent.GetX() + Reach, Extent.GetY());
        Side(Extent.GetX(), Extent.GetY() + Reach);

        Corner(-Extent.GetX(), -Extent.GetY());
        Corner(-Extent.GetX(),  Extent.GetY());
        Corner( Extent.GetX(), -Extent.GetY());
        Corner( Extent.GetX(),  Extent.GetY());

        if (!Found)
        {
            return false;
        }

        if (Entry >= Result.Entry)
        {
            Result.Entry  = Entry;
            Result.Normal = Facing;
        }
        Result.Leave = Min(Result.Leave, Leave);

        return Result.Entry <= Result.Leave;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Sweep::Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold3D> Contact)
    {
        // A pair that already shares a spot has no face to have come in through, which is Collide's business.
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

    Bool Sweep::Close(ConstRef<Interval> Result, Bool Met, Ref<Manifold2D> Contact)
    {
        if (!Met || Result.Normal.IsAlmostZero())
        {
            return false;
        }

        Contact.SetNormal(Result.Normal.GetXZ());
        Contact.SetTime(Max(Result.Entry, 0.0f));

        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<typename Form, typename Vector>
    Real32 Sweep::Free(ConstRef<Form> First, ConstRef<Form> Second, Vector Motion, Real32 Skin, Ref<Vector> Normal)
    {
        if (AnyManifold<Vector> Touch; Collide::Test(First, Second, Touch))
        {
            const Vector Facing = Touch.GetNormal();

            if (Vector::Dot(Motion, Facing) >= 0.0f)
            {
                return 1.0f;
            }
            Normal = Facing;
            return 0.0f;
        }

        AnyManifold<Vector> Contact;

        if (Test(First, Second, Motion, Contact))
        {
            // The motion stops a skin short, so a shape resting against a face can still be asked to slide along it.
            const Real32 Closing = -Vector::Dot(Motion, Contact.GetNormal());
            const Real32 Backoff = (Closing > kMinimumReach ? Skin / Closing : 0.0f);

            Normal = Contact.GetNormal();

            return Clamp(Contact.GetTime() - Backoff, 0.0f, 1.0f);
        }
        return 1.0f;
    }
}