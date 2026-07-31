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

#include "Exporter.hpp"
#include "Importer.hpp"
#include "Profile.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Audio
{
    /// \brief Converts one source sound into the engine's native sound format.
    class Baker final
    {
    public:

        /// \brief Bakes a source file into a destination file.
        ///
        /// \param Source      The path of the source sound to read.
        /// \param Destination The path the baked sound is written to.
        /// \param Profile     The settings controlling encoding and compression.
        /// \return `true` if the sound was baked and written, otherwise `false`.
        Bool Bake(Text Source, Text Destination, ConstRef<Profile> Profile) const;

        /// \brief Bakes encoded source bytes into a native sound blob.
        ///
        /// \param Data      The encoded bytes of the source file.
        /// \param Extension The source's extension, which selects the decoder.
        /// \param Profile   The settings controlling encoding and compression.
        /// \return A blob holding the sound file bytes, or an empty blob on failure.
        Blob Bake(ConstRef<Blob> Data, Text Extension, ConstRef<Profile> Profile) const;
    };
}