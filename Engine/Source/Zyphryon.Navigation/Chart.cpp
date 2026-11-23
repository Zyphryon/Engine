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

#include "Chart.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Reset(UInt32 Span, Real32 Rise, Real32 Climb)
    {
        mSpan     = static_cast<Real32>(Span);
        mRise     = Rise;
        mClimb    = Climb;
        mCheapest = 1.0f;
        mCount    = 0;

        mCosts.Fill(1.0f);
        mTiles.Clear();
        mSpare.Clear();
        mSlots.Clear();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Add(SInt32 X, SInt32 Z, AnyRef<Mesh> Floor)
    {
        UInt32 Slot = static_cast<UInt32>(mTiles.GetSize());

        // A tile already there gives its slot to the new one.
        if (const ConstPtr<UInt32> Found = mSlots.Find(GetKey(X, Z)))
        {
            Slot = (* Found);
        }
        else if (!mSpare.IsEmpty())
        {
            Slot = mSpare.GetBack();
            mSpare.RemoveLast();
        }
        else
        {
            ZY_ASSERT(mTiles.GetSize() <= 0xFFFF, "A chart holds at most 65536 tiles, each named in half a word");

            mTiles.Append();
        }

        Ref<Tile> Held = mTiles[Slot];
        Held.X     = X;
        Held.Z     = Z;
        Held.Floor = Move(Floor);

        Lay(Held);

        mSlots.Assign(GetKey(X, Z), Slot);

        Mend(Slot);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Remove(SInt32 X, SInt32 Z)
    {
        UInt32 Slot = 0;

        if (!mSlots.Extract(GetKey(X, Z), Slot))
        {
            return;
        }

        // The removed tile keeps its column and row so Mend can still find the tiles beside it.
        mTiles[Slot].Floor.Clear();
        mTiles[Slot].Gates.Clear();

        mSpare.Append(Slot);

        Mend(Slot);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::SetCost(UInt8 Area, Real32 Cost)
    {
        ZY_ASSERT(Cost > 0.0f, "An area costs something to cross");

        mCosts[Area] = Cost;
        mCheapest    = Cost;

        // The cheapest cost is found again, since the old cheapest may have risen.
        for (const Real32 Each : mCosts)
        {
            mCheapest = Min(mCheapest, Each);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstPtr<Mesh> Chart::GetMesh(SInt32 X, SInt32 Z) const
    {
        const ConstPtr<UInt32> Found = mSlots.Find(GetKey(X, Z));

        return Found ? AddressOf(mTiles[* Found].Floor) : nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 Chart::Snap(Ref<Vector3> Spot, Real32 Reach) const
    {
        const Vector2 Flat = Spot.GetXZ();

        UInt32  Found     = kNowhere;
        Real32  Closest   = kMaximum<Real32>;
        UInt32  Narrowest = 0;
        Vector2 Landed    = Flat;

        const SInt32 LowX  = static_cast<SInt32>(Floor((Flat.GetX() - Reach) / mSpan));
        const SInt32 LowZ  = static_cast<SInt32>(Floor((Flat.GetY() - Reach) / mSpan));
        const SInt32 HighX = static_cast<SInt32>(Floor((Flat.GetX() + Reach) / mSpan));
        const SInt32 HighZ = static_cast<SInt32>(Floor((Flat.GetY() + Reach) / mSpan));

        for (SInt32 Z = LowZ; Z <= HighZ; ++Z)
        {
            for (SInt32 X = LowX; X <= HighX; ++X)
            {
                const ConstPtr<UInt32> Slot = mSlots.Find(GetKey(X, Z));

                if (Slot == nullptr)
                {
                    continue;
                }

                ConstRef<Tile> Held = mTiles[* Slot];

                const Vector2 Origin(static_cast<Real32>(X) * mSpan, static_cast<Real32>(Z) * mSpan);

                // Only the buckets within reach are searched.
                const Real32 LocalX = Flat.GetX() - Origin.GetX();
                const Real32 LocalZ = Flat.GetY() - Origin.GetY();
                const Real32 LastX  = static_cast<Real32>(Held.Columns * kBucket) - 1.0f;
                const Real32 LastZ  = static_cast<Real32>(Held.Rows * kBucket) - 1.0f;

                const UInt32 FromX = static_cast<UInt32>(Clamp(LocalX - Reach, 0.0f, LastX)) / kBucket;
                const UInt32 FromZ = static_cast<UInt32>(Clamp(LocalZ - Reach, 0.0f, LastZ)) / kBucket;
                const UInt32 IntoX = static_cast<UInt32>(Clamp(LocalX + Reach, 0.0f, LastX)) / kBucket;
                const UInt32 IntoZ = static_cast<UInt32>(Clamp(LocalZ + Reach, 0.0f, LastZ)) / kBucket;

                for (UInt32 Row = FromZ; Row <= IntoZ; ++Row)
                {
                    for (UInt32 Column = FromX; Column <= IntoX; ++Column)
                    {
                        const UInt32 Bucket = Row * Held.Columns + Column;

                        for (UInt32 At = Held.Starts[Bucket]; At < Held.Starts[Bucket + 1]; ++At)
                        {
                            const AnyRect<UInt16> Box = Held.Boxes[Held.Items[At]];

                            // A polygon in several searched buckets is weighed only in the first.
                            const UInt32 FirstX = Max(Box.GetMinimumX() / kBucket, FromX);
                            const UInt32 FirstZ = Max(Box.GetMinimumY() / kBucket, FromZ);

                            if (FirstX != Column || FirstZ != Row)
                            {
                                continue;
                            }

                            // A polygon whose box is beyond reach, or farther than the best so far, cannot be nearer.
                            if (Flat.GetDistanceSquared((Rect(Box) + Origin).GetNearest(Flat)) > Min(Reach * Reach, Closest))
                            {
                                continue;
                            }

                            // The nearest polygon wins, counting how far the height lies outside its floor heights.
                            const ConstSpan<UInt16> Level  = Held.Floor.GetLevel(Held.Items[At]);
                            const Real32            Height = Spot.GetY();
                            const Real32            Above  = Max(0.0f, Level[0] - Height, Height - Level[1]) * mRise;

                            Vector2 Nearest;

                            const Real32 Distance = Measure(Held, Held.Items[At], Flat, Nearest);

                            if (Distance > Reach * Reach)
                            {
                                continue;
                            }

                            // On a tie, the polygon with the narrower floor heights is the surer match.
                            const UInt32 Width = static_cast<UInt32>(Level[1] - Level[0]);
                            const Real32 Apart = Distance + Above * Above;

                            if (Apart < Closest || (Apart == Closest && Width < Narrowest))
                            {
                                Closest   = Apart;
                                Narrowest = Width;
                                Found     = ((* Slot) << 16) | Held.Items[At];
                                Landed    = Nearest;
                            }
                        }
                    }
                }
            }
        }

        if (Found != kNowhere)
        {
            Spot = Vector3(Landed.GetX(), Sample(mTiles[Found >> 16], Found & 0xFFFF, Landed), Landed.GetY());
        }
        return Found;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector3 Chart::GetNearest(UInt32 Polygon, Vector3 Spot) const
    {
        Vector2 Nearest;

        Measure(mTiles[Polygon >> 16], Polygon & 0xFFFF, Spot.GetXZ(), Nearest);
        return Vector3(Nearest.GetX(), Sample(mTiles[Polygon >> 16], Polygon & 0xFFFF, Nearest), Nearest.GetY());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Chart::GetHeight(UInt32 Polygon, Vector2 Spot) const
    {
        return Sample(mTiles[Polygon >> 16], Polygon & 0xFFFF, Spot);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Mend(UInt32 Slot)
    {
        ConstRef<Tile> Held = mTiles[Slot];

        // Only the tiles from the changed slot on get new polygon indices.
        mCount = (Slot > 0) ? mTiles[Slot - 1].First + mTiles[Slot - 1].Floor.GetCount() : 0;

        for (UInt32 Index = Slot; Index < mTiles.GetSize(); ++Index)
        {
            mTiles[Index].First = mCount;

            mCount += mTiles[Index].Floor.GetCount();
        }

        // Only the edges shared with the four tiles beside it change, and a removed tile links nothing.
        for (UInt32 Edge = 0; Edge < 4; ++Edge)
        {
            const IntVector2       Step   = kAround[Edge];
            const ConstPtr<UInt32> Beside = mSlots.Find(GetKey(Held.X + Step.GetX(), Held.Z + Step.GetY()));

            Join(Slot, Edge, Beside ? (* Beside) : kNowhere);

            if (Beside != nullptr)
            {
                Join(* Beside, Edge ^ 2, Slot);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Lay(Ref<Tile> Held)
    {
        const ConstSpan<UInt16> Vertices = Held.Floor.GetVertices();
        const UInt32            Count    = Held.Floor.GetCount();

        Held.Boxes.Clear();
        Held.Gates.Clear();

        // The buckets cover the tile from its origin to the farthest corner of any polygon.
        UInt16 FarX = 0;
        UInt16 FarZ = 0;

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const ConstSpan<UInt16> Each = Held.Floor.GetPolygon(Index);

            UInt16 LowX  = Mesh::kNone;
            UInt16 LowZ  = Mesh::kNone;
            UInt16 HighX = 0;
            UInt16 HighZ = 0;

            for (UInt32 Side = 0; Side < Mesh::kSides && Each[Side] != Mesh::kNone; ++Side)
            {
                const UInt16 X = Vertices[Each[Side] * 3];
                const UInt16 Z = Vertices[Each[Side] * 3 + 2];

                LowX  = Min(LowX, X);
                LowZ  = Min(LowZ, Z);
                HighX = Max(HighX, X);
                HighZ = Max(HighZ, Z);

                // A side on the tile edge now stores its gate index in place of the edge.
                if (const UInt16 Past = Each[Mesh::kSides + Side]; Past != Mesh::kNone && Past >= Mesh::kPortal)
                {
                    // An edge at either end of x runs along z, and the other two along x.
                    const UInt32 Next  = (Side + 1 < Mesh::kSides && Each[Side + 1] != Mesh::kNone) ? Side + 1 : 0;
                    const UInt16 Mine  = (Past & 1) != 0 ? X : Z;
                    const UInt16 Other = Vertices[Each[Next] * 3 + ((Past & 1) != 0 ? 0 : 2)];
                    const UInt16 Name  = static_cast<UInt16>(Mesh::kPortal | Held.Gates.GetSize());

                    Ref<Gate> Door = Held.Gates.Append();
                    Door.Polygon = static_cast<UInt16>(Index);
                    Door.Side    = static_cast<UInt8>(Side);
                    Door.Edge    = static_cast<UInt8>(Past & 3);
                    Door.Lower   = Min(Mine, Other);
                    Door.Upper   = Max(Mine, Other);

                    Held.Floor.SetSide(Index, Side, Each[Side], Name);
                }
            }

            Held.Boxes.Append(LowX, LowZ, HighX, HighZ);

            FarX = Max(FarX, HighX);
            FarZ = Max(FarZ, HighZ);
        }

        Held.Columns = FarX / kBucket + 1;
        Held.Rows    = FarZ / kBucket + 1;

        Held.Starts.Resize(static_cast<UInt>(Held.Columns) * Held.Rows + 1);

        // Each polygon is counted into every bucket its box overlaps, then filed into the room those counts make.
        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const AnyRect<UInt16> Box = Held.Boxes[Index];

            for (UInt32 Z = Box.GetMinimumY() / kBucket; Z <= Box.GetMaximumY() / kBucket; ++Z)
            {
                for (UInt32 X = Box.GetMinimumX() / kBucket; X <= Box.GetMaximumX() / kBucket; ++X)
                {
                    ++Held.Starts[Z * Held.Columns + X];
                }
            }
        }

        for (UInt32 Index = 1; Index < Held.Starts.GetSize(); ++Index)
        {
            Held.Starts[Index] += Held.Starts[Index - 1];
        }

        Held.Items.Resize(Held.Starts[Held.Starts.GetSize() - 1]);

        // Filling from the last polygon back keeps each bucket in ascending order and leaves its start in place.
        for (UInt32 Index = Count; Index > 0; --Index)
        {
            const AnyRect<UInt16> Box = Held.Boxes[Index - 1];

            for (UInt32 Z = Box.GetMinimumY() / kBucket; Z <= Box.GetMaximumY() / kBucket; ++Z)
            {
                for (UInt32 X = Box.GetMinimumX() / kBucket; X <= Box.GetMaximumX() / kBucket; ++X)
                {
                    Held.Items[--Held.Starts[Z * Held.Columns + X]] = static_cast<UInt16>(Index - 1);
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Chart::Join(UInt32 Slot, UInt32 Edge, UInt32 Beside)
    {
        Ref<Tile> Held = mTiles[Slot];

        mLinks.Clear();

        for (UInt32 Index = 0; Index < Held.Gates.GetSize(); ++Index)
        {
            Ref<Gate>    Door  = Held.Gates[Index];
            const UInt32 First = static_cast<UInt32>(mLinks.GetSize());

            // A side on another edge keeps its links, copied to their new place.
            if (Door.Edge != Edge)
            {
                mLinks.Append(ConstSpan<Link>(Held.Links).Slice(Door.First, Door.Count));
                Door.First = First;
                continue;
            }

            Door.First = First;
            Door.Count = 0;

            if (Beside == kNowhere)
            {
                continue;
            }

            // A side links to every facing side it overlaps by more than a sliver.
            ConstRef<Tile>          Other = mTiles[Beside];
            const ConstSpan<UInt16> Mine  = Held.Floor.GetLevel(Door.Polygon);
            const Real32            Base  = static_cast<Real32>((Edge & 1) != 0 ? Held.X : Held.Z) * mSpan;

            for (UInt32 At = 0; At < Other.Gates.GetSize(); ++At)
            {
                ConstRef<Gate> Facing = Other.Gates[At];

                if (Facing.Edge != (Edge ^ 2))
                {
                    continue;
                }

                // The two floors must lie within the climb, or a floor would link to the one stacked over it.
                const ConstSpan<UInt16> Theirs = Other.Floor.GetLevel(Facing.Polygon);
                const Real32            Apart  = static_cast<Real32>(Max(0, Mine[0] - Theirs[1], Theirs[0] - Mine[1]));
                const Real32            From   = Base + Max(Door.Lower, Facing.Lower);
                const Real32            Into   = Base + Min(Door.Upper, Facing.Upper);

                if (Into - From > 0.01f && Apart <= mClimb)
                {
                    mLinks.Append((Beside << 16) | Facing.Polygon, From, Into);

                    ++Door.Count;
                }
            }
        }

        Swap(Held.Links, mLinks);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Chart::Sample(ConstRef<Tile> Held, UInt32 Polygon, Vector2 Spot) const
    {
        const ConstSpan<UInt16> Each     = Held.Floor.GetPolygon(Polygon);
        const ConstPtr<UInt16>  Vertices = Held.Floor.GetVertices().GetData();
        const ConstSpan<UInt16> Level    = Held.Floor.GetLevel(Polygon);

        // A polygon over one flat floor has that height everywhere.
        if (Level[0] == Level[1])
        {
            return Level[0];
        }

        const Vector2 Origin(static_cast<Real32>(Held.X) * mSpan, static_cast<Real32>(Held.Z) * mSpan);

        // The polygon is a fan of triangles from its first corner, and the one holding the position gives the height.
        const Vector2 Base   = GetSpot(Vertices, Each[0]);
        const Vector2 Offset = Spot - Origin - Base;
        const Real32  BaseY  = Vertices[Each[0] * 3 + 1];

        // A polygon whose corners share one height needs no fan.
        UInt32 Even = 1;

        while (Even < Mesh::kSides && Each[Even] != Mesh::kNone && Vertices[Each[Even] * 3 + 1] == BaseY)
        {
            ++Even;
        }

        Real32 Height = BaseY;
        Real32 Least  = (Even == Mesh::kSides || Each[Even] == Mesh::kNone) ? 0.0f : kMaximum<Real32>;

        // The fan stops at the triangle the position lies inside.
        for (UInt32 Side = 1; Least > 0.0f && Side + 1 < Mesh::kSides && Each[Side + 1] != Mesh::kNone; ++Side)
        {
            const Vector2 Along = GetSpot(Vertices, Each[Side]) - Base;
            const Vector2 Other = GetSpot(Vertices, Each[Side + 1]) - Base;
            const Real32  Twice = Vector2::Cross(Along, Other);

            if (Twice == 0.0f)
            {
                continue;
            }

            const Real32 First  = Vector2::Cross(Offset, Other) / Twice;
            const Real32 Second = Vector2::Cross(Along, Offset) / Twice;
            const Real32 AlongY = Vertices[Each[Side] * 3 + 1] - BaseY;
            const Real32 OtherY = Vertices[Each[Side + 1] * 3 + 1] - BaseY;
            const Real32 High   = BaseY + AlongY * First + OtherY * Second;

            // A position just outside every triangle takes the height of the closest one.
            const Real32 Outside = Max(0.0f, -First) + Max(0.0f, -Second) + Max(0.0f, First + Second - 1.0f);

            if (Outside < Least)
            {
                Least  = Outside;
                Height = High;
            }
        }
        return Clamp(Height, static_cast<Real32>(Level[0]), static_cast<Real32>(Level[1]));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Chart::Measure(ConstRef<Tile> Held, UInt32 Polygon, Vector2 Spot, Ref<Vector2> Nearest) const
    {
        const ConstSpan<UInt16> Each     = Held.Floor.GetPolygon(Polygon);
        const ConstPtr<UInt16>  Vertices = Held.Floor.GetVertices().GetData();

        const Vector2 Origin(static_cast<Real32>(Held.X) * mSpan, static_cast<Real32>(Held.Z) * mSpan);

        Array<Vector2, Mesh::kSides> Corner;
        UInt32                       Count  = 0;
        Bool                         Inside = true;

        for (; Count < Mesh::kSides && Each[Count] != Mesh::kNone; ++Count)
        {
            Corner[Count] = Origin + GetSpot(Vertices, Each[Count]);
        }

        // Polygons are wound so their inside lies right of every side.
        for (UInt32 Side = 0; Side < Count; ++Side)
        {
            Inside = Inside && !(Vector2::Cross(Corner[Side], Corner[Side + 1 < Count ? Side + 1 : 0], Spot) > 0.0f);
        }

        if (Inside)
        {
            Nearest = Spot;
            return 0.0f;
        }

        Real32 Closest = kMaximum<Real32>;

        for (UInt32 Side = 0; Side < Count; ++Side)
        {
            const Vector2 Candidate = Line(Corner[Side], Corner[Side + 1 < Count ? Side + 1 : 0]).GetNearest(Spot);
            const Real32  Apart     = Spot.GetDistanceSquared(Candidate);

            if (Apart < Closest)
            {
                Closest = Apart;
                Nearest = Candidate;
            }
        }
        return Closest;
    }
}