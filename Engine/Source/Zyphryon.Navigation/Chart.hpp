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

#include "Mesh.hpp"
#include "Zyphryon.Math/Geometry/Line.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    /// \brief Represents baked tiles laid side by side and linked across their shared edges at run time.
    class ZY_API Chart final
    {
    public:

        /// \brief The width of one bucket of a tile's lookup grid, in cells.
        static constexpr UInt32 kBucket  = 8;

        /// \brief The polygon value that names no polygon.
        static constexpr UInt32 kNowhere = 0xFFFFFFFF;

    public:

        /// \brief Constructs an empty chart where every area costs one.
        ZY_INLINE Chart()
            : mSpan     { 1 },
              mRise     { 0 },
              mClimb    { kMaximum<Real32> },
              mCheapest { 1 },
              mCount    { 0 }
        {
            mCosts.Fill(1.0f);
        }

        /// \brief Empties the chart, sets every area cost back to one, and sets the tile width and height weighting.
        ///
        /// \param Span  The width of a tile along each axis, in cells.
        /// \param Rise  The cells one height step counts as when snapping between stacked floors, or zero to ignore height.
        /// \param Climb The largest height gap two polygons across a tile edge may have and still link, in height steps.
        void Reset(UInt32 Span, Real32 Rise = 0.0f, Real32 Climb = kMaximum<Real32>);

        /// \brief Adds a tile, replacing any tile already there, and links it to the tiles beside it.
        ///
        /// \param X     The column of the tile, in tiles.
        /// \param Z     The row of the tile, in tiles.
        /// \param Floor The baked mesh of the tile, which the chart takes ownership of.
        void Add(SInt32 X, SInt32 Z, AnyRef<Mesh> Floor);

        /// \brief Removes a tile, if there is one, and cuts the links the tiles beside it had into it.
        ///
        /// \param X The column of the tile, in tiles.
        /// \param Z The row of the tile, in tiles.
        void Remove(SInt32 X, SInt32 Z);

        /// \brief Sets the cost of crossing one cell of an area, which searches weigh paths by.
        ///
        /// \param Area The area to set, as tagged on the field cells before the tile was baked.
        /// \param Cost The cost of crossing one cell, above zero, where plain ground costs one.
        void SetCost(UInt8 Area, Real32 Cost);

        /// \brief Gets the cost of crossing one cell of a polygon.
        ///
        /// \param Polygon The polygon to read, its tile slot in the high half and its index in the low half.
        /// \return The cost of its area, or one when that area was never set.
        ZY_INLINE Real32 GetCost(UInt32 Polygon) const
        {
            return mCosts[mTiles[Polygon >> 16].Floor.GetAreas()[Polygon & 0xFFFF]];
        }

        /// \brief Gets the mesh of a tile.
        ///
        /// \param X The column of the tile, in tiles.
        /// \param Z The row of the tile, in tiles.
        /// \return The mesh, or `nullptr` when there is no tile there.
        ConstPtr<Mesh> GetMesh(SInt32 X, SInt32 Z) const;

        /// \brief Gets the number of polygons across every tile.
        ///
        /// \return The number of polygons, one past the largest \ref GetIndex.
        ZY_INLINE UInt32 GetCount() const
        {
            return mCount;
        }

        /// \brief Gets the dense index of a polygon across the chart, valid until a tile is added or removed.
        ///
        /// \param Polygon The polygon to read.
        /// \return The index, below \ref GetCount.
        ZY_INLINE UInt32 GetIndex(UInt32 Polygon) const
        {
            return mTiles[Polygon >> 16].First + (Polygon & 0xFFFF);
        }

        /// \brief Gets the cheapest area cost, which a search estimates the remaining path with.
        ///
        /// \return The cost of one cell of the cheapest area, at most one.
        ZY_INLINE Real32 GetCheapest() const
        {
            return mCheapest;
        }

        /// \brief Finds the polygon nearest a position and moves the position onto it.
        ///
        /// \param Spot  The position in cells with its height in height steps, moved onto the polygon found.
        /// \param Reach The farthest horizontal distance to search, in cells.
        /// \return The polygon found, or \ref kNowhere when none lies within reach and the position is left as is.
        UInt32 Snap(Ref<Vector3> Spot, Real32 Reach) const;

        /// \brief Gets the point of a polygon nearest a position, ignoring height.
        ///
        /// \param Polygon The polygon to clamp to.
        /// \param Spot    The position to clamp, in cells, whose height is ignored.
        /// \return The position itself when inside the polygon, else the nearest point on its outline, at floor height.
        Vector3 GetNearest(UInt32 Polygon, Vector3 Spot) const;

        /// \brief Gets the floor height of a polygon under a position.
        ///
        /// \param Polygon The polygon to read.
        /// \param Spot    The horizontal position, in cells.
        /// \return The floor height in height steps, interpolated from the polygon's corners.
        Real32 GetHeight(UInt32 Polygon, Vector2 Spot) const;

        /// \brief Invokes an action for every side of a polygon, once per polygon past it, with the side's two ends.
        ///
        /// \param Polygon The polygon to walk.
        /// \param Action  The action invoked with the polygon past the side or \ref kNowhere, then the two ends in cells.
        template<typename Callable>
        ZY_INLINE void Visit(UInt32 Polygon, AnyRef<Callable> Action) const
        {
            ConstRef<Tile>          Held     = mTiles[Polygon >> 16];
            const ConstSpan<UInt16> Each     = Held.Floor.GetPolygon(Polygon & 0xFFFF);
            const ConstPtr<UInt16>  Vertices = Held.Floor.GetVertices().GetData();

            const Vector2 Origin(static_cast<Real32>(Held.X) * mSpan, static_cast<Real32>(Held.Z) * mSpan);
            const Vector2 First  = Origin + GetSpot(Vertices, Each[0]);
            Vector2       Before = First;

            for (UInt32 Side = 0; Side < Mesh::kSides && Each[Side] != Mesh::kNone; ++Side)
            {
                const Bool    Last  = (Side + 1 == Mesh::kSides || Each[Side + 1] == Mesh::kNone);
                const Vector2 After = Last ? First : Origin + GetSpot(Vertices, Each[Side + 1]);
                const UInt16  Past  = Each[Mesh::kSides + Side];

                if (Past < Mesh::kPortal)
                {
                    ZY_INLINE_CALL Action((Polygon & 0xFFFF0000) | Past, Before, After);
                }
                else if (Past == Mesh::kNone || Held.Gates[Past & 0x7FFF].Count == 0)
                {
                    ZY_INLINE_CALL Action(kNowhere, Before, After);
                }
                else
                {
                    // A side on the tile edge is split into the stretch it shares with each polygon past it.
                    ConstRef<Gate> Door  = Held.Gates[Past & 0x7FFF];
                    const Bool     Along = (Door.Edge & 1) != 0;

                    for (UInt32 At = Door.First; At < Door.First + Door.Count; ++At)
                    {
                        ConstRef<Link> Link  = Held.Links[At];
                        const Vector2  Begin = Clip(Before, Link, Along);
                        const Vector2  End   = Clip(After, Link, Along);

                        ZY_INLINE_CALL Action(Link.Into, Begin, End);
                    }
                }
                Before = After;
            }
        }

    private:

        /// \brief Represents a polygon of the next tile that a side on the tile edge leads into.
        struct Link final
        {
            /// The polygon past the side, or \ref kNowhere when unset.
            UInt32 Into  = kNowhere;

            /// The low end of the stretch both sides share along the edge, in cells.
            Real32 Lower = 0.0f;

            /// The high end of the stretch both sides share along the edge, in cells.
            Real32 Upper = 0.0f;
        };

        /// \brief Represents a side of a polygon that lies on the edge of its tile.
        struct Gate final
        {
            /// The index of the polygon the side belongs to, within the tile.
            UInt16 Polygon = 0;

            /// The index of the side within its polygon, below \ref Mesh::kSides.
            UInt8  Side    = 0;

            /// The tile edge the side lies on, from 0 to 3 in \ref kAround order.
            UInt8  Edge    = 0;

            /// The low end of the side along the edge, in cells from the tile origin.
            UInt16 Lower   = 0;

            /// The high end of the side along the edge, in cells from the tile origin.
            UInt16 Upper   = 0;

            /// The index of the side's first link among the tile's links.
            UInt32 First   = 0;

            /// The number of links of the side, zero while no tile lies past the edge.
            UInt32 Count   = 0;
        };

        /// \brief Represents a tile of the chart with its bucket lookup.
        struct Tile final
        {
            /// The column of the tile, in tiles.
            SInt32                    X       = 0;

            /// The row of the tile, in tiles.
            SInt32                    Z       = 0;

            /// The chart index of the tile's first polygon.
            UInt32                    First   = 0;

            /// The number of buckets along x.
            UInt32                    Columns = 0;

            /// The number of buckets along z.
            UInt32                    Rows    = 0;

            /// The baked mesh of the tile.
            Mesh                      Floor;

            /// The index in the items where each bucket starts, plus one entry past the last bucket.
            Sequence<UInt32>          Starts;

            /// The polygons of every bucket, stored bucket after bucket.
            Sequence<UInt16>          Items;

            /// The bounding box of every polygon, in cells from the tile origin.
            Sequence<AnyRect<UInt16>> Boxes;

            /// The polygon sides that lie on the tile edge.
            Sequence<Gate>            Gates;

            /// The polygons of the tiles beside it that the gates lead into, grouped by gate.
            Sequence<Link>            Links;
        };

        /// \brief The offset to the tile past each edge, in tiles, indexed by edge.
        static constexpr Array<IntVector2, 4> kAround =
        {
            IntVector2(-1,  0),
            IntVector2( 0,  1),
            IntVector2( 1,  0),
            IntVector2( 0, -1)
        };

        /// \brief Renumbers the polygons from a slot on and relinks that tile with the four tiles beside it.
        ///
        /// \param Slot The slot of the tile that was added or removed.
        void Mend(UInt32 Slot);

        /// \brief Builds the bucket lookup of a tile and gathers its gates.
        ///
        /// \param Held The tile to build, with its mesh already set.
        void Lay(Ref<Tile> Held);

        /// \brief Links the gates of a tile on one edge to the facing gates of the tile past it.
        ///
        /// \param Slot   The slot of the tile to link.
        /// \param Edge   The edge to link across, from 0 to 3 in \ref kAround order.
        /// \param Beside The slot of the tile past the edge, or \ref kNowhere when there is none.
        void Join(UInt32 Slot, UInt32 Edge, UInt32 Beside);

        /// \brief Gets the floor height of a polygon of a tile under a position.
        ///
        /// \param Held    The tile the polygon belongs to.
        /// \param Polygon The index of the polygon within the tile.
        /// \param Spot    The horizontal position, in cells.
        /// \return The floor height in height steps, clamped between the lowest and highest floor under the polygon.
        Real32 Sample(ConstRef<Tile> Held, UInt32 Polygon, Vector2 Spot) const;

        /// \brief Measures the distance from a position to a polygon of a tile and finds the nearest point on it.
        ///
        /// \param Held    The tile the polygon belongs to.
        /// \param Polygon The index of the polygon within the tile.
        /// \param Spot    The horizontal position to measure from, in cells.
        /// \param Nearest Receives the nearest point of the polygon, the position itself when inside.
        /// \return The squared distance in cells, zero when the position is inside the polygon.
        Real32 Measure(ConstRef<Tile> Held, UInt32 Polygon, Vector2 Spot, Ref<Vector2> Nearest) const;

        /// \brief Packs the coordinates of a tile into its lookup key.
        ///
        /// \param X The column of the tile, in tiles.
        /// \param Z The row of the tile, in tiles.
        /// \return The key, the column in the high half and the row in the low half.
        ZY_INLINE static UInt64 GetKey(SInt32 X, SInt32 Z)
        {
            return (static_cast<UInt64>(static_cast<UInt32>(X)) << 32) | static_cast<UInt32>(Z);
        }

        /// \brief Gets the horizontal position of a mesh corner.
        ///
        /// \param Vertices The corners of the mesh, as column, height and row each.
        /// \param Corner   The index of the corner.
        /// \return The column and row of the corner, in cells from the tile origin.
        ZY_INLINE static Vector2 GetSpot(ConstPtr<UInt16> Vertices, UInt32 Corner)
        {
            return Vector2(Vertices[Corner * 3], Vertices[Corner * 3 + 2]);
        }

        /// \brief Clamps an end of a side on the tile edge to the stretch it shares with a linked polygon.
        ///
        /// \param Spot  The end to clamp, in cells.
        /// \param Link  The link whose stretch bounds the end.
        /// \param Along `true` when the edge runs along x, `false` when it runs along z.
        /// \return The end itself when within the stretch, else the nearest end of the stretch.
        ZY_INLINE static Vector2 Clip(Vector2 Spot, ConstRef<Link> Link, Bool Along)
        {
            if (Along)
            {
                return Vector2(Clamp(Spot.GetX(), Link.Lower, Link.Upper), Spot.GetY());
            }
            return Vector2(Spot.GetX(), Clamp(Spot.GetY(), Link.Lower, Link.Upper));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Real32                mSpan;
        Real32                mRise;
        Real32                mClimb;
        Real32                mCheapest;
        UInt32                mCount;
        Array<Real32, 256>    mCosts;
        Sequence<Tile>        mTiles;
        Sequence<UInt32>      mSpare;
        Sequence<Link>        mLinks;
        Table<UInt64, UInt32> mSlots;
    };
}