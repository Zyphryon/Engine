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

#include "Field.hpp"
#include "Mesh.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    /// \brief Represents a baker that turns the cells of a field into mesh polygons, keeping its buffers between tiles.
    class ZY_API Baker final
    {
    public:

        /// \brief Represents the settings a field is baked with.
        struct Recipe final
        {
            /// The unmeshed cells round the tile, at least the radius plus one where tiles meet, or plus two with a slope.
            UInt16 Border    = 0;

            /// The radius of the body walking the mesh, in cells, by which the floor is kept back from walls.
            UInt16 Radius    = 0;

            /// The highest step a body climbs between two neighbouring floors, in height steps.
            UInt16 Climb     = 1;

            /// The steepest ground a body stands on, as the rise across one cell in height steps, or zero for any.
            Real32 Slope     = 0.0f;

            /// The farthest a polygon side may stray from the cells it outlines, in cells.
            Real32 Tolerance = 1.3f;

            /// The fewest cells an island of floor that touches no tile edge must cover to be kept, or zero to keep all.
            UInt32 Island    = 0;

            /// The most cells a region spans along either axis before it is cut, or zero to never cut one.
            UInt32 Extent    = 0;

            /// The headroom a body needs over a floor, in height steps, or zero to ignore the slabs.
            UInt16 Headroom  = 0;
        };

    public:

        /// \brief Constructs a baker with empty buffers.
        ZY_INLINE Baker()
            : mFloors  { nullptr },
              mAreas   { nullptr },
              mWidth   { 0 },
              mDepth   { 0 },
              mBorder  { 0 },
              mSpanX   { 0 },
              mSpanZ   { 0 },
              mClimb   { 0 },
              mStretch { 0 },
              mBand    { 0 },
              mLayered { false },
              mSteep   { false },
              mWay     { 0 },
              mRegion  { 0 }
        {
        }

        /// \brief Bakes a field into a mesh.
        ///
        /// \param Source The field to bake, which holds the tile and the border the recipe names.
        /// \param Recipe The settings the field is baked with.
        /// \param Output Receives the polygons of the tile, replacing whatever the mesh held.
        void Bake(ConstRef<Field> Source, ConstRef<Recipe> Recipe, Ref<Mesh> Output);

    private:

        /// \brief The label of a cell nothing may stand on, and of a side a wall lies past.
        static constexpr UInt16 kWall    = 0;

        /// \brief The label of a cell past the tile edge that may be stood on.
        static constexpr UInt16 kBeyond  = 0xFFFF;

        /// \brief The index that points at nothing.
        static constexpr UInt32 kNowhere = 0xFFFFFFFF;

        /// \brief The bit a diagonal carries when another corner lies inside the triangle it clips, so the corner is no ear.
        static constexpr UInt32 kPinched = 0x80000000;

        /// \brief The steepest slope used, in height steps, so the squared rises still fit in 32 bits.
        static constexpr Real32 kSteep   = 32766.0f;

        /// \brief Represents a run of cells along one row that may be walked from end to end.
        struct Run final
        {
            /// The first column of the run.
            UInt16 Lower  = 0;

            /// The column past the last of the run.
            UInt16 Upper  = 0;

            /// The area the run belongs to.
            UInt8  Area   = 0;

            /// `true` when the cells of the run hold another floor over or under this one, `false` when it stands alone.
            Bool   Piled  = false;

            /// The height of its lowest floor, in height steps, or zero on plain ground.
            UInt16 Low    = 0;

            /// The height of its highest floor, in height steps, or the greatest height on plain ground.
            UInt16 High   = 0xFFFF;

            /// The number of runs in the row before that this one joins.
            UInt8  Above  = 0;

            /// The run in the row before that this one joined last, or \ref kNowhere.
            UInt32 Parent = kNowhere;

            /// `true` when the run joins the row before along all the columns they share, `false` otherwise.
            Bool   Whole  = true;

            /// The number of runs in the row after that this one joins.
            UInt8  Below  = 0;

            /// The region the run belongs to, counted from one.
            UInt16 Region = 0;

            /// The run in the row after that continues the region, or \ref kNowhere.
            UInt32 Next   = kNowhere;
        };

        /// \brief Represents one floor of a stacked cell and the neighbouring floors it joins.
        struct Storey final
        {
            /// The column of the cell in the field.
            UInt16           X     = 0;

            /// The row of the cell in the field.
            UInt16           Y     = 0;

            /// The height of the floor, in height steps.
            UInt16           Floor = 0;

            /// The highest a neighbouring floor may stand and still leave a body headroom under the ceiling over this one.
            UInt16           Roof  = 0xFFFF;

            /// The area of the floor, or \ref Field::kBlocked once it is too steep or too close to a wall.
            UInt8            Area  = Field::kBlocked;

            /// `true` when the cell holds another floor over or under this one, `false` when it stands alone.
            Bool             Piled = false;

            /// The run the floor belongs to, or \ref kNowhere when it belongs to none.
            UInt32           Run   = kNowhere;

            /// The floor joined toward lesser x, greater z, greater x and lesser z, or \ref kNowhere.
            Array<UInt32, 4> Link  = Array(kNowhere, kNowhere, kNowhere, kNowhere);

            /// \brief Checks whether the floor is joined on all four sides.
            ///
            /// \return `true` when all four sides are joined, `false` when a wall or a drop lies past one.
            ZY_INLINE Bool IsSurrounded() const
            {
                return Link[0] != kNowhere && Link[1] != kNowhere && Link[2] != kNowhere && Link[3] != kNowhere;
            }

            /// \brief Gets the step a body takes between this floor and a neighbouring one.
            ///
            /// \param Other The neighbouring floor.
            /// \return The height between the two, in height steps, or the largest value when a body does not fit.
            ZY_INLINE SInt32 GetStep(ConstRef<Storey> Other) const
            {
                return Min(Roof, Other.Roof) >= Max(Floor, Other.Floor) ? Abs(Floor - Other.Floor) : kMaximum<SInt32>;
            }
        };

        /// \brief Represents a region of cells with no hole, made of one run on each of its rows.
        struct Region final
        {
            /// The run on its first row.
            UInt32 First = 0;

            /// The row its first run lies on.
            UInt16 Top   = 0;

            /// The area the region belongs to.
            UInt8  Area  = 0;

            /// The height band its floors stand in, or \ref kNowhere when no floor is stacked over or under it.
            UInt32 Tier  = 0;

            /// The height of its lowest floor, in height steps, or zero on plain ground.
            UInt16 Low   = 0;

            /// The height of its highest floor, in height steps, or the greatest height on plain ground.
            UInt16 High  = 0xFFFF;

            /// The number of cells the region covers.
            UInt32 Cells = 0;
        };

        /// \brief Represents a corner of an outline and what lies past the side leaving it.
        struct Point final
        {
            /// The column of the corner, in cells from the tile's lowest corner.
            SInt32 X    = 0;

            /// The row of the corner, in cells from the tile's lowest corner.
            SInt32 Z    = 0;

            /// The region past the side leaving the corner, or \ref kWall or \ref kBeyond.
            UInt16 Past = kWall;

            /// The stacked floor the side leaving the corner belongs to, or \ref kNowhere on plain ground.
            UInt32 Own  = kNowhere;

            /// \brief Checks whether two corners stand at one position.
            ///
            /// \param Other The corner to compare with.
            /// \return `true` when both stand at the same position, `false` otherwise.
            ZY_INLINE Bool IsAt(ConstRef<Point> Other) const
            {
                return X == Other.X && Z == Other.Z;
            }
        };

        /// \brief Represents a corner of the outline being triangulated, linked to its neighbours still left.
        struct Knot final
        {
            /// The previous corner among those not yet clipped.
            UInt16 Before = 0;

            /// The next corner among those not yet clipped.
            UInt16 After  = 0;

            /// The mesh corner standing at it.
            UInt16 Corner = 0;

            /// The region, wall or tile edge past the side leaving it, or its own region once that side is a diagonal.
            UInt16 Past   = kWall;

            /// The squared diagonal that clips it, with \ref kPinched set when it is no ear, or \ref kNowhere when reflex.
            UInt32 Span   = kNowhere;
        };

        /// \brief Represents a side of a polygon being merged, by the corner it leaves.
        struct Flank final
        {
            /// The mesh corner the side leaves.
            UInt16 Corner = 0;

            /// The region, wall or tile edge past the side.
            UInt16 Past   = kWall;

            /// The polygon sharing the side, counted from one, or zero when none does.
            UInt16 Twin   = 0;
        };

        /// \brief Represents a polygon while the triangles of a tile are merged into as few convex ones as possible.
        struct Shape final
        {
            /// The sides of the polygon, in winding order.
            Array<Flank, Mesh::kSides> Flank;

            /// The sides in use, or zero once the polygon is merged into another or dropped.
            UInt8                      Count  = 0;

            /// The region the polygon was cut from, counted from one.
            UInt16                     Region = 0;

            /// The height of the lowest floor under the polygon, in height steps.
            UInt16                     Low    = 0;

            /// The height of the highest floor under the polygon, in height steps.
            UInt16                     High   = 0xFFFF;

            /// \brief Gets a side by its position round the polygon, wrapping once past either end.
            ///
            /// \param Index The position of the side, from minus the side count to one short of twice the side count.
            /// \return The side at that position.
            ZY_INLINE ConstRef<Baker::Flank> GetSide(SInt32 Index) const
            {
                return Flank[Index < 0 ? Index + Count : Index >= Count ? Index - Count : Index];
            }

            /// \brief Finds the side that runs from one corner to another.
            ///
            /// \param From The corner the side leaves.
            /// \param Into The corner the side ends at.
            /// \return The position of the side, or minus one when the polygon has no such side.
            ZY_INLINE SInt32 Find(UInt16 From, UInt16 Into) const
            {
                for (SInt32 Side = Count - 1; Side >= 0; --Side)
                {
                    if (Flank[Side].Corner == From && GetSide(Side + 1).Corner == Into)
                    {
                        return Side;
                    }
                }
                return -1;
            }
        };

        /// \brief Represents a side two polygons may share.
        struct Edge final
        {
            /// The lesser mesh corner of the side.
            UInt16           Lower  = 0;

            /// The greater mesh corner of the side.
            UInt16           Upper  = 0;

            /// The polygon walking the side from the lesser corner, and the one walking it back, or \ref Mesh::kNone.
            Array<UInt16, 2> Owner  = Array(Mesh::kNone, Mesh::kNone);

            /// The position of the side round the first of those polygons.
            UInt8            Side   = 0;

            /// The squared length of the side, in cells squared, which orders the merges longest first.
            UInt32           Length = 0;

            /// The next side filed under the same lesser corner, or \ref kNowhere.
            UInt32           Next   = kNowhere;
        };

        /// \brief Gets the index of a field cell among the open cells, which carry one blocked cell of padding round them.
        ///
        /// \param X The column of the cell in the field, from minus one to the width.
        /// \param Y The row of the cell in the field, from minus one to the height.
        /// \return The index of the cell among the open cells.
        ZY_INLINE UInt32 GetOpen(SInt32 X, SInt32 Y) const
        {
            return static_cast<UInt32>(Y + 1) * (mWidth + 2) + static_cast<UInt32>(X + 1);
        }

        /// \brief Gets the index of a tile cell among the labels, which carry one cell of padding round them.
        ///
        /// \param X The column of the cell in the tile, from minus one to the tile width.
        /// \param Z The row of the cell in the tile, from minus one to the tile height.
        /// \return The index of the cell among the labels.
        ZY_INLINE UInt32 GetLabel(SInt32 X, SInt32 Z) const
        {
            return static_cast<UInt32>(Z + 1) * (mSpanX + 2) + static_cast<UInt32>(X + 1);
        }

        /// \brief Gets the height of the ground of a tile cell.
        ///
        /// \param X The column of the cell in the tile, which may lie in the border.
        /// \param Z The row of the cell in the tile, which may lie in the border.
        /// \return The height of the ground, in height steps.
        ZY_INLINE SInt32 GetFloor(SInt32 X, SInt32 Z) const
        {
            return mFloors[(Z + mBorder) * mWidth + (X + mBorder)];
        }

        /// \brief Blocks every cell too steep to stand on, before the floor is eroded.
        ///
        /// \param Cap  The rise along one axis, in height steps, past which a cell is too steep on its own.
        /// \param Most The highest the squared rises along both axes may add up to, in height steps squared.
        void Grade(SInt32 Cap, SInt32 Most);

        /// \brief Erodes the floor away from whatever blocks it, leaving the cells the middle of a body may stand on.
        ///
        /// \param Radius The distance to erode by, in cells.
        void Erode(UInt32 Radius);

        /// \brief Splits the open cells of the tile into regions, and labels every cell with its region.
        void Partition();

        /// \brief Walks the outline of a region, adding a corner wherever it turns or what lies past it changes.
        ///
        /// \tparam Stacked `true` to read the links of stacked floors, `false` to read the cells of plain ground.
        /// \param  Region  The region to outline.
        template<Bool Stacked>
        void Outline(ConstRef<Region> Region);

        /// \brief Adds one cell side to the outline being walked, merging it into the side before when it continues it.
        ///
        /// \param X     The column the side starts at, in cells.
        /// \param Z     The row the side starts at, in cells.
        /// \param StepX The step the side takes along x, from minus one to one.
        /// \param StepZ The step the side takes along z, from minus one to one.
        /// \param Past  The region past the side, or \ref kWall or \ref kBeyond.
        /// \param Own   The stacked floor the side belongs to, or \ref kNowhere on plain ground.
        ZY_INLINE void Trace(SInt32 X, SInt32 Z, SInt32 StepX, SInt32 StepZ, UInt16 Past, UInt32 Own = kNowhere)
        {
            const SInt32 Way = StepX * 2 + StepZ;

            if (Way != mWay || Past != mRaw.GetBack().Past)
            {
                mRaw.Append(X, Z, Past, Own);
                mWay = Way;

                // What lies past the side tells which regions form one island and which reach the tile edge.
                if (Past == kBeyond)
                {
                    mOuter[mRegion] = 1;
                }
                else if (Past != kWall)
                {
                    mGroups[GetRoot(mGroups, Past)] = GetRoot(mGroups, mRegion);
                }
            }
        }

        /// \brief Adds the sides of a run of region cells that face one way to the outline being walked.
        ///
        /// \tparam Stacked `true` to read the links of stacked floors, `false` to read the cells of plain ground.
        /// \param  Top     The row the region starts on.
        /// \param  Row     The row of the cells, counted from the first row of the region.
        /// \param  From    The first column of the run.
        /// \param  Into    The column past the last of the run, with no cell when it is not past the first.
        /// \param  Way     The way the sides face, as the links of a floor are numbered.
        template<Bool Stacked>
        ZY_INLINE void Face(SInt32 Top, SInt32 Row, SInt32 From, SInt32 Into, UInt32 Way)
        {
            const SInt32 Z      = Top + Row;
            const SInt32 StepX  = (Way == 1 ? 1 : 0) - (Way == 3 ? 1 : 0);
            const SInt32 StepZ  = (Way == 0 ? 1 : 0) - (Way == 2 ? 1 : 0);
            const SInt32 FarX   = (Way >= 2 ? 1 : 0);
            const SInt32 FarZ   = (Way == 1 || Way == 2 ? 1 : 0);
            const UInt32 Beside = GetLabel(-StepZ, Z + StepX);
            UInt32       Held   = kNowhere;

            // Sides facing lesser z are walked toward lesser x, and a corner is added only where what lies past changes.
            for (SInt32 X = (Way == 3 ? Into - 1 : From); X >= From && X < Into; X += (Way == 3 ? -1 : 1))
            {
                UInt32 Own  = kNowhere;
                UInt16 Past = kWall;

                // Stacked floors read what lies past from their links, and plain ground from the neighbouring label.
                if constexpr (Stacked)
                {
                    // A cell with several floors is searched for the one this run passes through.
                    Own = mStacks[(Z + mBorder) * mWidth + mBorder + X];

                    while (mStoreys[Own].Run != mOrder[Row])
                    {
                        ++Own;
                    }

                    Past = GetBeside(Own, Way);
                }
                else
                {
                    Past = mLabels[Beside + X];

                    if (mSteep && Past != kWall && Abs(GetFloor(X, Z) - GetFloor(X - StepZ, Z + StepX)) > mClimb)
                    {
                        Past = kWall;
                    }
                }

                if (Past != Held)
                {
                    Trace(X + FarX, Z + FarZ, StepX, StepZ, Past, Own);
                    Held = Past;
                }
            }
        }

        /// \brief Straightens the walls of the outline just walked, keeping every corner where what lies past changes.
        ///
        /// \param Tolerance The farthest a straightened wall may stray from the corners it drops, in cells.
        void Simplify(Real32 Tolerance);

        /// \brief Adds the straightened outline as one polygon when it is convex, or clips it into triangles otherwise.
        ///
        /// \param Output The mesh the corners are added to.
        void Triangulate(Ref<Mesh> Output);

        /// \brief Measures the diagonal that clipping a corner of the outline would leave.
        ///
        /// \param Here The corner to measure, among those not yet clipped.
        /// \return The squared diagonal, with \ref kPinched set when it is no ear, or \ref kNowhere when reflex.
        ZY_INLINE UInt32 GetSpan(UInt32 Here) const
        {
            ConstRef<Knot>  Held   = mRing[Here];
            ConstRef<Point> First  = mLoop[Held.Before];
            ConstRef<Point> Second = mLoop[Here];
            ConstRef<Point> Third  = mLoop[Held.After];

            if (Turn(First, Second, Third) >= 0)
            {
                return kNowhere;
            }

            const SInt32 AlongX = Third.X - First.X;
            const SInt32 AlongZ = Third.Z - First.Z;
            const UInt32 Across = static_cast<UInt32>(AlongX * AlongX + AlongZ * AlongZ);

            // A corner is an ear when no other corner lies inside its triangle, except one standing at a corner of it.
            for (UInt32 At = mRing[Held.After].After; At != Held.Before; At = mRing[At].After)
            {
                ConstRef<Point> Each = mLoop[At];

                if (Turn(First, Second, Each) <= 0 && Turn(Second, Third, Each) <= 0 && Turn(Third, First, Each) <= 0)
                {
                    if (!Each.IsAt(First) && !Each.IsAt(Second) && !Each.IsAt(Third))
                    {
                        return Across | kPinched;
                    }
                }
            }
            return Across;
        }

        /// \brief Adds a polygon made of the outline corners that follow one another from one of them.
        ///
        /// \param First The corner the polygon starts at, among those not yet clipped.
        /// \param Count The number of corners in the polygon, three when an ear is clipped.
        /// \param Past  The region, wall or tile edge past the side that closes the polygon.
        ZY_INLINE void Clip(UInt32 First, UInt32 Count, UInt16 Past)
        {
            ConstRef<Region> Region = mRegions[mRegion - 1];
            Ref<Shape>       Made   = mShapes.Append();

            for (UInt32 Side = 0, At = First; Side < Count; ++Side, At = mRing[At].After)
            {
                Made.Flank[Side] = Flank(mRing[At].Corner, mRing[At].Past);
            }

            Made.Flank[Count - 1].Past = Past;
            Made.Count                 = static_cast<UInt8>(Count);
            Made.Region                = mRegion;
            Made.Low                   = Region.Low;
            Made.High                  = Region.High;
        }

        /// \brief Drops the polygons of every island too small to keep that touches no tile edge.
        ///
        /// \param Least The fewest cells an island must cover to be kept.
        void Prune(UInt32 Least);

        /// \brief Gets the mesh corner standing at an outline corner, adding it the first time it is asked for.
        ///
        /// \param Corner The outline corner, in cells from the tile's lowest corner.
        /// \param Output The mesh the corner is added to.
        /// \return The index of the mesh corner.
        ZY_INLINE UInt16 Weld(ConstRef<Point> Corner, Ref<Mesh> Output)
        {
            const SInt32 Border = static_cast<SInt32>(mBorder);
            Ref<UInt16>  Slot   = mCorners[static_cast<UInt32>(Corner.Z) * (mSpanX + 1) + static_cast<UInt32>(Corner.X)];

            // With stacked floors one position may hold a corner per floor, told apart by the floors joined round it.
            if (mLayered)
            {
                // A stacked corner stands as high as the highest floor joined round it.
                UInt16       High = 0;
                const UInt32 Key  = GetCorner(Corner.Own, Corner.X + Border, Corner.Z + Border, High);

                for (UInt16 At = Slot; At != 0; At = mChain[At - 1])
                {
                    if (mKeys[At - 1] == Key)
                    {
                        return At - 1;
                    }
                }

                const UInt16 Made = Output.AddVertex(static_cast<UInt16>(Corner.X), High, static_cast<UInt16>(Corner.Z));

                mChain.Append(Slot);
                mKeys.Append(Key);
                Slot = Made + 1;
                return Made;
            }

            if (Slot == 0)
            {
                // A corner takes the height of the highest open cell touching it, all four of which lie inside the field.
                UInt16 Height = 0;

                for (SInt32 Z = Corner.Z - 1 + Border; Z <= Corner.Z + Border; ++Z)
                {
                    for (SInt32 X = Corner.X - 1 + Border; X <= Corner.X + Border; ++X)
                    {
                        if (mOpen[GetOpen(X, Z)] != Field::kBlocked)
                        {
                            Height = Max(Height, mFloors[static_cast<UInt32>(Z) * mWidth + static_cast<UInt32>(X)]);
                        }
                    }
                }

                Slot = Output.AddVertex(static_cast<UInt16>(Corner.X), Height, static_cast<UInt16>(Corner.Z)) + 1;
            }
            return Slot - 1;
        }

        /// \brief Files every side of every polygon, pairing the two polygons that share one.
        ///
        /// \param Output The mesh holding the corners the polygons name.
        void Pair(ConstRef<Mesh> Output);

        /// \brief Merges polygons across the sides they share, longest side first, wherever the result stays convex.
        ///
        /// \param Output The mesh holding the corners the polygons name.
        void Merge(ConstRef<Mesh> Output);

        /// \brief Merges a polygon into another across the run of sides they share, when the result stays convex.
        ///
        /// \param First    The polygon walking the side from its lesser corner, which takes the other in.
        /// \param Second   The polygon walking the side back, which is left with no side once merged.
        /// \param Edge     The side the two share.
        /// \param Vertices The mesh corners, three values each.
        /// \return `true` when the two were merged, `false` when the result would be concave or hold too many corners.
        Bool Fuse(UInt16 First, UInt16 Second, ConstRef<Edge> Edge, ConstSpan<UInt16> Vertices);

        /// \brief Checks whether a side of a polygon is shared with another, including whatever that one has absorbed.
        ///
        /// \param Side  The side to check.
        /// \param Other The polygon asked about, standing for every polygon merged into it.
        /// \return `true` when the polygon past the side is the one asked about, `false` when a wall or another is.
        ZY_INLINE Bool IsShared(ConstRef<Flank> Side, UInt16 Other)
        {
            return Side.Twin != 0 && GetRoot(mMerged, Side.Twin - 1) == Other;
        }

        /// \brief Writes the merged polygons into the mesh, with what lies past each side.
        ///
        /// \param Output The mesh to write into.
        void Emit(Ref<Mesh> Output);

        /// \brief Gets what lies past one side of a stacked floor.
        ///
        /// \param Own The floor whose side is asked about.
        /// \param Way The way the side faces, as the links of a floor are numbered.
        /// \return The region of the floor joined past it, \ref kBeyond past the tile edge, or \ref kWall when none is.
        ZY_INLINE UInt16 GetBeside(UInt32 Own, UInt32 Way) const
        {
            const UInt32 Next = mStoreys[Own].Link[Way];

            if (Next == kNowhere || mStoreys[Next].Area == Field::kBlocked)
            {
                return kWall;
            }

            // Only the open floors of the tile get a run, so a floor with none lies in the border.
            const UInt32 Held = mStoreys[Next].Run;

            return (Held == kNowhere) ? kBeyond : mRuns[Held].Region;
        }

        /// \brief Gets the key every floor joined round a corner of a stacked floor agrees on, so each names it alike.
        ///
        /// \param Own  The floor whose corner is asked about.
        /// \param X    The column of the corner in the field.
        /// \param Y    The row of the corner in the field.
        /// \param High Receives the height of the corner, the highest of the floors joined round it, in height steps.
        /// \return The lowest index among the floors joined round the corner.
        ZY_INLINE UInt32 GetCorner(UInt32 Own, SInt32 X, SInt32 Y, Ref<UInt16> High) const
        {
            UInt32 Least = Own;

            High = mStoreys[Own].Floor;

            // The floors round a corner form a chain or a loop, walked from this one in both directions.
            for (UInt32 Round = 0; Round < 2; ++Round)
            {
                for (UInt32 At = Own, Step = Round; ; ++Step)
                {
                    ConstRef<Storey> Each = mStoreys[At];

                    At = (Step & 1) == 0 ? Each.Link[Each.X < X ? 2 : 0] : Each.Link[Each.Y < Y ? 1 : 3];

                    if (At == Own)
                    {
                        return Least;
                    }
                    if (At == kNowhere)
                    {
                        break;
                    }

                    Least = Min(Least, At);
                    High  = Max(High, mStoreys[At].Floor);
                }
            }
            return Least;
        }

        /// \brief Checks whether any slab adds a floor that plain ground cannot hold.
        ///
        /// \param Source The field to read.
        /// \param Height The headroom a body needs over a floor, in height steps.
        /// \return `true` when some slab is stood on or walked under, `false` when every slab only blocks its cell.
        Bool IsStacked(ConstRef<Field> Source, SInt32 Height) const;

        /// \brief Copies the areas of the field, blocking every cell a slab rises over, to be baked as plain ground.
        ///
        /// \param Source The field to read.
        void Shut(ConstRef<Field> Source);

        /// \brief Builds the floors of every cell, the ground and each slab top over it, and joins them to their neighbours.
        ///
        /// \param Source The field to read.
        /// \param Height The headroom a body needs over a floor, in height steps.
        void Stack(ConstRef<Field> Source, SInt32 Height);

        /// \brief Blocks every stacked floor too steep to stand on, as \ref Grade does for plain ground.
        ///
        /// \param Cap  The rise along one axis, in height steps, past which a floor is too steep on its own.
        /// \param Most The highest the squared rises along both axes may add up to, in height steps squared.
        void Pitch(SInt32 Cap, SInt32 Most);

        /// \brief Erodes the stacked floors away from whatever blocks them, as \ref Erode does for plain ground.
        ///
        /// \param Radius The distance to erode by, in cells.
        void Wear(UInt32 Radius);

        /// \brief Splits the open stacked floors of the tile into regions, as \ref Partition does for plain ground.
        void Cut();

        /// \brief Keeps the floor just added to a cell when it is clear with headroom for a body, and drops it otherwise.
        ///
        /// \param Slabs  The slabs of the field.
        /// \param Head   The first slab over the cell, counted from one, or zero for none.
        /// \param Skip   The slab whose top is the floor, counted from one, or zero for the ground.
        /// \param First  The first floor of the cell.
        /// \param Height The headroom a body needs over a floor, in height steps.
        ZY_INLINE void Settle(ConstSpan<Field::Slab> Slabs, UInt32 Head, UInt32 Skip, UInt32 First, SInt32 Height)
        {
            const UInt32 Last    = static_cast<UInt32>(mStoreys.GetSize()) - 1;
            const SInt32 Floor   = mStoreys[Last].Floor;
            Bool         Buried  = (mStoreys[Last].Area == Field::kBlocked);
            SInt32       Ceiling = 0xFFFF;

            for (UInt32 At = Head; At != 0; At = mOrder[At - 1])
            {
                ConstRef<Field::Slab> Each = Slabs[At - 1];

                if (At == Skip)
                {
                    continue;
                }

                if (Each.Bottom < Floor && Floor < Each.Top)
                {
                    Buried = true;
                }
                else if (Each.Bottom >= Floor)
                {
                    Ceiling = Min(Ceiling, static_cast<SInt32>(Each.Bottom));
                }
            }

            // Two tops at one height are one floor.
            for (UInt32 At = First; At < Last; ++At)
            {
                Buried = Buried || mStoreys[At].Floor == Floor;
            }

            if (Buried || Ceiling - Floor < Height)
            {
                mStoreys.RemoveLast();
                return;
            }

            mStoreys[Last].Roof = static_cast<UInt16>(Ceiling - Height);

            // The floors of a cell are kept lowest first.
            for (UInt32 At = Last; At > First && mStoreys[At].Floor < mStoreys[At - 1].Floor; --At)
            {
                Swap(mStoreys[At], mStoreys[At - 1]);
            }
        }

        /// \brief Picks the floor of a neighbouring cell nearest to a floor, among those a body can step to.
        ///
        /// \param Index The floor to pick for.
        /// \param First The first floor of the neighbouring cell.
        /// \param Last  The floor past the last of the neighbouring cell.
        /// \return The nearest floor within the climb, the lowest of two equally near, or \ref kNowhere.
        ZY_INLINE UInt32 Pick(UInt32 Index, UInt32 First, UInt32 Last) const
        {
            UInt32 Found   = kNowhere;
            SInt32 Nearest = mClimb + 1;

            for (UInt32 At = First; At < Last; ++At)
            {
                if (const SInt32 Apart = mStoreys[Index].GetStep(mStoreys[At]); Apart < Nearest)
                {
                    Nearest = Apart;
                    Found   = At;
                }
            }
            return Found;
        }

        /// \brief Joins the floors of a cell to those of a neighbouring cell built before it, both ways.
        ///
        /// \param First  The first floor of the cell.
        /// \param Last   The floor past the last of the cell.
        /// \param Beside The neighbouring cell, toward lesser x or lesser z.
        /// \param Way    The way the neighbouring cell lies, as the links of a floor are numbered.
        ZY_INLINE void Join(UInt32 First, UInt32 Last, UInt32 Beside, UInt32 Way)
        {
            const UInt32 Lower = mStacks[Beside];
            const UInt32 Upper = mStacks[Beside + 1];

            // A single floor next to a single floor is joined when within the climb, with nothing to pick.
            if (Last - First == 1 && Upper - Lower == 1)
            {
                if (mStoreys[First].GetStep(mStoreys[Lower]) <= mClimb)
                {
                    mStoreys[First].Link[Way]     = Lower;
                    mStoreys[Lower].Link[Way ^ 2] = First;
                }
                return;
            }

            // Otherwise each floor picks the nearest in reach, and is joined only when that floor picks it back.
            for (UInt32 At = First; At < Last; ++At)
            {
                if (const UInt32 Other = Pick(At, Lower, Upper); Other != kNowhere && Pick(Other, First, Last) == At)
                {
                    mStoreys[At].Link[Way]        = Other;
                    mStoreys[Other].Link[Way ^ 2] = At;
                }
            }
        }

        /// \brief Gets the distance from a floor joined on all four sides to the nearest one not clear, looking one way.
        ///
        /// \param Index The floor to look from.
        /// \param Side  The way to look, at the neighbouring floor and the diagonal one a turn past it.
        /// \return The distance, two more than the neighbouring floor's or three more than the diagonal one's.
        ZY_INLINE UInt32 GetNear(UInt32 Index, UInt32 Side) const
        {
            const UInt32 Beside = mStoreys[Index].Link[Side];
            const UInt32 Corner = mStoreys[Beside].Link[(Side + 3) & 3];
            const UInt32 Near   = mKept[Beside] + 2u;

            return (Corner != kNowhere) ? Min(Near, mKept[Corner] + 3u) : Near;
        }

        /// \brief Checks whether two joined floors may share a region.
        ///
        /// \param Left  The first floor.
        /// \param Right The second floor.
        /// \return `true` when they share an area and both stand alone or both lie in one height band, `false` otherwise.
        ZY_INLINE Bool IsAlike(ConstRef<Storey> Left, ConstRef<Storey> Right) const
        {
            if (Left.Area != Right.Area || Left.Piled != Right.Piled)
            {
                return false;
            }
            return !Left.Piled || Left.Floor == Right.Floor || Left.Floor / mBand == Right.Floor / mBand;
        }

        /// \brief Carries the regions of the row before through the runs of a row, starting a region where none carries on.
        ///
        /// \param Start The first run of the row.
        /// \param End   The run past the last of the row.
        /// \param Z     The row the runs lie on.
        ZY_INLINE void Carry(UInt32 Start, UInt32 End, SInt32 Z)
        {
            // A run carries a region on only where the two rows meet one to one, so a region keeps one run per row.
            const Bool Uncut = (mStretch == 0 || Z % mStretch != 0);

            for (UInt32 New = Start; New < End; ++New)
            {
                Ref<Run> Made = mRuns[New];

                if (Made.Above == 1 && Made.Whole && mRuns[Made.Parent].Below == 1 && Uncut)
                {
                    Ref<Run> Held = mRuns[Made.Parent];

                    Made.Region = Held.Region;
                    Held.Next   = New;
                }
                else
                {
                    // Only a floor with another over or under it is held to one height band.
                    const UInt32 Tier = Made.Piled ? Made.Low / mBand : kNowhere;

                    mRegions.Append(New, static_cast<UInt16>(Z), Made.Area, Tier, Made.Low, Made.High);

                    ZY_ASSERT(mRegions.GetSize() < kBeyond, "A tile holds at most 65534 regions, so bake it smaller");

                    Made.Region = static_cast<UInt16>(mRegions.GetSize());
                }

                Ref<Region> Region = mRegions[Made.Region - 1];

                Region.Cells += Made.Upper - Made.Lower;
                Region.Low    = Min(Region.Low, Made.Low);
                Region.High   = Max(Region.High, Made.High);

                // Plain ground labels every cell with its region, which the outline reads to tell what lies past a side.
                if (!mLayered)
                {
                    Stamp(mLabels.GetData() + GetLabel(Made.Lower, Z), Made.Upper - Made.Lower, Made.Region);
                }
            }
        }

        /// \brief Gets the rise from one floor onto a neighbouring one, when a body can step between them.
        ///
        /// \param From  The height of the floor stepped off, in height steps.
        /// \param Into  The height of the floor stepped onto, in height steps.
        /// \param Open  The lesser area of the two cells, which is blocked when either is.
        /// \param Reach The smallest difference a body cannot climb, in height steps.
        /// \return The rise in height steps, negative for a drop, or zero when no step leads between them.
        ZY_INLINE static SInt32 GetRise(SInt32 From, SInt32 Into, SInt32 Open, SInt32 Reach)
        {
            const SInt32 Rise = Into - From;

            return Rise * Clamp(Min(Reach - Abs(Rise), Open), 0, 1);
        }

        /// \brief Gets the slope of the ground through a cell, the gentler of the rises onto it and off it.
        ///
        /// \param Onto The rise onto the cell from the one before it, in height steps.
        /// \param Off  The rise off the cell onto the one after it, in height steps.
        /// \param Cap  The rise past which the ground is too steep on its own, in height steps.
        /// \return The rise in height steps, negative for a fall, or zero where the ground turns or lies level.
        ZY_INLINE static SInt32 GetSlope(SInt32 Onto, SInt32 Off, SInt32 Cap)
        {
            return Clamp(Max(Min(Onto, Off), 0) + Min(Max(Onto, Off), 0), -Cap, Cap);
        }

        /// \brief Writes one label over a run of cells, four at a time with the last four overlapping its end.
        ///
        /// \param Into  The first label to write over.
        /// \param Count The number of labels to write.
        /// \param Label The region the cells are labelled with.
        ZY_INLINE static void Stamp(Ptr<UInt16> Into, UInt32 Count, UInt16 Label)
        {
            const UInt64 Four = Label * 0x0001000100010001ull;

            for (UInt32 At = 0; At + 4 < Count; At += 4)
            {
                Blit(Into + At, sizeof(Four), AddressOf(Four));
            }

            if (Count >= 4)
            {
                Blit(Into + Count - 4, sizeof(Four), AddressOf(Four));
            }
            else
            {
                Fill(Into, Count, Label);
            }
        }

        /// \brief Gets how far a row of cells stays alike from one of them, reading eight cells at a time.
        ///
        /// \param Row   The cells of the row.
        /// \param From  The cell to start from.
        /// \param Limit The cell to stop before at the latest.
        /// \return The first cell after the start that differs from it, or the limit when none does.
        ZY_INLINE static SInt32 Extend(ConstPtr<UInt8> Row, SInt32 From, SInt32 Limit)
        {
            const UInt64 Alike = Row[From] * 0x0101010101010101ull;
            SInt32       X     = From + 1;

            for (UInt64 Word = 0; X + 8 <= Limit; X += 8)
            {
                Blit(AddressOf(Word), sizeof(Word), Row + X);

                if (Word != Alike)
                {
                    return X + static_cast<SInt32>(CountTrailingZeros(Word ^ Alike) >> 3);
                }
            }

            while (X < Limit && Row[X] == Row[From])
            {
                ++X;
            }
            return X;
        }

        /// \brief Gets the root of the set a member belongs to, shortening the path to it.
        ///
        /// \param Sets  The parent of each member, itself at a root.
        /// \param Index The member to look up.
        /// \return The root of its set.
        ZY_INLINE static UInt16 GetRoot(Ref<Sequence<UInt16>> Sets, UInt16 Index)
        {
            while (Sets[Index] != Index)
            {
                Sets[Index] = Sets[Sets[Index]];
                Index       = Sets[Index];
            }
            return Index;
        }

        /// \brief Checks whether three mesh corners walked in a row turn the way a polygon is wound.
        ///
        /// \param Vertices The mesh corners, three values each.
        /// \param Before   The corner walked from.
        /// \param Here     The corner the turn is made at.
        /// \param After    The corner walked to.
        /// \return `true` when the turn keeps a polygon convex, `false` when it bends back or doubles back.
        ZY_INLINE static Bool Holds(ConstSpan<UInt16> Vertices, UInt16 Before, UInt16 Here, UInt16 After)
        {
            const SInt32 FromX = Vertices[Here * 3]      - Vertices[Before * 3];
            const SInt32 FromZ = Vertices[Here * 3 + 2]  - Vertices[Before * 3 + 2];
            const SInt32 IntoX = Vertices[After * 3]     - Vertices[Here * 3];
            const SInt32 IntoZ = Vertices[After * 3 + 2] - Vertices[Here * 3 + 2];
            const SInt32 Bend  = FromX * IntoZ - IntoX * FromZ;

            return Bend < 0 || (Bend == 0 && FromX * IntoX + FromZ * IntoZ > 0);
        }

        /// \brief Computes which way three corners turn.
        ///
        /// \param First  The corner the turn starts from.
        /// \param Second The corner the turn is made at.
        /// \param Third  The corner the turn ends at.
        /// \return The cross product of their sides, negative in winding order and zero when they lie on one line.
        ZY_INLINE static SInt32 Turn(ConstRef<Point> First, ConstRef<Point> Second, ConstRef<Point> Third)
        {
            return (Second.X - First.X) * (Third.Z - First.Z) - (Third.X - First.X) * (Second.Z - First.Z);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ConstPtr<UInt16> mFloors;
        ConstPtr<UInt8>  mAreas;
        UInt32           mWidth;
        UInt32           mDepth;
        UInt32           mBorder;
        UInt32           mSpanX;
        UInt32           mSpanZ;
        SInt32           mClimb;
        SInt32           mStretch;
        UInt32           mBand;
        Bool             mLayered;
        Bool             mSteep;
        SInt32           mWay;
        UInt16           mRegion;
        Sequence<UInt16> mGroups;
        Sequence<UInt8>  mOuter;
        Sequence<UInt32> mSizes;
        Sequence<UInt8>  mOpen;
        Sequence<UInt16> mLabels;
        Sequence<Run>    mRuns;
        Sequence<Region> mRegions;
        Sequence<Point>  mRaw;
        Sequence<UInt8>  mKept;
        Sequence<UInt32> mPending;
        Sequence<Point>  mLoop;
        Sequence<Knot>   mRing;
        Sequence<UInt16> mMerged;
        Sequence<UInt16> mCorners;
        Sequence<Shape>  mShapes;
        Sequence<Edge>   mEdges;
        Sequence<UInt32> mFirst;
        Sequence<UInt32> mOrder;
        Sequence<UInt16> mNames;
        Sequence<UInt32> mStacks;
        Sequence<Storey> mStoreys;
        Sequence<UInt16> mChain;
        Sequence<UInt32> mKeys;
        Sequence<UInt8>  mShut;
        Sequence<UInt8>  mGraded;
        Sequence<SInt32> mRises;
    };
}