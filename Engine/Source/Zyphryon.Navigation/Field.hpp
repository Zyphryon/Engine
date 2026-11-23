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
    /// \brief Represents one tile and its border as a grid of cells, each with a ground and the slabs over it.
    class Field final
    {
    public:

        /// \brief The area of a ground or slab top that nothing may stand on.
        static constexpr UInt8 kBlocked = 0;

        /// \brief Represents a solid block over the ground of a cell, whose top is a floor of its own.
        struct Slab final
        {
            /// The row-major index of the cell the slab stands over.
            UInt32 Cell   = 0;

            /// The height of the underside in height steps, which caps the headroom of the floor below.
            UInt16 Bottom = 0;

            /// The height of the top in height steps, at or above the underside.
            UInt16 Top    = 0;

            /// The area of the top, or \ref kBlocked when the slab is only an obstacle.
            UInt8  Area   = kBlocked;
        };

    public:

        /// \brief Constructs an empty field.
        ZY_INLINE Field()
            : mWidth { 0 },
              mDepth { 0 }
        {
        }

        /// \brief Resizes the field and clears it, leaving every cell blocked with its ground at zero and no slab.
        ///
        /// \param Width The number of cells along x, border included.
        /// \param Depth The number of cells along z, border included.
        ZY_INLINE void Reset(UInt16 Width, UInt16 Depth)
        {
            mWidth = Width;
            mDepth = Depth;

            mAreas.Resize(static_cast<UInt>(Width) * Depth);
            mFloors.Resize(static_cast<UInt>(Width) * Depth);
            mSlabs.Clear();
        }

        /// \brief Gets the number of cells along x.
        ///
        /// \return The number of cells along x, border included.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the number of cells along z.
        ///
        /// \return The number of cells along z, border included.
        ZY_INLINE UInt16 GetDepth() const
        {
            return mDepth;
        }

        /// \brief Sets the area and height of the ground of one cell.
        ///
        /// \param X     The column of the cell, below the width.
        /// \param Z     The row of the cell, below the height.
        /// \param Area  The area of the ground, or \ref kBlocked when nothing may stand on it.
        /// \param Floor The height of the ground, in height steps.
        ZY_INLINE void SetCell(UInt32 X, UInt32 Z, UInt8 Area, UInt16 Floor)
        {
            const UInt Index = static_cast<UInt>(Z) * mWidth + X;

            mAreas[Index]  = Area;
            mFloors[Index] = Floor;
        }

        /// \brief Adds a solid block over the ground of one cell, such as a piece of bridge or a stair step.
        ///
        /// \param X      The column of the cell, below the width.
        /// \param Z      The row of the cell, below the height.
        /// \param Area   The area of the top, or \ref kBlocked when the slab is only an obstacle.
        /// \param Bottom The height of the underside in height steps, which caps the headroom of the floor below.
        /// \param Top    The height of the top in height steps, at or above the underside.
        ZY_INLINE void AddSlab(UInt32 X, UInt32 Z, UInt8 Area, UInt16 Bottom, UInt16 Top)
        {
            mSlabs.Append(Z * mWidth + X, Bottom, Top, Area);
        }

        /// \brief Gets the area of the ground of every cell, in row-major order.
        ///
        /// \return The areas, one per cell.
        ZY_INLINE ConstSpan<UInt8> GetAreas() const
        {
            return mAreas;
        }

        /// \brief Gets the height of the ground of every cell, in row-major order.
        ///
        /// \return The heights in height steps, one per cell.
        ZY_INLINE ConstSpan<UInt16> GetFloors() const
        {
            return mFloors;
        }

        /// \brief Gets every slab, in the order they were added.
        ///
        /// \return The slabs, empty when the field holds only ground.
        ZY_INLINE ConstSpan<Slab> GetSlabs() const
        {
            return mSlabs;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        UInt16           mWidth;
        UInt16           mDepth;
        Sequence<UInt8>  mAreas;
        Sequence<UInt16> mFloors;
        Sequence<Slab>   mSlabs;
    };
}