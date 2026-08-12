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

namespace Pipeline::Baker::Texture
{
    /// \brief Represents the bitmaps one source decodes into, and the layout they compose.
    struct Surface final
    {
        /// The layout the slices compose.
        Graphic::TextureLayout Layout = Graphic::TextureLayout::Texture2D;

        /// One bitmap per array slice or cube face, in the order the layout addresses them.
        Sequence<Bitmap>       Slices;

        /// \brief Checks whether the decode produced anything at all.
        ///
        /// \return `true` when the surface carries at least one slice with pixels, otherwise `false`.
        ZY_INLINE Bool IsValid() const
        {
            return !Slices.IsEmpty() && !Slices.GetFront().GetPixels().IsEmpty();
        }

        /// \brief Wraps a single bitmap as a flat surface holding it as its only slice.
        ///
        /// \param Source The bitmap the surface takes over.
        /// \return A surface holding \p Source as a plain 2D texture.
        ZY_INLINE static Surface From(AnyRef<Bitmap> Source)
        {
            Surface Result;
            Result.Slices.Append(Move(Source));
            return Result;
        }
    };
}