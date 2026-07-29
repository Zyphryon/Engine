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
#include "Profile.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Texture
{
    /// \brief Writes a decoded bitmap out in the engine's native texture format.
    class Exporter final
    {
    public:

        /// \brief The file extension a baked texture is written with.
        static constexpr Text   kOutput  = "tex";

        /// \brief The four-character code every baked texture starts with, stored little-endian as `ZTEX`.
        static constexpr UInt32 kMagic   = 'Z' | ('T' << 8) | ('E' << 16) | ('X' << 24);

        /// \brief The layout revision of the header this exporter writes.
        static constexpr UInt16 kVersion = 1;

    public:

        /// \brief Checks whether this exporter can write the given format.
        ///
        /// \param Format The texture format to test.
        /// \return `true` if the format is supported, otherwise `false`.
        static Bool IsSupported(Graphic::TextureFormat Format);

        /// \brief Serializes one or more decoded bitmaps into a native texture blob.
        ///
        /// \param Slices  The decoded bitmaps to write, one per array slice or cube face.
        /// \param Layout  The layout the slices compose.
        /// \param Profile The settings controlling format, mip generation, and compression.
        /// \return A blob holding the texture file bytes, or an empty blob on failure.
        static Blob Export(AnyRef<Sequence<Bitmap>> Slices, Graphic::TextureLayout Layout, ConstRef<Profile> Profile);

        /// \brief Serializes a single decoded bitmap into a native texture blob.
        ///
        /// \param Source  The decoded bitmap to write.
        /// \param Profile The settings controlling format, mip generation, and compression.
        /// \return A blob holding the texture file bytes, or an empty blob on failure.
        static Blob Export(AnyRef<Bitmap> Source, ConstRef<Profile> Profile);
    };
}
