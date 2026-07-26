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

#include <Zyphryon.Render/Typography/Font.hpp>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Tool::Baker::Font
{
    /// \brief Writes a baked typeface out in the engine's native font format.
    class Exporter final
    {
    public:

        /// \brief The file extension a baked typeface is written with.
        static constexpr Text   kOutput  = "fnt";

        /// \brief The four-character code every baked typeface starts with, stored little-endian as `ZFNT`.
        static constexpr UInt32 kMagic   = 'Z' | ('F' << 8) | ('N' << 16) | ('T' << 24);

        /// \brief The layout revision of the header this exporter writes.
        static constexpr UInt16 kVersion = 1;

        /// \brief The tag of the chunk holding the kerning table.
        static constexpr UInt32 kKerning = 'K' | ('E' << 8) | ('R' << 16) | ('N' << 24);

        /// \brief The tag of the chunk holding the glyph atlas.
        static constexpr UInt32 kAtlas   = 'A' | ('T' << 8) | ('L' << 16) | ('S' << 24);

        /// \brief The four-character code of the native texture format the atlas chunk carries.
        static constexpr UInt32 kTexture = 'Z' | ('T' << 8) | ('E' << 16) | ('X' << 24);

    public:

        /// \brief Serializes a baked typeface into a native font blob.
        ///
        /// \param Metrics The typeface metrics.
        /// \param Glyphs  The glyph table, keyed by codepoint.
        /// \param Kerning The kerning table, which is omitted from the output when empty.
        /// \param Pages   The interleaved four-channel texels of every atlas page, in page order.
        /// \param Side    The side every page shares, in texels.
        /// \return A blob holding the font file bytes, or an empty blob on failure.
        static Blob Export(
            Ref<Render::Font::Metrics> Metrics,
            Ref<Render::Font::Glyphs>  Glyphs,
            Ref<Render::Font::Kerning> Kerning,
            ConstSpan<Blob>            Pages,
            UInt16                     Side);
    };
}
