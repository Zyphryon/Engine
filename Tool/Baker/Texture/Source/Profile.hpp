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

namespace Tool::Baker::Texture
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

        /// Treat the source as an atlas holding the six faces of a cube, cut apart before baking.
        Bool                   Cube     = false;

        /// \brief Reads these settings from a parsed command line.
        ///
        /// \param Environment The parsed command line.
        /// \return The resolved settings.
        static Profile From(ConstRef<Environment> Environment)
        {
            Profile Result;
            Result.Format   = Environment.GetEnum<Graphic::TextureFormat>("format", Result.Format);
            Result.Mipmaps  = Environment.GetBool("mipmaps",    Result.Mipmaps);
            Result.Linear   = Environment.GetBool("linear",     Result.Linear);
            Result.Compress = Environment.GetBool("compressed", Result.Compress);
            Result.Cube     = Environment.GetBool("cube",       Result.Cube);

            return Result;
        }
    };
}