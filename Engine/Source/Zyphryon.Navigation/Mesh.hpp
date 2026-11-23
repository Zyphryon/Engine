// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    /// \brief Represents the walkable floor of one tile as convex polygons over shared vertices, packed in one block.
    class Mesh final
    {
    public:

        /// \brief The most sides a polygon has, which is also the most corners.
        static constexpr UInt32 kSides  = 6;

        /// \brief The value of an unused corner, and of a side that is a wall.
        static constexpr UInt16 kNone   = 0xFFFF;

        /// \brief The bit set on a side that lies on the tile edge, with the index of that edge in the bits below.
        static constexpr UInt16 kPortal = 0x8000;

    public:

        /// \brief Constructs an empty mesh.
        ZY_INLINE Mesh()
            : mVertices { 0 },
              mPolygons { 0 }
        {
        }

        /// \brief Empties the mesh, keeping its memory for the next tile.
        ZY_INLINE void Clear()
        {
            mBlock.Clear();
            mVertices = 0;
            mPolygons = 0;
        }

        /// \brief Adds a vertex that polygons may share, before \ref Lay is called.
        ///
        /// \param X The column of the vertex, in cells from the tile origin.
        /// \param Y The height of the vertex, in height steps.
        /// \param Z The row of the vertex, in cells from the tile origin.
        /// \return The index of the vertex, below \ref kNone.
        ZY_INLINE UInt16 AddVertex(UInt16 X, UInt16 Y, UInt16 Z)
        {
            ZY_ASSERT(mVertices < kNone, "A tile holds at most 65535 corners, so bake it smaller");

            mBlock.Append(Array(X, Y, Z));
            return static_cast<UInt16>(mVertices++);
        }

        /// \brief Appends the polygons after the vertices, every corner unused, every side a wall, every area and height zero.
        ///
        /// \param Count The number of polygons, at most \ref kPortal so no polygon index carries the portal bit.
        ZY_INLINE void Lay(UInt32 Count)
        {
            ZY_ASSERT(Count <= kPortal, "A tile holds at most 32768 polygons, so bake it smaller");

            mPolygons = Count;

            mBlock.Fill(kNone, static_cast<UInt>(Count) * kSides * 2);
            mBlock.Fill(0, static_cast<UInt>(Count) * 3);
        }

        /// \brief Sets one corner of a polygon and what lies past the side starting at it, after \ref Lay.
        ///
        /// \param Index  The polygon to write, below \ref GetCount.
        /// \param Side   The corner to write, below \ref kSides.
        /// \param Corner The index of the vertex at the corner.
        /// \param Past   The polygon past the side, \ref kPortal with the edge index on a tile edge, or \ref kNone for a wall.
        ZY_INLINE void SetSide(UInt32 Index, UInt32 Side, UInt16 Corner, UInt16 Past)
        {
            const UInt Slot = static_cast<UInt>(mVertices) * 3 + static_cast<UInt>(Index) * kSides * 2 + Side;

            mBlock[Slot]            = Corner;
            mBlock[Slot + kSides] = Past;
        }

        /// \brief Sets the area of a polygon, after \ref Lay.
        ///
        /// \param Index The polygon to write, below \ref GetCount.
        /// \param Area  The area to set.
        ZY_INLINE void SetArea(UInt32 Index, UInt8 Area)
        {
            mBlock[GetTail() + Index] = Area;
        }

        /// \brief Sets the lowest and highest floor height under a polygon, after \ref Lay.
        ///
        /// \param Index The polygon to write, below \ref GetCount.
        /// \param Low   The lowest floor height under the polygon, in height steps.
        /// \param High  The highest floor height under the polygon, in height steps.
        ZY_INLINE void SetLevel(UInt32 Index, UInt16 Low, UInt16 High)
        {
            const UInt Slot = GetTail() + mPolygons + static_cast<UInt>(Index) * 2;

            mBlock[Slot]     = Low;
            mBlock[Slot + 1] = High;
        }

        /// \brief Gets every vertex.
        ///
        /// \return The vertices as column, height and row triples.
        ZY_INLINE ConstSpan<UInt16> GetVertices() const
        {
            return ConstSpan<UInt16>(mBlock).Slice(0, static_cast<UInt>(mVertices) * 3);
        }

        /// \brief Gets every polygon.
        ///
        /// \return The polygons, each \ref kSides vertex indices followed by what lies past each of its sides.
        ZY_INLINE ConstSpan<UInt16> GetPolygons() const
        {
            const UInt First = static_cast<UInt>(mVertices) * 3;

            return ConstSpan<UInt16>(mBlock).Slice(First, GetTail() - First);
        }

        /// \brief Gets one polygon.
        ///
        /// \param Index The polygon to read, below \ref GetCount.
        /// \return The \ref kSides vertex indices of the polygon, followed by what lies past each of its sides.
        ZY_INLINE ConstSpan<UInt16> GetPolygon(UInt32 Index) const
        {
            const UInt First = static_cast<UInt>(mVertices) * 3 + static_cast<UInt>(Index) * kSides * 2;

            return ConstSpan<UInt16>(mBlock).Slice(First, kSides * 2);
        }

        /// \brief Gets the area of every polygon.
        ///
        /// \return The areas, one per polygon.
        ZY_INLINE ConstSpan<UInt16> GetAreas() const
        {
            return ConstSpan<UInt16>(mBlock).Slice(GetTail(), mPolygons);
        }

        /// \brief Gets the lowest and highest floor height under every polygon.
        ///
        /// \return The heights in height steps, the lowest then the highest for each polygon.
        ZY_INLINE ConstSpan<UInt16> GetLevels() const
        {
            return ConstSpan<UInt16>(mBlock).Slice(GetTail() + mPolygons, static_cast<UInt>(mPolygons) * 2);
        }

        /// \brief Gets the lowest and highest floor height under one polygon.
        ///
        /// \param Index The polygon to read, below \ref GetCount.
        /// \return The lowest then the highest floor height, in height steps.
        ZY_INLINE ConstSpan<UInt16> GetLevel(UInt32 Index) const
        {
            return GetLevels().Slice(static_cast<UInt>(Index) * 2, 2);
        }

        /// \brief Gets the number of vertices.
        ///
        /// \return The number of vertices.
        ZY_INLINE UInt32 GetCorners() const
        {
            return mVertices;
        }

        /// \brief Gets the number of polygons.
        ///
        /// \return The number of polygons.
        ZY_INLINE UInt32 GetCount() const
        {
            return mPolygons;
        }

    private:

        /// \brief Gets where the per-polygon areas and heights start in the block, after the vertices and polygons.
        ///
        /// \return The offset of the first area in the block.
        ZY_INLINE UInt GetTail() const
        {
            return static_cast<UInt>(mVertices) * 3 + static_cast<UInt>(mPolygons) * kSides * 2;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<UInt16> mBlock;
        UInt32           mVertices;
        UInt32           mPolygons;
    };
}