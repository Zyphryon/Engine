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

#include "Zyphryon.Content/Loader.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    /// \brief Content loader for font assets using the Artery font format.
    class ArteryLoader final : public Loader
    {
    public:

        /// \brief An array with the extension supported by this content loader.
        static constexpr Text kTypes[] = { "artery" };

    public:

        /// \see Loader::Load(Ref<Service>, Ref<Scope>, AnyRef<Blob>)
        Bool Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data) override;

    private:

        /// \brief Header structure for the Artery font format.
        struct ArteryFontHeader final
        {
            Char   Tag[0x10];
            UInt32 Magic;
            UInt32 Version;
            UInt32 Flags;
            UInt32 RealType;
            UInt32 Reserved[4];
            UInt32 MetadataFormat;
            UInt32 MetadataLength;
            UInt32 VariantCount;
            UInt32 VariantsLength;
            UInt32 ImageCount;
            UInt32 ImagesLength;
            UInt32 AppendixCount;
            UInt32 AppendicesLength;
            UInt32 Reserved2[8];
        };

        /// \brief Footer structure for the Artery font format.
        struct ArteryFontFooter final
        {
            UInt32 Salt;
            UInt32 Magic;
            UInt32 Reserved[4];
            UInt32 Length;
            UInt32 Checksum;
        };

        /// \brief Variant header structure for the Artery font format.
        struct ArteryFontVariantHeader final
        {
            UInt32 Flags;
            UInt32 Weight;
            UInt32 CodepointType;
            UInt32 ImageType;
            UInt32 FallbackVariant;
            UInt32 FallbackGlyph;
            UInt32 Reserved[6];
            Real32 Metrics[32];
            UInt32 NameLength;
            UInt32 MetadataLength;
            UInt32 GlyphCount;
            UInt32 KernPairCount;
        };

        /// \brief Image header structure for the Artery font format.
        struct ArteryImageHeader final
        {
            UInt32 Flags;
            UInt32 Encoding;
            UInt32 Width;
            UInt32 Height;
            UInt32 Channels;
            UInt32 PixelFormat;
            UInt32 ImageType;
            UInt32 RowLength;
            SInt32 Orientation;
            UInt32 ChildImages;
            UInt32 TextureFlags;
            UInt32 Reserved[3];
            UInt32 MetadataLength;
            UInt32 DataLength;
        };

        /// \brief Glyph structure for the Artery font format.
        struct ArteryGlyph final
        {
            UInt32 Codepoint;
            UInt32 Image;

            struct {
                Real32 Left, Bottom, Right, Top;
            } PlaneBounds, ImageBounds;

            struct {
                Real32 Horizontal, Vertical;
            } Advance;
        };

        /// \brief Kerning pair structure for the Artery font format.
        struct ArteryKernPair final
        {
            UInt32  Codepoint1, Codepoint2;

            struct {
                Real32 Horizontal, Vertical;
            } Advance;
        };
    };
}