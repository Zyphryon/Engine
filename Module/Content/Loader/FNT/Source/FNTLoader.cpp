// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "FNTLoader.hpp"
#include "Zyphryon.Render/Typography/Font.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Content
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool FNTLoader::Load(Ref<Service> Service, Ref<Scope> Scope, AnyRef<Blob> Data)
    {
        UInt32 Cursor = 0;

#define ARTERY_FONT_DECODE_ALIGN()                                                          \
        if (Cursor & 0x03u) Cursor += (0x04u - (Cursor & 0x03u));

#define ARTERY_FONT_DECODE_READ(Target, Size)                                               \
        Target = * reinterpret_cast<Ptr<decltype(Target)>>(Data.GetData<UInt8>() + Cursor); \
        Cursor += Size;

#define ARTERY_FONT_DECODE_READ_PTR(Target, Size)                                           \
        Target = reinterpret_cast<decltype(Target)>(Data.GetData<UInt8>() + Cursor);        \
        Cursor += Size;

#define ARTERY_FONT_DECODE_READ_STRING(Str, Size)                    \
        if (Size > 0)                                                \
        {                                                            \
            Ptr<Char> Characters;                                    \
            ARTERY_FONT_DECODE_READ_PTR(Characters, Size);           \
            ARTERY_FONT_DECODE_ALIGN();                              \
        }

        Render::Font::Metrics FontMetrics;
        Render::Font::Glyphs  FontGlyphs;
        Render::Font::Kerning FontKerning;

        // Read header
        ArteryFontHeader FontHeader;
        ARTERY_FONT_DECODE_READ(FontHeader, sizeof(ArteryFontHeader));

        if (!Compare(FontHeader.Tag, "ARTERY/FONT\0\0\0\0\0", sizeof(FontHeader.Tag)))
        {
            LOG_WARNING("Artery Font: Invalid header detected.");
            return false;
        }
        if (FontHeader.Magic != 0x4D276A5C || FontHeader.RealType != 0x14)
        {
            LOG_WARNING("Artery Font: Non 32-bit float font.");
            return false;
        }

        ARTERY_FONT_DECODE_READ_STRING(nullptr /* Metadata */, FontHeader.MetadataLength);

        if (FontHeader.VariantCount > 1) // TODO: Support more than 1?
        {
            LOG_WARNING("Artery Font: Invalid font with more than 1 font-face.");
            return false;
        }

        // Read variants
        for (SInt Index = 0; Index < FontHeader.VariantCount; ++Index)
        {
            ArteryFontVariantHeader VariantHeader;
            ARTERY_FONT_DECODE_READ(VariantHeader, sizeof(ArteryFontVariantHeader));
            ARTERY_FONT_DECODE_READ_STRING(nullptr /* VariantName */, VariantHeader.NameLength);
            ARTERY_FONT_DECODE_READ_STRING(nullptr /* VariantMetadata */, VariantHeader.MetadataLength);

            Ptr<ArteryGlyph>    Glyphs;
            ARTERY_FONT_DECODE_READ_PTR(Glyphs, sizeof(ArteryGlyph) * VariantHeader.GlyphCount);

            FontGlyphs.Reserve(VariantHeader.GlyphCount);
            for (SInt Element = 0; Element < VariantHeader.GlyphCount; ++Element)
            {
                Ref<ArteryGlyph> Glyph = Glyphs[Element];

                FontGlyphs.Assign(Glyph.Codepoint, Render::Font::Glyph {
                    Rect(Glyph.PlaneBounds.Left, Glyph.PlaneBounds.Bottom, Glyph.PlaneBounds.Right, Glyph.PlaneBounds.Top),
                    Rect(Glyph.ImageBounds.Left, Glyph.ImageBounds.Bottom, Glyph.ImageBounds.Right, Glyph.ImageBounds.Top),
                    Glyph.Advance.Horizontal
                });
            }

            Ptr<ArteryKernPair> KernPairs;
            ARTERY_FONT_DECODE_READ_PTR(KernPairs, sizeof(ArteryKernPair) * VariantHeader.KernPairCount);

            FontKerning.Reserve(VariantHeader.KernPairCount);
            for (SInt Element = 0; Element < VariantHeader.KernPairCount; ++Element)
            {
                Ref<ArteryKernPair> Pair = KernPairs[Element];

                const UInt64 Key = static_cast<UInt64>(Pair.Codepoint1) << 32 | Pair.Codepoint2;
                FontKerning.Assign(Key, Pair.Advance.Horizontal);
            }

            FontMetrics.Size               = VariantHeader.Metrics[0];
            FontMetrics.Distance           = VariantHeader.Metrics[1];
            FontMetrics.Ascender           = VariantHeader.Metrics[3];
            FontMetrics.Descender          = VariantHeader.Metrics[4];
            FontMetrics.UnderlineOffset    = VariantHeader.Metrics[6];
            FontMetrics.UnderlineSize      = VariantHeader.Metrics[5];
            FontMetrics.UnderlineThickness = VariantHeader.Metrics[7];
        }

        // Read images
        Blob   FontAtlasBytes;
        UInt32 FontAtlasWidth;
        UInt32 FontAtlasHeight;

        for (SInt Index = 0; Index < FontHeader.ImageCount; ++Index)
        {
            ArteryImageHeader ImageHeader;
            ARTERY_FONT_DECODE_READ(ImageHeader, sizeof(ArteryImageHeader));
            ARTERY_FONT_DECODE_READ_STRING(nullptr /* ImageMetadata */, ImageHeader.MetadataLength);

            Ptr<UInt8> ImageData;
            ARTERY_FONT_DECODE_READ_PTR(ImageData, ImageHeader.DataLength);
            ARTERY_FONT_DECODE_ALIGN();

            if (ImageHeader.Encoding != 0x01)
            {
                LOG_WARNING("Artery Font: Invalid font image.");
                return false; // Only support binary mode & top down orientation
            }

            FontAtlasBytes = Blob::Allocate<Byte>(ImageHeader.DataLength);
            FontAtlasBytes.Copy(ImageData, ImageHeader.DataLength);

            FontAtlasWidth  = ImageHeader.Width;
            FontAtlasHeight = ImageHeader.Height;
        }

        // Creates the texture atlas for the font.
        const Retainer<Graphic::Image> Atlas = Retainer<Graphic::Image>::Create(
            Str::Join(Scope.GetResource()->GetKey().GetPath(), '#', "Atlas"));
        Atlas->Setup(
            Graphic::TextureLayout::Texture2D,
            Graphic::TextureFormat::RGBA8UIntNorm, FontAtlasWidth, FontAtlasHeight, 1, Move(FontAtlasBytes));
        Atlas->SetPolicy(Resource::Policy::Exclusive);

        // Creates the material for the font.
        constexpr Graphic::Sampler kLinearSampler {
            .Filter = Graphic::TextureFilter::Linear
        };

        Retainer<Graphic::Material> Material = Retainer<Graphic::Material>::Create(
            Str::Join(Scope.GetResource()->GetKey().GetPath(), '#', "Material"));
        Material->SetPolicy(Resource::Policy::Exclusive);
        Material->SetImage(Graphic::TextureSlot::Albedo, Atlas);
        Material->SetSampler(Graphic::TextureSlot::Albedo, kLinearSampler);
        Material->SetParameter(
            "Range"_Hash, Vector2(FontMetrics.Distance / FontAtlasWidth, FontMetrics.Distance / FontAtlasHeight));

        // Normalize Glyphs coordinates.
        const Vector2 AtlasCoordNormalized(1.0f / FontAtlasWidth, 1.0f / FontAtlasHeight);

        for (auto & Iterator : FontGlyphs)
        {
            Ref<Render::Font::Glyph> Element = Iterator.Second;

            Element.AtlasBounds *= AtlasCoordNormalized;
        }

        // Read appendices?
        // Read footer?

        const Retainer<Render::Font> Asset = Retainer<Render::Font>::Cast(Scope.GetResource());
        Asset->Setup(Move(FontMetrics), Move(FontGlyphs), Move(FontKerning), Material);
        return true;
    }
}