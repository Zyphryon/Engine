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

#include "Generator.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Font
{
    /// \brief One glyph's place in the atlas.
    struct Cell final
    {
        /// The codepoint the cell holds.
        UInt32 Codepoint = 0;

        /// The page the cell was placed on.
        UInt32 Page      = 0;

        /// The column the cell starts at, in texels.
        UInt32 X         = 0;

        /// The row the cell starts at, in texels.
        UInt32 Y         = 0;

        /// The generated field, which is empty for a blank such as the space.
        Field  Data;
    };

    /// \brief Places every glyph field into one or more square textures.
    class Atlas final
    {
    public:

        /// \brief The sheets the glyphs were placed on.
        struct Layout final
        {
            /// The side every page shares, in texels.
            UInt32 Side  = 0;

            /// The number of pages the glyphs needed, or zero when they could not be placed at all.
            UInt32 Pages = 0;
        };

    public:

        /// \brief Assigns every cell a page and a position within it.
        ///
        /// A single page grows until it holds everything or reaches \p Limit, and only then does a further page
        /// open. Every page shares one side, so the runtime can treat them as interchangeable textures.
        ///
        /// \param Cells   The cells to place, reordered by descending height.
        /// \param Padding The gap left between neighbouring cells, in texels.
        /// \param Limit   The largest side a page may take, in texels.
        /// \return The resulting layout.
        static Layout Arrange(Ref<Sequence<Cell>> Cells, UInt32 Padding, UInt32 Limit);

        /// \brief Draws the cells belonging to one page into a four-channel texture.
        ///
        /// \param Cells The placed cells.
        /// \param Side  The side of the page, in texels.
        /// \param Page  The page to draw.
        /// \return The interleaved texels of the finished page.
        static Blob Compose(ConstRef<Sequence<Cell>> Cells, UInt32 Side, UInt32 Page);
    };
}