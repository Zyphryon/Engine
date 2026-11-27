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

#include "Zyphryon.Math/Vector3.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyNavigation
{
    /// \brief Represents the floors a solid lays to be stood on, as a grid of cells in the solid's own space.
    class Relief final
    {
    public:

        /// \brief The floor a cell holds where the solid lays nothing to stand on.
        static constexpr Real32 kNone = -kMaximum<Real32>;

        /// \brief Represents how the floors are worked out of the faces of a solid.
        struct Recipe final
        {
            /// The width of one cell, in the units the faces are given in.
            Real32 Spacing  = 0.25f;

            /// The steepest face a body stands on, as its rise across one unit of ground, or zero for any.
            Real32 Slope    = 1.0f;

            /// The headroom a body needs over a floor, in the units the faces are given in.
            Real32 Headroom = 2.0f;
        };

    public:

        /// \brief Constructs an empty relief.
        ZY_INLINE Relief()
            : mSpacing { 0.0f },
              mWidth   { 0 },
              mDepth   { 0 }
        {
        }

        /// \brief Bakes the faces of a solid into floors, each cell taking the lowest face with headroom over it.
        ///
        /// \param Faces  The corners of every face in the solid's own space, three a face, clockwise from its front.
        /// \param Recipe The settings the faces are baked with.
        void Bake(ConstSpan<Vector3> Faces, ConstRef<Recipe> Recipe);

        /// \brief Gets the corner the cells are counted from.
        ///
        /// \return The corner of cell zero, in the solid's own space, across its x and z axes.
        ZY_INLINE Vector2 GetCorner() const
        {
            return mCorner;
        }

        /// \brief Gets the width of one cell.
        ///
        /// \return The width, in the units the faces were given in, zero before the first bake.
        ZY_INLINE Real32 GetSpacing() const
        {
            return mSpacing;
        }

        /// \brief Gets the number of cells along x.
        ///
        /// \return The width of the relief, in cells.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the number of cells along z.
        ///
        /// \return The depth of the relief, in cells.
        ZY_INLINE UInt16 GetDepth() const
        {
            return mDepth;
        }

        /// \brief Gets the floor of one cell.
        ///
        /// \param X The column of the cell, below the width.
        /// \param Z The row of the cell, below the depth.
        /// \return The height of the floor in the solid's own space, or \ref kNone where the cell lays none.
        ZY_INLINE Real32 GetFloor(UInt32 X, UInt32 Z) const
        {
            return mFloors[Z * mWidth + X];
        }

        /// \brief Gets the floor of every cell, in row-major order.
        ///
        /// \return The heights in the solid's own space, one per cell, \ref kNone where a cell lays none.
        ZY_INLINE ConstSpan<Real32> GetFloors() const
        {
            return mFloors;
        }

        /// \brief Gets the floor height under a position, eased between the floors of the cells about it.
        ///
        /// \param Spot  The horizontal position, in the solid's own space.
        /// \param Climb The highest step between two neighbouring floors that is eased, so a wall stays a wall.
        /// \return The floor height in the solid's own space, or \ref kNone where the solid lays none.
        Real32 GetHeight(Vector2 Spot, Real32 Climb) const;

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mCorner);
            Archive.Serialize(mSpacing);
            Archive.Serialize(mWidth);
            Archive.Serialize(mDepth);
            Archive.Serialize(mFloors);
        }

    private:

        /// \brief The points a cell is read at along each of its sides, so a floor narrower than a cell is found.
        static constexpr UInt32 kPoints = 3;

        /// \brief The height two faces lie within to count as one, as both sides of a thin board do.
        static constexpr Real32 kSkin   = 0.05f;

        /// \brief Represents a face found over a point of the grid.
        struct Hit final
        {
            /// The point of the grid the face lies over, in row-major order.
            UInt32 Point;

            /// The height the face lies at over the point, in the solid's own space.
            Real32 Height;

            /// `true` when the face is turned up and level enough to stand on, `false` otherwise.
            Bool   Flat;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector2          mCorner;
        Real32           mSpacing;
        UInt16           mWidth;
        UInt16           mDepth;
        Sequence<Real32> mFloors;
    };
}