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

#include "Bitmap.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Image
{
    /// \brief Cuts one atlas into the bitmaps a cube map or an array texture is assembled from.
    class Slicer final
    {
    public:

        /// \brief The number of faces a cube map carries.
        static constexpr UInt32 kFaces = 6;

        /// \brief How an atlas divides into cells, and the order they are cut in.
        struct Layout final
        {
            /// \brief Where one cell sits, counted in whole cells from the top-left.
            struct Cell final
            {
                /// The cell's column, counted from the left edge.
                UInt16 Column = 0;

                /// The cell's row, counted from the top edge.
                UInt16 Row    = 0;

                /// Whether the cell is turned half a revolution as it is cut.
                Bool   Turned = false;
            };

            /// The number of cells the atlas divides into across.
            UInt16          Columns = 0;

            /// The number of cells the atlas divides into down.
            UInt16          Rows    = 0;

            /// The cells to cut in order, or empty to walk the whole grid row by row.
            ConstSpan<Cell> Cells;

            /// \brief Checks whether the layout describes a division at all.
            ///
            /// \return `true` when the layout divides the atlas, otherwise `false`.
            ZY_INLINE Bool IsValid() const
            {
                return Columns > 0 && Rows > 0;
            }

            /// \brief Gets how many bitmaps the layout cuts an atlas into.
            ///
            /// \return The number of cells the layout emits.
            ZY_INLINE UInt32 GetCount() const
            {
                return Cells.IsEmpty() ? static_cast<UInt32>(Columns) * Rows : Cells.GetSize();
            }

            /// \brief Gets the cell cut at the given position.
            ///
            /// \param Index The position in the emitted order, below \ref GetCount.
            /// \return The cell to cut, walked row by row when the layout carries no explicit order.
            ZY_INLINE Cell GetCell(UInt32 Index) const
            {
                if (Cells.IsEmpty())
                {
                    return Cell(static_cast<UInt16>(Index % Columns), static_cast<UInt16>(Index / Columns), false);
                }
                return Cells[Index];
            }
        };

    public:

        /// \brief Gets the cube arrangement packed in a grid, ordered +X, -X, +Y, -Y, +Z, -Z.
        ///
        /// \param Columns The number of faces across.
        /// \param Rows    The number of faces down.
        /// \return The layout, or an invalid layout when the grid holds no known arrangement.
        static Layout Describe(UInt32 Columns, UInt32 Rows);

        /// \brief Builds a layout that walks an atlas row by row in cells of a fixed extent.
        ///
        /// \param Source The atlas the cells are measured against.
        /// \param Width  The width of one cell, in texels.
        /// \param Height The height of one cell, in texels.
        /// \return The layout, or an invalid layout when the cells do not tile the atlas exactly.
        static Layout Divide(ConstRef<Bitmap> Source, UInt32 Width, UInt32 Height);

        /// \brief Slice an atlas into one bitmap per cell of a layout.
        ///
        /// \param Source The single-level atlas to cut.
        /// \param Layout The division to cut it along.
        /// \return One bitmap per cell, or an empty sequence when the atlas cannot be cut that way.
        static Sequence<Bitmap> Slice(ConstRef<Bitmap> Source, ConstRef<Layout> Layout);
    };
}
