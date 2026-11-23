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

#include "Baker.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Bake(ConstRef<Field> Source, ConstRef<Recipe> Recipe, Ref<Mesh> Output)
    {
        mFloors  = Source.GetFloors().GetData();
        mWidth   = Source.GetWidth();
        mDepth   = Source.GetDepth();
        mBorder  = Recipe.Border;
        mSpanX   = mWidth - 2 * mBorder;
        mSpanZ   = mDepth - 2 * mBorder;
        mClimb   = Recipe.Climb;
        mStretch = static_cast<SInt32>(Recipe.Extent);
        mLayered = Recipe.Headroom != 0 && IsStacked(Source, Recipe.Headroom);

        // Erosion keeps only cells within a climb of their four neighbours, so steps need checking only with no radius.
        mSteep   = (Recipe.Radius == 0);

        // A stacked region spans a height band narrow enough to keep floors a body's height apart in separate regions.
        mBand    = (Recipe.Headroom > Recipe.Climb) ? (Recipe.Headroom - Recipe.Climb - 1u) / 2u + 1u : 1u;

        Output.Clear();

        mShapes.Clear();
        mGroups.Clear();
        mChain.Clear();
        mKeys.Clear();

        // Slabs that only block the cells they stand on are folded into the ground, which bakes as plain ground.
        mAreas = Source.GetAreas().GetData();

        if (Recipe.Headroom != 0 && !mLayered && Source.GetSlabs().GetSize() != 0)
        {
            Shut(Source);
        }

        // Rises are compared squared, so the slope is capped where its square still fits.
        const Real32 Steep = Min(Recipe.Slope, kSteep);
        const SInt32 Cap   = static_cast<SInt32>(Steep) + 1;
        const SInt32 Most  = static_cast<SInt32>(static_cast<Real64>(Steep) * Steep);

        // Stacked floors are read link by link, and plain ground cell by cell.
        if (mLayered)
        {
            Stack(Source, Recipe.Headroom);

            if (Recipe.Slope > 0.0f)
            {
                Pitch(Cap, Most);
            }
            Wear(Recipe.Radius);
            Cut();
        }
        else
        {
            if (Recipe.Slope > 0.0f)
            {
                Grade(Cap, Most);
            }
            Erode(Recipe.Radius);
            Partition();
        }

        mCorners.Resize((mSpanX + 1) * (mSpanZ + 1));

        // Every region is its own island until its outline shows which regions it joins.
        mOuter.Resize(mRegions.GetSize() + 1);

        for (UInt32 Index = 0; Index <= mRegions.GetSize(); ++Index)
        {
            mGroups.Append(static_cast<UInt16>(Index));
        }

        for (UInt32 Index = 0; Index < mRegions.GetSize(); ++Index)
        {
            mRegion = static_cast<UInt16>(Index + 1);

            if (mLayered)
            {
                Outline<true>(mRegions[Index]);
            }
            else
            {
                Outline<false>(mRegions[Index]);
            }
            Simplify(Recipe.Tolerance);
            Triangulate(Output);
        }

        if (Recipe.Island > 0)
        {
            Prune(Recipe.Island);
        }

        Pair(Output);
        Merge(Output);
        Emit(Output);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Grade(SInt32 Cap, SInt32 Most)
    {
        const SInt32 Wide  = static_cast<SInt32>(mWidth);
        const SInt32 Tall  = static_cast<SInt32>(mDepth);
        const SInt32 Reach = mClimb + 1;

        mGraded.Resize(static_cast<UInt>(mWidth) * mDepth);
        mRises.Resize(static_cast<UInt>(mWidth) * 2 + 1);

        // The rises along a row lie between its cells, with none before the first or past the last.
        const Ptr<SInt32> Side = mRises.GetData();
        const Ptr<SInt32> Back = Side + Wide + 1;

        for (SInt32 Y = 0; Y < Tall; ++Y)
        {
            // The last row stands in for the row after it, so the ground reads level past the edge.
            const ConstPtr<UInt8>  Row   = mAreas + Y * Wide;
            const ConstPtr<UInt8>  Upper = (Y + 1 < Tall) ? Row + Wide : Row;
            const ConstPtr<UInt16> Floor = mFloors + Y * Wide;
            const ConstPtr<UInt16> Over  = (Y + 1 < Tall) ? Floor + Wide : Floor;
            const Ptr<UInt8>       Into  = mGraded.GetData() + Y * Wide;

            for (SInt32 X = 1; X < Wide; ++X)
            {
                Side[X] = GetRise(Floor[X - 1], Floor[X], Min(Row[X - 1], Row[X]), Reach);
            }

            // A cell is weighed by the rises onto it and off it, and its area kept by a minimum so the loop vectorizes.
            for (SInt32 X = 0; X < Wide; ++X)
            {
                const SInt32 Ahead  = GetRise(Floor[X], Over[X], Min(Row[X], Upper[X]), Reach);
                const SInt32 Across = GetSlope(Side[X], Side[X + 1], Cap);
                const SInt32 Along  = GetSlope(Back[X], Ahead, Cap);
                const SInt32 Area   = Row[X];
                const SInt32 Keep   = Clamp(Most + 1 - (Across * Across + Along * Along), 0, 1);

                Into[X] = static_cast<UInt8>(Min(Area, Keep * 255));
                Back[X] = Ahead;
            }
        }

        mAreas = mGraded.GetData();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Erode(UInt32 Radius)
    {
        const SInt32          Pitch = static_cast<SInt32>(mWidth) + 2;
        const SInt32          Wide  = static_cast<SInt32>(mWidth);
        const SInt32          Tall  = static_cast<SInt32>(mDepth);
        const ConstPtr<UInt8> Areas = mAreas;

        mOpen.Resize(static_cast<UInt>(Pitch) * (mDepth + 2));

        const Ptr<UInt8> Origin = mOpen.GetData() + Pitch + 1;

        // A body with no radius stands wherever the floor is.
        if (Radius == 0)
        {
            for (SInt32 Y = 0; Y < Tall; ++Y)
            {
                Copy(Origin + Y * Pitch, static_cast<UInt>(Wide), Areas + Y * Wide);
            }
            return;
        }

        mKept.Resize(mOpen.GetSize());

        // The distance to the nearest cell that is not clear counts two per side and three per diagonal, in one byte.
        const UInt32 Body  = Min(Radius, 127u);
        const SInt32 Far   = static_cast<SInt32>(Min(Body * 2, 252u));
        const UInt8  Least = static_cast<UInt8>(Min(Body * 2, 253u));
        const SInt32 Reach = mClimb + 1;
        const SInt32 Last  = Wide - 1;

        // Passes swap between two planes, starting on the one that leaves the last result in the open cells.
        Ptr<UInt8> From = (Body % 2 == 1) ? Origin : mKept.GetData() + Pitch + 1;
        Ptr<UInt8> Into = (Body % 2 == 1) ? mKept.GetData() + Pitch + 1 : Origin;

        // A cell is clear when it and its four neighbours are floor, all within a climb of it.
        for (SInt32 Y = 1; Y + 1 < Tall; ++Y)
        {
            const ConstPtr<UInt8>  Row   = Areas + Y * Wide;
            const ConstPtr<UInt8>  Lower = Row - Wide;
            const ConstPtr<UInt8>  Upper = Row + Wide;
            const ConstPtr<UInt16> Floor = mFloors + Y * Wide;
            const ConstPtr<UInt16> Under = Floor - Wide;
            const ConstPtr<UInt16> Over  = Floor + Wide;
            const Ptr<UInt8>       Clear = From + Y * Pitch;

            for (SInt32 X = 1; X < Last; ++X)
            {
                const SInt32 Stood = Floor[X];
                const SInt32 Along = Max(Abs(Floor[X - 1] - Stood), Abs(Floor[X + 1] - Stood));
                const SInt32 Steep = Max(Along, Abs(Under[X] - Stood), Abs(Over[X] - Stood));
                const SInt32 Flat  = Min(Row[X], Row[X - 1], Row[X + 1], Lower[X], Upper[X]);

                Clear[X] = static_cast<UInt8>(Clamp(Min(Flat, Reach - Steep) * 256, 0, Far));
            }
        }

        // Each pass spreads the distance one cell farther over the whole plane, the padding between rows included.
        for (UInt32 Pass = 1; Pass < Body; ++Pass)
        {
            const ConstPtr<UInt8> Row   = From;
            const ConstPtr<UInt8> Above = From - Pitch;
            const ConstPtr<UInt8> Below = From + Pitch;
            const Ptr<UInt8>      Next  = Into;

            for (SInt32 At = 0, Cells = Pitch * Tall - 2; At < Cells; ++At)
            {
                const UInt8 Side   = Min(Row[At - 1], Row[At + 1], Above[At], Below[At]);
                const UInt8 Corner = Min(Above[At - 1], Above[At + 1], Below[At - 1], Below[At + 1]);

                Next[At] = Min(Row[At], static_cast<UInt8>(Side + 2), static_cast<UInt8>(Corner + 3));
            }
            Swap(From, Into);
        }

        // Cells closer to a wall than the radius are blocked, and the rest keep their area.
        for (SInt32 Y = 0; Y < Tall; ++Y)
        {
            const ConstPtr<UInt8> Row  = Areas + Y * Wide;
            const Ptr<UInt8>      Open = Origin + Y * Pitch;

            for (SInt32 X = 0; X < Wide; ++X)
            {
                Open[X] = static_cast<UInt8>(Row[X] & (Open[X] >= Least ? 0xFF : 0));
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Partition()
    {
        const SInt32 SpanX  = static_cast<SInt32>(mSpanX);
        const SInt32 SpanZ  = static_cast<SInt32>(mSpanZ);
        const SInt32 Border = static_cast<SInt32>(mBorder);
        const SInt32 Climb  = mClimb;

        mRuns.Clear();
        mRegions.Clear();
        mLabels.Resize(static_cast<UInt>(mSpanX + 2) * (mSpanZ + 2));

        UInt32 Before = 0;

        for (SInt32 Z = 0; Z < SpanZ; ++Z)
        {
            const UInt32           Start = static_cast<UInt32>(mRuns.GetSize());
            const ConstPtr<UInt8>  Open  = mOpen.GetData() + GetOpen(Border, Z + Border);
            const ConstPtr<UInt16> Floor = mFloors + static_cast<UInt32>(Z + Border) * mWidth + mBorder;

            // A run lasts while its cells share one area, up to where a region is cut.
            for (SInt32 X = 0; X < SpanX; )
            {
                const SInt32 From  = X;
                const UInt8  Area  = Open[X];
                const SInt32 Limit = (mStretch == 0) ? SpanX : Min(SpanX, From - From % mStretch + mStretch);

                X = Extend(Open, From, Limit);

                // A step too high to climb ends the run early.
                for (SInt32 At = From + 1; mSteep && At < X; ++At)
                {
                    if (Abs(Floor[At] - Floor[At - 1]) > mClimb)
                    {
                        X = At;
                    }
                }

                if (Area != Field::kBlocked)
                {
                    mRuns.Append(static_cast<UInt16>(From), static_cast<UInt16>(X), Area);
                }
            }

            const UInt32 End = static_cast<UInt32>(mRuns.GetSize());

            // Every run is matched with the runs of the row before that it joins.
            const ConstPtr<UInt16> Prior = Floor - mWidth;

            for (UInt32 Old = Before, New = Start; Z > 0 && Old < Start && New < End; )
            {
                Ref<Run> Older = mRuns[Old];
                Ref<Run> Newer = mRuns[New];

                const SInt32 First = Max(Older.Lower, Newer.Lower);
                const SInt32 Last  = Min(Older.Upper, Newer.Upper);

                if (First < Last && Older.Area == Newer.Area)
                {
                    SInt32 Joined = Last - First;

                    for (SInt32 X = First, Until = mSteep ? Last : First; X < Until; ++X)
                    {
                        Joined -= Clamp(Abs(Floor[X] - Prior[X]) - Climb, 0, 1);
                    }

                    if (Joined != 0)
                    {
                        ++Older.Below;
                        ++Newer.Above;

                        Newer.Parent = Old;
                        Newer.Whole  = Newer.Whole && (Joined == Last - First);
                    }
                }

                if (Older.Upper < Newer.Upper)
                {
                    ++Old;
                }
                else
                {
                    ++New;
                }
            }

            Carry(Start, End, Z);

            Before = Start;
        }

        // The open border cells are what a side on the tile edge opens onto.
        for (SInt32 X = -1; X <= SpanX; ++X)
        {
            mLabels[GetLabel(X, -1)]    = (mOpen[GetOpen(X + Border, Border - 1)] != Field::kBlocked)     ? kBeyond : kWall;
            mLabels[GetLabel(X, SpanZ)] = (mOpen[GetOpen(X + Border, Border + SpanZ)] != Field::kBlocked) ? kBeyond : kWall;
        }

        for (SInt32 Z = 0; Z < SpanZ; ++Z)
        {
            mLabels[GetLabel(-1, Z)]    = (mOpen[GetOpen(Border - 1, Z + Border)] != Field::kBlocked)     ? kBeyond : kWall;
            mLabels[GetLabel(SpanX, Z)] = (mOpen[GetOpen(Border + SpanX, Z + Border)] != Field::kBlocked) ? kBeyond : kWall;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    template<Bool Stacked>
    void Baker::Outline(ConstRef<Region> Region)
    {
        mRaw.Clear();
        mOrder.Clear();
        mWay = 0;

        for (UInt32 At = Region.First; At != kNowhere; At = mRuns[At].Next)
        {
            mOrder.Append(At);
        }

        const SInt32 Rows = static_cast<SInt32>(mOrder.GetSize());
        const SInt32 Top  = Region.Top;

        // Up the low side, keeping the region on the right.
        for (SInt32 Row = 0; Row < Rows; ++Row)
        {
            const SInt32 Left = mRuns[mOrder[Row]].Lower;

            Face<Stacked>(Top, Row, Left, Left + 1, 0);

            if (Row + 1 < Rows)
            {
                const SInt32 Next = mRuns[mOrder[Row + 1]].Lower;

                Face<Stacked>(Top, Row, Left, Next, 1);
                Face<Stacked>(Top, Row + 1, Next, Left, 3);
            }
        }

        // Along the far side.
        Face<Stacked>(Top, Rows - 1, mRuns[mOrder[Rows - 1]].Lower, mRuns[mOrder[Rows - 1]].Upper, 1);

        // Down the high side.
        for (SInt32 Row = Rows - 1; Row >= 0; --Row)
        {
            const SInt32 Right = mRuns[mOrder[Row]].Upper;

            Face<Stacked>(Top, Row, Right - 1, Right, 2);

            if (Row > 0)
            {
                const SInt32 Next = mRuns[mOrder[Row - 1]].Upper;

                Face<Stacked>(Top, Row - 1, Right, Next, 1);
                Face<Stacked>(Top, Row, Next, Right, 3);
            }
        }

        // Back along the near side.
        Face<Stacked>(Top, 0, mRuns[mOrder[0]].Lower, mRuns[mOrder[0]].Upper, 3);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Simplify(Real32 Tolerance)
    {
        const UInt32 Count = static_cast<UInt32>(mRaw.GetSize());

        mLoop.Clear();
        mKept.Resize(Count);
        mOrder.Clear();

        // A corner stays when either side next to it is not a wall, since the polygon past that side ends there too.
        for (UInt32 Index = 0, Before = Count - 1; Index < Count; Before = Index++)
        {
            if (mRaw[Index].Past != kWall || mRaw[Before].Past != kWall)
            {
                mKept[Index] = 1;
                mOrder.Append(Index);
            }
        }

        // An outline that is all wall keeps its lowest and highest corners to straighten between.
        if (mOrder.IsEmpty())
        {
            UInt32 Low  = 0;
            UInt32 High = 0;

            for (UInt32 Index = 1; Index < Count; ++Index)
            {
                ConstRef<Point> Each = mRaw[Index];

                if (Each.X < mRaw[Low].X || (Each.X == mRaw[Low].X && Each.Z < mRaw[Low].Z))
                {
                    Low = Index;
                }
                if (Each.X > mRaw[High].X || (Each.X == mRaw[High].X && Each.Z > mRaw[High].Z))
                {
                    High = Index;
                }
            }

            mKept[Low]  = 1;
            mKept[High] = 1;

            mOrder.Append(Min(Low, High));
            mOrder.Append(Max(Low, High));
        }

        // Each wall between two kept corners keeps its corner farthest off the line, while that one is out of tolerance.
        const Real32 Limit = Tolerance * Tolerance;
        const UInt32 Kept  = static_cast<UInt32>(mOrder.GetSize());

        mPending.Clear();

        for (UInt32 Index = 0, Before = Kept - 1; Index < Kept; Before = Index++)
        {
            // Two kept corners with none between them leave nothing to straighten.
            if (mOrder[Index] - mOrder[Before] + (Index == 0 ? Count : 0) > 1)
            {
                mPending.Append(mOrder[Before]);
                mPending.Append(mOrder[Index]);
            }
        }

        while (!mPending.IsEmpty())
        {
            const UInt32 Into = mPending.GetBack();
            mPending.RemoveLast();
            const UInt32 From = mPending.GetBack();
            mPending.RemoveLast();

            const Real32 OriginX = static_cast<Real32>(mRaw[From].X);
            const Real32 OriginZ = static_cast<Real32>(mRaw[From].Z);
            const Real32 AlongX  = static_cast<Real32>(mRaw[Into].X) - OriginX;
            const Real32 AlongZ  = static_cast<Real32>(mRaw[Into].Z) - OriginZ;
            const Real32 Length  = AlongX * AlongX + AlongZ * AlongZ;

            Real32 Farthest = Limit;
            UInt32 Found    = kNowhere;

            for (UInt32 Index = (From + 1 == Count) ? 0 : From + 1; Index != Into; Index = (Index + 1 == Count) ? 0 : Index + 1)
            {
                const Real32 OffsetX = static_cast<Real32>(mRaw[Index].X) - OriginX;
                const Real32 OffsetZ = static_cast<Real32>(mRaw[Index].Z) - OriginZ;
                const Real32 Dot     = OffsetX * AlongX + OffsetZ * AlongZ;
                const Real32 Share   = Length > 0.0f ? Clamp(Dot / Length, 0.0f, 1.0f) : 0.0f;
                const Real32 AwayX   = OffsetX - AlongX * Share;
                const Real32 AwayZ   = OffsetZ - AlongZ * Share;

                if (const Real32 Apart = AwayX * AwayX + AwayZ * AwayZ; Apart > Farthest)
                {
                    Farthest = Apart;
                    Found    = Index;
                }
            }

            if (Found != kNowhere)
            {
                mKept[Found] = 1;

                mPending.Append(From);
                mPending.Append(Found);
                mPending.Append(Found);
                mPending.Append(Into);
            }
        }

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            if (mKept[Index] != 0)
            {
                mLoop.Append(mRaw[Index]);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Triangulate(Ref<Mesh> Output)
    {
        const UInt32 Count = static_cast<UInt32>(mLoop.GetSize());

        if (Count < 3)
        {
            return;
        }

        // Every corner starts linked to its two neighbours, with what lies past the side leaving it.
        mRing.Clear();

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            const UInt32 Before = (Index == 0 ? Count : Index) - 1;
            const UInt32 After  = (Index + 1 == Count) ? 0 : Index + 1;
            const UInt16 Corner = Weld(mLoop[Index], Output);

            mRing.Append(static_cast<UInt16>(Before), static_cast<UInt16>(After), Corner, mLoop[Index].Past);
        }

        // An outline that is convex and fits in one mesh polygon is kept whole.
        Bool   Whole = (Count <= Mesh::kSides);
        UInt32 Swing = 0;

        for (UInt32 Index = 0; Whole && Index < Count; ++Index)
        {
            ConstRef<Point> First  = mLoop[mRing[Index].Before];
            ConstRef<Point> Second = mLoop[Index];
            ConstRef<Point> Third  = mLoop[mRing[Index].After];

            const SInt32 FromX = Second.X - First.X;
            const SInt32 IntoX = Third.X - Second.X;
            const SInt32 Bend  = Turn(First, Second, Third);

            // An outline that goes round once turns back along x twice, and one that crosses itself turns back more.
            Swing += ((FromX < 0) != (IntoX < 0)) ? 1 : 0;
            Whole  = Bend < 0 || (Bend == 0 && FromX * IntoX + (Second.Z - First.Z) * (Third.Z - Second.Z) > 0);
        }

        if (Whole && Swing <= 2)
        {
            Clip(0, Count, mRing[Count - 1].Past);
            return;
        }

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            mRing[Index].Span = GetSpan(Index);
        }

        UInt32 First = 0;

        for (UInt32 Left = Count; Left > 3; --Left)
        {
            // The ear with the shortest diagonal goes first, which keeps the triangles from growing thin.
            UInt32 Best     = kNowhere;
            UInt32 Shortest = kNowhere;

            for (UInt32 Index = 0; Index < Count; ++Index)
            {
                if (mRing[Index].Span < Shortest)
                {
                    Shortest = mRing[Index].Span;
                    Best     = Index;
                }
            }

            if (Best == kNowhere)
            {
                return;
            }

            // The ear is clipped, and its two neighbours are linked across the diagonal and measured again.
            const Knot Gone = mRing[Best];

            Clip(Gone.Before, 3, mRegion);

            mRing[Best].Span         = kNowhere;
            mRing[Gone.Before].After = Gone.After;
            mRing[Gone.Before].Past  = mRegion;
            mRing[Gone.After].Before = Gone.Before;

            First = (Best == First) ? Gone.After : First;

            // The last three corners are clipped as they stand, so they need no measuring.
            if (Left > 4)
            {
                mRing[Gone.Before].Span = GetSpan(Gone.Before);
                mRing[Gone.After].Span  = GetSpan(Gone.After);
            }
        }

        const UInt32 Second = mRing[First].After;
        const UInt32 Third  = mRing[Second].After;

        if (Turn(mLoop[First], mLoop[Second], mLoop[Third]) < 0)
        {
            Clip(First, 3, mRing[Third].Past);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Prune(UInt32 Least)
    {
        mSizes.Resize(mRegions.GetSize() + 1);

        for (UInt32 Index = 1; Index <= mRegions.GetSize(); ++Index)
        {
            const UInt16 Group = GetRoot(mGroups, static_cast<UInt16>(Index));

            mSizes[Group] += mRegions[Index - 1].Cells;
            mOuter[Group]  = mOuter[Group] | mOuter[Index];
        }

        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            const UInt16 Group = GetRoot(mGroups, mShapes[Index].Region);

            if (mSizes[Group] < Least && mOuter[Group] == 0)
            {
                mShapes[Index].Count = 0;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Pair(ConstRef<Mesh> Output)
    {
        ZY_ASSERT(mShapes.GetSize() <= Mesh::kNone, "A tile cuts into at most 65535 pieces, so bake it smaller");

        const ConstSpan<UInt16> Vertices = Output.GetVertices();

        mEdges.Clear();
        mFirst.Clear();
        mFirst.Fill(kNowhere, Output.GetCorners());

        // A side is filed by the polygon walking it from its lesser corner, and claimed by the one walking it back.
        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            ConstRef<Shape> Each = mShapes[Index];

            for (UInt32 Side = 0; Side < Each.Count; ++Side)
            {
                const UInt16 From = Each.Flank[Side].Corner;
                const UInt16 Into = Each.GetSide(Side + 1).Corner;
                const UInt16 Past = Each.Flank[Side].Past;

                // Only a side with a region past it has a polygon to share it with.
                if (From < Into && Past != kWall && Past != kBeyond)
                {
                    const SInt32 AlongX = Vertices[Into * 3]     - Vertices[From * 3];
                    const SInt32 AlongZ = Vertices[Into * 3 + 2] - Vertices[From * 3 + 2];

                    Ref<Edge> Made = mEdges.Append();
                    Made.Lower    = From;
                    Made.Upper    = Into;
                    Made.Owner[0] = static_cast<UInt16>(Index);
                    Made.Side     = static_cast<UInt8>(Side);
                    Made.Length   = static_cast<UInt32>(AlongX * AlongX + AlongZ * AlongZ);
                    Made.Next     = mFirst[From];

                    mFirst[From] = static_cast<UInt32>(mEdges.GetSize() - 1);
                }
            }
        }

        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            Ref<Shape> Each = mShapes[Index];

            for (UInt32 Side = 0; Side < Each.Count; ++Side)
            {
                const UInt16 From = Each.Flank[Side].Corner;
                const UInt16 Into = Each.GetSide(Side + 1).Corner;
                const UInt16 Past = Each.Flank[Side].Past;

                if (From > Into && Past != kWall && Past != kBeyond)
                {
                    for (UInt32 At = mFirst[Into]; At != kNowhere; At = mEdges[At].Next)
                    {
                        Ref<Edge>  Held  = mEdges[At];
                        Ref<Shape> Other = mShapes[Held.Owner[0]];

                        if (Held.Upper != From || Held.Owner[1] != Mesh::kNone)
                        {
                            continue;
                        }

                        // A side is shared with the polygon whose side runs between the same two regions the other way.
                        if (Other.Region == Past && Other.Flank[Held.Side].Past == Each.Region)
                        {
                            Held.Owner[1]               = static_cast<UInt16>(Index);
                            Other.Flank[Held.Side].Twin = static_cast<UInt16>(Index + 1);
                            Each.Flank[Side].Twin       = static_cast<UInt16>(Held.Owner[0] + 1);
                            break;
                        }
                    }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Merge(ConstRef<Mesh> Output)
    {
        const ConstSpan<UInt16> Vertices = Output.GetVertices();

        // Only a side shared by two polygons of one area can be merged across, so the rest are dropped.
        UInt32 Left = 0;

        for (UInt32 Index = 0; Index < mEdges.GetSize(); ++Index)
        {
            ConstRef<Edge> Each = mEdges[Index];

            if (Each.Owner[1] == Mesh::kNone)
            {
                continue;
            }

            ConstRef<Region> Near = mRegions[mShapes[Each.Owner[0]].Region - 1];
            ConstRef<Region> Far  = mRegions[mShapes[Each.Owner[1]].Region - 1];

            // Polygons in different height bands stay apart, as their regions were cut to.
            if (Near.Area == Far.Area && Near.Tier == Far.Tier)
            {
                mEdges[Left++] = Each;
            }
        }

        Sort(mEdges.GetData(), Left, [](ConstRef<Edge> First, ConstRef<Edge> Second)
        {
            return First.Length > Second.Length;
        });

        // Each polygon names the one it was merged into, itself until then.
        mMerged.Clear();

        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            mMerged.Append(static_cast<UInt16>(Index));
        }

        // A merge can allow one refused before it, so the refused sides get a second pass.
        for (UInt32 Pass = 0; Pass < 2; ++Pass)
        {
            UInt32 Kept = 0;

            for (UInt32 At = 0; At < Left; ++At)
            {
                ConstRef<Edge> Each = mEdges[At];

                const UInt16 First  = GetRoot(mMerged, Each.Owner[0]);
                const UInt16 Second = GetRoot(mMerged, Each.Owner[1]);

                if (First == Second)
                {
                    continue;
                }

                if (Fuse(First, Second, Each, Vertices))
                {
                    mMerged[Second] = First;
                }
                else
                {
                    mEdges[Kept++] = Each;
                }
            }
            Left = Kept;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::Fuse(UInt16 First, UInt16 Second, ConstRef<Edge> Edge, ConstSpan<UInt16> Vertices)
    {
        Ref<Shape> Near = mShapes[First];
        Ref<Shape> Far  = mShapes[Second];

        const SInt32 NearCount = Near.Count;
        const SInt32 FarCount  = Far.Count;

        SInt32 Head = Near.Find(Edge.Lower, Edge.Upper);
        SInt32 Back = Far.Find(Edge.Upper, Edge.Lower);

        if (Head < 0 || Back < 0)
        {
            return false;
        }

        // Two polygons may share several sides in a row, and are merged across all of them at once.
        const SInt32 Most  = Min(NearCount, FarCount) - 1;
        SInt32       Tail  = Head + 1;
        SInt32       Front = Back + 1;

        // The run carries on over shared sides and stops at a wall that merely ends on the same corners.
        while (Tail - Head < Most && Near.GetSide(Tail + 1).Corner == Far.GetSide(Back - 1).Corner)
        {
            if (!IsShared(Near.GetSide(Tail), Second))
            {
                break;
            }
            ++Tail;
            --Back;
        }
        while (Tail - Head < Most && Near.GetSide(Head - 1).Corner == Far.GetSide(Front + 1).Corner)
        {
            if (!IsShared(Near.GetSide(Head - 1), Second))
            {
                break;
            }
            --Head;
            ++Front;
        }

        const SInt32 Total = NearCount + FarCount - 2 * (Tail - Head);

        if (Total < 3 || Total > static_cast<SInt32>(Mesh::kSides))
        {
            return false;
        }

        // The corner at each end of the run must turn the way a polygon is wound, or go straight on.
        if (!Holds(Vertices, Near.GetSide(Head - 1).Corner, Near.GetSide(Head).Corner, Far.GetSide(Front + 1).Corner))
        {
            return false;
        }
        if (!Holds(Vertices, Far.GetSide(Back - 1).Corner, Near.GetSide(Tail).Corner, Near.GetSide(Tail + 1).Corner))
        {
            return false;
        }

        // Every corner kept brings along the side leaving it, with what lies past that side.
        Array<Flank, Mesh::kSides> Joined;
        UInt8                      Count = 0;

        for (SInt32 Index = Tail; Index < Head + NearCount; ++Index)
        {
            Joined[Count++] = Near.GetSide(Index);
        }
        for (SInt32 Index = Front; Index < Back + FarCount; ++Index)
        {
            Joined[Count++] = Far.GetSide(Index);
        }

        Near.Flank = Joined;
        Near.Count = Count;
        Near.Low   = Min(Near.Low, Far.Low);
        Near.High  = Max(Near.High, Far.High);
        Far.Count  = 0;
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Emit(Ref<Mesh> Output)
    {
        // The polygons merged away are skipped, so each one left gets the index the mesh will hold it at.
        UInt32 Live = 0;

        mNames.Clear();

        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            mNames.Append(static_cast<UInt16>(Live));

            Live += (mShapes[Index].Count >= 3) ? 1 : 0;
        }

        Output.Lay(Live);

        const ConstSpan<UInt16> Vertices = Output.GetVertices();

        for (UInt32 Index = 0; Index < mShapes.GetSize(); ++Index)
        {
            ConstRef<Shape> Each = mShapes[Index];
            const UInt32    Slot = mNames[Index];

            if (Each.Count < 3)
            {
                continue;
            }

            Output.SetArea(Slot, mRegions[Each.Region - 1].Area);
            Output.SetLevel(Slot, Each.Low, Each.High);

            for (UInt32 Side = 0; Side < Each.Count; ++Side)
            {
                ConstRef<Flank> Held = Each.Flank[Side];
                UInt16          Past = Mesh::kNone;

                if (Held.Twin != 0)
                {
                    Past = mNames[GetRoot(mMerged, Held.Twin - 1)];
                }
                else if (Held.Past == kBeyond)
                {
                    // A side on the tile edge runs with the tile on its right, so its direction tells which edge.
                    const UInt16 Into   = Each.GetSide(Side + 1).Corner;
                    const SInt32 AlongX = Vertices[Into * 3]     - Vertices[Held.Corner * 3];
                    const SInt32 AlongZ = Vertices[Into * 3 + 2] - Vertices[Held.Corner * 3 + 2];
                    const UInt32 Way    = (AlongX != 0 ? 1 : 0) | (AlongX < 0 || AlongZ < 0 ? 2 : 0);

                    Past = static_cast<UInt16>(Mesh::kPortal | Way);
                }

                Output.SetSide(Slot, Side, Held.Corner, Past);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool Baker::IsStacked(ConstRef<Field> Source, SInt32 Height) const
    {
        const ConstSpan<Field::Slab> Slabs = Source.GetSlabs();

        for (UInt32 Index = 0; Index < Slabs.GetSize(); ++Index)
        {
            ConstRef<Field::Slab> Each = Slabs[Index];

            // A top to stand on, or ground with headroom under a slab, is a floor plain ground cannot hold.
            if (Each.Area != Field::kBlocked || Each.Bottom - mFloors[Each.Cell] >= Height)
            {
                return true;
            }
        }
        return false;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Shut(ConstRef<Field> Source)
    {
        const ConstSpan<Field::Slab> Slabs = Source.GetSlabs();

        mShut.Clear();
        mShut.Append(Source.GetAreas());

        // A slab wholly under the ground changes nothing, and any other blocks its cell.
        for (UInt32 Index = 0; Index < Slabs.GetSize(); ++Index)
        {
            if (ConstRef<Field::Slab> Each = Slabs[Index]; Each.Top > mFloors[Each.Cell])
            {
                mShut[Each.Cell] = Field::kBlocked;
            }
        }

        mAreas = mShut.GetData();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Stack(ConstRef<Field> Source, SInt32 Height)
    {
        const ConstSpan<Field::Slab> Slabs = Source.GetSlabs();
        const ConstSpan<UInt8>       Areas = Source.GetAreas();
        const UInt32                 Cells = mWidth * mDepth;

        // Until its floors are built a cell holds its first slab, and each slab the next one over the same cell.
        mStacks.Resize(Cells + 1);
        mOrder.Resize(Slabs.GetSize());
        mStoreys.Clear();

        for (UInt32 Index = static_cast<UInt32>(Slabs.GetSize()); Index > 0; --Index)
        {
            mOrder[Index - 1] = Exchange(mStacks[Slabs[Index - 1].Cell], Index);
        }

        UInt32 Cell = 0;

        for (UInt32 Y = 0; Y < mDepth; ++Y)
        {
            for (UInt32 X = 0; X < mWidth; ++X, ++Cell)
            {
                const UInt32 First = static_cast<UInt32>(mStoreys.GetSize());
                const UInt32 Head  = Exchange(mStacks[Cell], First);
                const SInt32 Soil  = mFloors[Cell];

                // The ground and then every slab top over it are added as floors, each kept only when clear.
                mStoreys.Append(X, Y, Soil, 0, Areas[Cell]);

                Settle(Slabs, Head, 0, First, Height);

                for (UInt32 At = Head; At != 0; At = mOrder[At - 1])
                {
                    if (ConstRef<Field::Slab> Each = Slabs[At - 1]; Each.Top > Soil)
                    {
                        mStoreys.Append(X, Y, Each.Top, 0, Each.Area);

                        Settle(Slabs, Head, At, First, Height);
                    }
                }

                const UInt32 Last = static_cast<UInt32>(mStoreys.GetSize());

                // The floors of a cell holding more than one are marked as piled.
                for (UInt32 At = First; Last - First > 1 && At < Last; ++At)
                {
                    mStoreys[At].Piled = true;
                }

                // The floors of a cell are joined to those of the cells built before it.
                if (X > 0)
                {
                    Join(First, Last, Cell - 1, 0);
                }
                if (Y > 0)
                {
                    Join(First, Last, Cell - mWidth, 3);
                }
            }
        }
        mStacks[Cells] = static_cast<UInt32>(mStoreys.GetSize());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Pitch(SInt32 Cap, SInt32 Most)
    {
        const UInt32 Count = static_cast<UInt32>(mStoreys.GetSize());

        // A floor is weighed by the floors it joins, and counts as level toward any side with no link.
        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            Ref<Storey>      Each = mStoreys[Index];
            Array<SInt32, 4> Rise;

            for (UInt32 Way = 0; Way < 4; ++Way)
            {
                Rise[Way] = (Each.Link[Way] != kNowhere) ? mStoreys[Each.Link[Way]].Floor - Each.Floor : 0;
            }

            const SInt32 Across = GetSlope(-Rise[0], Rise[2], Cap);
            const SInt32 Along  = GetSlope(-Rise[3], Rise[1], Cap);

            if (Across * Across + Along * Along > Most)
            {
                Each.Area = Field::kBlocked;
            }
        }

        // Blocked floors are unlinked once all are weighed, so their neighbours erode away from them as from a wall.
        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            Ref<Storey> Each = mStoreys[Index];

            if (Each.Area != Field::kBlocked)
            {
                continue;
            }

            for (UInt32 Way = 0; Way < 4; ++Way)
            {
                if (const UInt32 Beside = Exchange(Each.Link[Way], kNowhere); Beside != kNowhere)
                {
                    mStoreys[Beside].Link[Way ^ 2] = kNowhere;
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Wear(UInt32 Radius)
    {
        const UInt32 Count = mStoreys.GetSize();

        // A radius of one blocks every floor that is not joined on all four sides.
        for (UInt32 Index = 0; Radius == 1 && Index < Count; ++Index)
        {
            if (Ref<Storey> Each = mStoreys[Index]; !Each.IsSurrounded())
            {
                Each.Area = Field::kBlocked;
            }
        }

        if (Radius < 2)
        {
            return;
        }

        // A floor joined on all four sides starts at the largest distance, and the rest at zero.
        mKept.Resize(Count);

        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            mKept[Index] = mStoreys[Index].IsSurrounded() ? 255 : 0;
        }

        // A radius of two only checks the eight floors round each one, with no distance carried.
        if (Radius == 2)
        {
            for (UInt32 Index = 0; Index < Count; ++Index)
            {
                Ref<Storey> Each = mStoreys[Index];
                UInt32      Near = mKept[Index];

                // A floor joined on all four sides checks its four neighbours, and through them the four diagonal ones.
                if (Near != 0)
                {
                    Near = mKept[Each.Link[0]] & mKept[Each.Link[1]] & mKept[Each.Link[2]] & mKept[Each.Link[3]];
                }
                if (Near != 0)
                {
                    Near = mKept[mStoreys[Each.Link[0]].Link[3]] & mKept[mStoreys[Each.Link[1]].Link[0]];
                    Near = Near & mKept[mStoreys[Each.Link[2]].Link[1]] & mKept[mStoreys[Each.Link[3]].Link[2]];
                }

                if (Near == 0)
                {
                    Each.Area = Field::kBlocked;
                }
            }
            return;
        }

        // The distance spreads over the links forward and then back, two per side and three per diagonal.
        for (UInt32 Index = 0; Index < Count; ++Index)
        {
            if (const UInt32 Near = mKept[Index]; Near != 0)
            {
                mKept[Index] = static_cast<UInt8>(Min(Near, GetNear(Index, 0), GetNear(Index, 3)));
            }
        }

        // On the way back every floor closer to a wall than the radius is blocked.
        for (UInt32 Index = Count; Index-- > 0; )
        {
            UInt32 Near = mKept[Index];

            if (Near != 0)
            {
                Near         = Min(Near, GetNear(Index, 2), GetNear(Index, 1));
                mKept[Index] = static_cast<UInt8>(Near);
            }
            if (Near < Radius * 2)
            {
                mStoreys[Index].Area = Field::kBlocked;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Baker::Cut()
    {
        const SInt32 SpanX  = static_cast<SInt32>(mSpanX);
        const SInt32 SpanZ  = static_cast<SInt32>(mSpanZ);
        const SInt32 Border = static_cast<SInt32>(mBorder);

        mRuns.Clear();
        mRegions.Clear();

        for (SInt32 Z = 0; Z < SpanZ; ++Z)
        {
            const UInt32 Start = mRuns.GetSize();
            const UInt32 Row   = static_cast<UInt32>(Z + Border) * mWidth + mBorder;

            // An open floor with no run starts one, which carries on through the floors joined toward greater x.
            for (UInt32 Head = mStacks[Row]; Head < mStacks[Row + SpanX]; ++Head)
            {
                ConstRef<Storey> Each = mStoreys[Head];

                if (Each.Area == Field::kBlocked || Each.Run != kNowhere)
                {
                    continue;
                }

                const UInt32 New   = mRuns.GetSize();
                const SInt32 Lower = Each.X - Border;
                const SInt32 Limit = (mStretch == 0) ? SpanX : Min(SpanX, (Lower / mStretch + 1) * mStretch);

                SInt32 Upper  = Lower;
                UInt16 Low    = 0xFFFF;
                UInt16 High   = 0;
                UInt32 Last   = kNowhere;
                UInt32 Parent = kNowhere;
                UInt8  Above  = 0;
                SInt32 Joined = 0;

                for (UInt32 At = Head; At != kNowhere; )
                {
                    Ref<Storey>  Own   = mStoreys[At];
                    const UInt32 Prior = Own.Link[3];
                    const UInt32 Next  = Own.Link[2];

                    Own.Run = New;
                    Low     = Min(Low, Own.Floor);
                    High    = Max(High, Own.Floor);

                    // The runs of the row before that it joins are counted as it passes them.
                    if (Z == 0 || Prior == kNowhere || !IsAlike(mStoreys[Prior], Own))
                    {
                        Last = kNowhere;
                    }
                    else
                    {
                        if (const UInt32 Held = mStoreys[Prior].Run; Held != Last)
                        {
                            ++mRuns[Held].Below;
                            ++Above;

                            Parent = Held;
                            Last   = Held;
                        }
                        ++Joined;
                    }

                    // It carries on while the floor joined toward greater x is alike, up to where a region is cut.
                    At = (++Upper < Limit && Next != kNowhere && IsAlike(Own, mStoreys[Next])) ? Next : kNowhere;
                }

                const Bool Whole = (Above != 1 || Joined == Min(Upper, mRuns[Parent].Upper) - Max(Lower, mRuns[Parent].Lower));

                mRuns.Append(Lower, Upper, Each.Area, Each.Piled, Low, High, Above, Parent, Whole);
            }

            Carry(Start, static_cast<UInt32>(mRuns.GetSize()), Z);
        }
    }
}