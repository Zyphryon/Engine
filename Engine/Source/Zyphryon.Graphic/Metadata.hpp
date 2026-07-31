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

#include "Types.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief The properties of a texture format that the engine has to state rather than derive.
    struct TextureMetadata final
    {
        UInt32 BitsPerPixel : 8;    ///< The total number of bits per pixel, or per block when compressed.
        UInt32 BlockSize    : 4;    ///< The edge length of a compression block, or one when uncompressed.
        UInt32 Components   : 3;    ///< The number of colour components.
        UInt32 IsDepth      : 1;    ///< The format carries depth information.
        UInt32 IsStencil    : 1;    ///< The format carries stencil information.
        UInt32 IsSRGB       : 1;    ///< The format is sRGB-encoded.
        UInt32 IsFloat      : 1;    ///< The format stores floating-point values.
        UInt32 IsNormalized : 1;    ///< The format stores values normalized to [0, 1] or [-1, 1].
        UInt32 IsSigned     : 1;    ///< The format can represent negative values.
        UInt32 IsPacked     : 1;    ///< The format packs its components at bit boundaries rather than byte ones.

        /// \brief Checks whether the format stores block-compressed texels.
        ///
        /// \return `true` when texels are grouped into blocks, otherwise `false`.
        constexpr Bool IsCompressed() const
        {
            return BlockSize > 1;
        }

        /// \brief Checks whether the format delivers raw integers rather than a normalized or real range.
        ///
        /// \return `true` when values reach a shader unscaled, otherwise `false`.
        constexpr Bool IsInteger() const
        {
            return Components && !IsFloat && !IsNormalized;
        }

        /// \brief Checks whether the format can carry values outside the unit range.
        ///
        /// \return `true` when the format is a colour format with a real range, otherwise `false`.
        constexpr Bool IsHDR() const
        {
            return IsFloat && !IsDepth;
        }

        /// \brief Checks whether the format can be sampled in a shader.
        ///
        /// \return `true` when the format is a colour format, otherwise `false`.
        constexpr Bool IsSampler() const
        {
            return Components && !IsDepth && !IsStencil;
        }

        /// \brief Gets the width of a single component.
        ///
        /// \note Meaningless for a packed or compressed format, whose components do not share a width.
        ///
        /// \return The number of bits one component occupies.
        constexpr UInt32 BitsPerComponent() const
        {
            return Components ? (BitsPerPixel / Components) : 0;
        }
    };

    /// \brief Gets the metadata of a texture format.
    ///
    /// \param Format The texture format to query.
    /// \return The metadata of the format.
    constexpr TextureMetadata GetTextureMetadata(TextureFormat Format)
    {
        static constexpr TextureMetadata kMetadata[] = {
            // TextureFormat::Unspecified
            { .BitsPerPixel = 0,   .BlockSize = 0, .Components = 0 },
            // TextureFormat::BC1UIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 3, .IsNormalized = 1 },
            // TextureFormat::BC1UIntNorm_sRGB
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 3, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::BC2UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::BC2UIntNorm_sRGB
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::BC3UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::BC3UIntNorm_sRGB
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::BC4UIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 1, .IsNormalized = 1 },
            // TextureFormat::BC4SIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 1, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::BC5UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 2, .IsNormalized = 1 },
            // TextureFormat::BC5SIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 2, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::BC6UFloat
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 3, .IsFloat = 1 },
            // TextureFormat::BC6SFloat
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 3, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::BC7UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::BC7UIntNorm_sRGB
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBUIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 3, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBUIntNorm_sRGB
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 3, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBA1UIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBA1UIntNorm_sRGB
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBAUIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::ETC2RGBAUIntNorm_sRGB
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::EACR11UIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 1, .IsNormalized = 1 },
            // TextureFormat::EACR11SIntNorm
            { .BitsPerPixel = 4,   .BlockSize = 4, .Components = 1, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::EACRG11UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 2, .IsNormalized = 1 },
            // TextureFormat::EACRG11SIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 4, .Components = 2, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::R8SInt
            { .BitsPerPixel = 8,   .BlockSize = 1, .Components = 1, .IsSigned = 1 },
            // TextureFormat::R8SIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 1, .Components = 1, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::R8UInt
            { .BitsPerPixel = 8,   .BlockSize = 1, .Components = 1 },
            // TextureFormat::R8UIntNorm
            { .BitsPerPixel = 8,   .BlockSize = 1, .Components = 1, .IsNormalized = 1 },
            // TextureFormat::R16SInt
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1, .IsSigned = 1 },
            // TextureFormat::R16SIntNorm
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::R16UInt
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1 },
            // TextureFormat::R16UIntNorm
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1, .IsNormalized = 1 },
            // TextureFormat::R16Float
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::R32SInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 1, .IsSigned = 1 },
            // TextureFormat::R32UInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 1 },
            // TextureFormat::R32Float
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 1, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RG8SInt
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 2, .IsSigned = 1 },
            // TextureFormat::RG8SIntNorm
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 2, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::RG8UInt
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 2 },
            // TextureFormat::RG8UIntNorm
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 2, .IsNormalized = 1 },
            // TextureFormat::RG16SInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2, .IsSigned = 1 },
            // TextureFormat::RG16SIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::RG16UInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2 },
            // TextureFormat::RG16UIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2, .IsNormalized = 1 },
            // TextureFormat::RG16Float
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RG32SInt
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 2, .IsSigned = 1 },
            // TextureFormat::RG32UInt
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 2 },
            // TextureFormat::RG32Float
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 2, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RGB32SInt
            { .BitsPerPixel = 96,  .BlockSize = 1, .Components = 3, .IsSigned = 1 },
            // TextureFormat::RGB32UInt
            { .BitsPerPixel = 96,  .BlockSize = 1, .Components = 3 },
            // TextureFormat::RGB32Float
            { .BitsPerPixel = 96,  .BlockSize = 1, .Components = 3, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RGBA8SInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsSigned = 1 },
            // TextureFormat::RGBA8SIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::RGBA8UInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4 },
            // TextureFormat::RGBA8UIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::RGBA8UIntNorm_sRGB
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsSRGB = 1, .IsNormalized = 1 },
            // TextureFormat::RGBA16SInt
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 4, .IsSigned = 1 },
            // TextureFormat::RGBA16SIntNorm
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 4, .IsNormalized = 1, .IsSigned = 1 },
            // TextureFormat::RGBA16UInt
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 4 },
            // TextureFormat::RGBA16UIntNorm
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 4, .IsNormalized = 1 },
            // TextureFormat::RGBA16Float
            { .BitsPerPixel = 64,  .BlockSize = 1, .Components = 4, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RGBA32SInt
            { .BitsPerPixel = 128, .BlockSize = 1, .Components = 4, .IsSigned = 1 },
            // TextureFormat::RGBA32UInt
            { .BitsPerPixel = 128, .BlockSize = 1, .Components = 4 },
            // TextureFormat::RGBA32Float
            { .BitsPerPixel = 128, .BlockSize = 1, .Components = 4, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::RGB10A2UInt
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsPacked = 1 },
            // TextureFormat::RGB10A2UIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 4, .IsNormalized = 1, .IsPacked = 1 },
            // TextureFormat::R11G11B10Float
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 3, .IsPacked = 1 },
            // TextureFormat::RGB9E5Float
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 3, .IsPacked = 1 },
            // TextureFormat::D16UIntNorm
            { .BitsPerPixel = 16,  .BlockSize = 1, .Components = 1, .IsDepth = 1, .IsNormalized = 1 },
            // TextureFormat::D32Float
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 1, .IsDepth = 1, .IsFloat = 1, .IsSigned = 1 },
            // TextureFormat::D24S8UIntNorm
            { .BitsPerPixel = 32,  .BlockSize = 1, .Components = 2, .IsDepth = 1, .IsStencil = 1, .IsNormalized = 1 },
            // TextureFormat::D32S8UIntNorm
            { .BitsPerPixel = 40,  .BlockSize = 1, .Components = 2, .IsDepth = 1, .IsStencil = 1, .IsFloat = 1, .IsSigned = 1 },
        };
        return kMetadata[Enum::Cast(Format)];
    }

    /// \brief The properties of a vertex format that the engine has to state rather than derive.
    struct VertexMetadata final
    {
        UInt16 Components        : 3;   ///< The number of components per vertex element.
        UInt16 BytesPerComponent : 3;   ///< The size of each component, in bytes.
        UInt16 IsFloat           : 1;   ///< The format stores floating-point values.
        UInt16 IsNormalized      : 1;   ///< The format stores values normalized to [0, 1] or [-1, 1].
        UInt16 IsSigned          : 1;   ///< The format can represent negative values.
        UInt16 IsPacked          : 1;   ///< The format packs its components at bit boundaries rather than byte ones.

        /// \brief Checks whether the format delivers raw integers rather than a normalized or real range.
        ///
        /// \return `true` when values reach a shader unscaled, otherwise `false`.
        constexpr Bool IsInteger() const
        {
            return Components && !IsFloat && !IsNormalized;
        }
    };

    /// \brief Gets the metadata of a vertex format.
    ///
    /// \param Format The vertex format to query.
    /// \return The metadata of the format.
    constexpr VertexMetadata GetVertexMetadata(VertexFormat Format)
    {
        static constexpr VertexMetadata kMetadata[] = {
            // VertexFormat::Float16x2
            { .Components = 2, .BytesPerComponent = 2, .IsFloat = 1 },
            // VertexFormat::Float16x4
            { .Components = 4, .BytesPerComponent = 2, .IsFloat = 1 },
            // VertexFormat::Float32x1
            { .Components = 1, .BytesPerComponent = 4, .IsFloat = 1 },
            // VertexFormat::Float32x2
            { .Components = 2, .BytesPerComponent = 4, .IsFloat = 1 },
            // VertexFormat::Float32x3
            { .Components = 3, .BytesPerComponent = 4, .IsFloat = 1 },
            // VertexFormat::Float32x4
            { .Components = 4, .BytesPerComponent = 4, .IsFloat = 1 },
            // VertexFormat::SInt8x4
            { .Components = 4, .BytesPerComponent = 1, .IsSigned = 1 },
            // VertexFormat::SIntNorm8x4
            { .Components = 4, .BytesPerComponent = 1, .IsNormalized = 1, .IsSigned = 1 },
            // VertexFormat::UInt8x4
            { .Components = 4, .BytesPerComponent = 1 },
            // VertexFormat::UIntNorm8x4
            { .Components = 4, .BytesPerComponent = 1, .IsNormalized = 1 },
            // VertexFormat::SInt16x2
            { .Components = 2, .BytesPerComponent = 2, .IsSigned = 1 },
            // VertexFormat::SIntNorm16x2
            { .Components = 2, .BytesPerComponent = 2, .IsNormalized = 1, .IsSigned = 1 },
            // VertexFormat::UInt16x2
            { .Components = 2, .BytesPerComponent = 2 },
            // VertexFormat::UIntNorm16x2
            { .Components = 2, .BytesPerComponent = 2, .IsNormalized = 1 },
            // VertexFormat::SInt16x4
            { .Components = 4, .BytesPerComponent = 2, .IsSigned = 1 },
            // VertexFormat::SIntNorm16x4
            { .Components = 4, .BytesPerComponent = 2, .IsNormalized = 1, .IsSigned = 1 },
            // VertexFormat::UInt16x4
            { .Components = 4, .BytesPerComponent = 2 },
            // VertexFormat::UIntNorm16x4
            { .Components = 4, .BytesPerComponent = 2, .IsNormalized = 1 },
            // VertexFormat::SInt32x1
            { .Components = 1, .BytesPerComponent = 4, .IsSigned = 1 },
            // VertexFormat::UInt32x1
            { .Components = 1, .BytesPerComponent = 4 },
            // VertexFormat::SInt32x2
            { .Components = 2, .BytesPerComponent = 4, .IsSigned = 1 },
            // VertexFormat::UInt32x2
            { .Components = 2, .BytesPerComponent = 4 },
            // VertexFormat::SInt32x3
            { .Components = 3, .BytesPerComponent = 4, .IsSigned = 1 },
            // VertexFormat::UInt32x3
            { .Components = 3, .BytesPerComponent = 4 },
            // VertexFormat::SInt32x4
            { .Components = 4, .BytesPerComponent = 4, .IsSigned = 1 },
            // VertexFormat::UInt32x4
            { .Components = 4, .BytesPerComponent = 4 },
            // VertexFormat::UInt10_10_10_2
            { .Components = 4, .BytesPerComponent = 4, .IsPacked = 1 },
            // VertexFormat::UIntNorm10_10_10_2
            { .Components = 4, .BytesPerComponent = 4, .IsNormalized = 1, .IsPacked = 1 },
        };
        return kMetadata[Enum::Cast(Format)];
    }

    /// \brief Gets the extent of a mip level.
    ///
    /// \param Extent The extent of the base level, in pixels.
    /// \param Level  The zero-based mip level.
    /// \return The extent of the level, never smaller than one pixel.
    constexpr UInt32 GetLevelExtent(UInt16 Extent, UInt8 Level)
    {
        return static_cast<UInt16>(Max(1u, static_cast<UInt32>(Extent) >> Level));
    }

    /// \brief Gets the number of levels in a complete mip chain.
    ///
    /// \param Width  The base width, in pixels.
    /// \param Height The base height, in pixels.
    /// \return The level count down to and including the one-pixel level.
    constexpr UInt8 GetLevelCount(UInt16 Width, UInt16 Height)
    {
        UInt8 Count = 1;

        for (UInt32 Extent = Max(Width, Height); Extent > 1; Extent >>= 1)
        {
            ++Count;
        }
        return Count;
    }

    /// \brief Gets the row pitch of a mip level.
    ///
    /// \param Format The texture format to measure.
    /// \param Width  The base width, in pixels.
    /// \param Level  The zero-based mip level.
    /// \return The distance between consecutive rows of the level, in bytes.
    constexpr UInt32 GetLevelPitch(TextureFormat Format, UInt16 Width, UInt8 Level)
    {
        const TextureMetadata Metadata = GetTextureMetadata(Format);

        const UInt32 Block  = Max(1u, Metadata.BlockSize);
        const UInt32 Extent = GetLevelExtent(Width, Level);
        return ((Extent + Block - 1) / Block) * (Metadata.BitsPerPixel * Block * Block / 8);
    }

    /// \brief Gets the number of rows a mip level occupies.
    ///
    /// \param Format The texture format to measure.
    /// \param Height The base height, in pixels.
    /// \param Level  The zero-based mip level.
    /// \return The row count, counted in blocks for a compressed format.
    constexpr UInt32 GetLevelRows(TextureFormat Format, UInt16 Height, UInt8 Level)
    {
        const UInt32 Block  = Max(1u, GetTextureMetadata(Format).BlockSize);
        const UInt32 Extent = GetLevelExtent(Height, Level);
        return (Extent + Block - 1) / Block;
    }

    /// \brief Gets the tightly-packed size of a single mip level.
    ///
    /// \param Format The texture format to measure.
    /// \param Width  The base width, in pixels.
    /// \param Height The base height, in pixels.
    /// \param Level  The zero-based mip level.
    /// \return The size of the level, in bytes.
    constexpr UInt32 GetLevelSize(TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Level)
    {
        return GetLevelPitch(Format, Width, Level) * GetLevelRows(Format, Height, Level);
    }

    /// \brief Gets the offset of a mip level within a tightly-packed chain.
    ///
    /// \param Format The texture format to measure.
    /// \param Width  The base width, in pixels.
    /// \param Height The base height, in pixels.
    /// \param Level  The zero-based mip level.
    /// \return The distance from the start of the chain to the level, in bytes.
    constexpr UInt32 GetLevelOffset(TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Level)
    {
        UInt32 Offset = 0;

        for (UInt8 Current = 0; Current < Level; ++Current)
        {
            Offset += GetLevelSize(Format, Width, Height, Current);
        }
        return Offset;
    }
}