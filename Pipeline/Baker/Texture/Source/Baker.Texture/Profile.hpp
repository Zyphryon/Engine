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

#include <Zyphryon.Graphic/Types.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Pipeline::Baker::Texture
{
    /// \brief Settings that control how a source bitmap is baked into the engine's native texture format.
    struct Profile final
    {
        /// The target texture format; `Unspecified` infers one from the source's channel count.
        Graphic::TextureFormat Format   = Graphic::TextureFormat::Unspecified;

        /// Generate a full mip chain down to 1x1; when `false` only the base level is kept.
        Bool                   Mipmaps  = false;

        /// Treat the source as linear-encoded colour, selecting a linear texture format.
        Bool                   Linear   = true;

        /// LZ4-compress the pixel payload.
        Bool                   Compress = true;

        /// \brief A pair of numbers naming how an atlas divides, left zeroed when it was not asked for.
        struct Extent final
        {
            /// The measurement across.
            UInt16 Width  = 0;

            /// The measurement down.
            UInt16 Height = 0;

            /// \brief Checks whether the pair names a division at all.
            ///
            /// \return `true` when both axes carry a measurement, otherwise `false`.
            ZY_INLINE Bool IsValid() const
            {
                return Width > 0 && Height > 0;
            }
        };

        /// The grid a cube atlas packs its six faces in, counted in faces.
        Extent                 Cube;

        /// The extent of one slice of an array atlas, counted in texels.
        Extent                 Slice;

        /// \brief Reads these settings from a parsed command line.
        ///
        /// \param Environment The parsed command line.
        /// \return The resolved settings.
        static Profile From(ConstRef<Environment> Environment);
    };
}