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

#include "Importer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture::Codec
{
    /// \brief An \ref Importer backed by `stb_image`, covering the common lossy and lossless source families.
    class STBImporter final : public Importer
    {
    public:

        /// \brief The source file extensions this importer accepts.
        ///
        /// \note Kept in step with the `STBI_ONLY_` set the build enables, so an accepted extension always has
        ///       a decoder behind it. Photoshop, GIF and Softimage are not art-pipeline interchange formats,
        ///       and stb reads sixteen-bit PNM without the byte swap its own specification calls for.
        static constexpr Text kTypes[] =
        {
            "png", "jpg", "jpeg", "tga", "bmp", "hdr"
        };

    public:

        /// \see Importer::GetTypes()
        ZY_INLINE ConstSpan<Text> GetTypes() const override
        {
            return kTypes;
        }

        /// \see Importer::Import(ConstSpan<Byte>, ConstRef<Profile>)
        Bitmap Import(ConstSpan<Byte> Source, ConstRef<Profile> Profile) const override;
    };
}