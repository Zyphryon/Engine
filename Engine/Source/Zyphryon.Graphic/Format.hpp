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
    /// \brief Describes the properties of a texture format.
    struct TextureFormatDescription final
    {
        UInt32 BitsPerPixel   : 8;  ///< The total number of bits per pixel.
        UInt32 BlockSize      : 8;  ///< The size of compression block.
        UInt32 Components     : 8;  ///< The number of color components.
        UInt32 IsDepth        : 1;  ///< The format contains depth information.
        UInt32 IsStencil      : 1;  ///< The format contains stencil information.
        UInt32 IsSRGB         : 1;  ///< The format uses sRGB gamma correction.
        UInt32 IsFloat        : 1;  ///< The format uses floating-point representation.
        UInt32 IsInteger      : 1;  ///< The format uses integer representation.
        UInt32 IsNormalized   : 1;  ///< The format values are normalized to [0, 1] or [-1, 1].
        UInt32 IsHDR          : 1;  ///< The format supports high dynamic range values.
        UInt32 IsSampler      : 1;  ///< The format can be sampled in shaders.

        constexpr Bool IsCompressed() const
        {
            return BlockSize > 1;
        }
    };

    /// \brief Gets the description for a texture format.
    ///
    /// \param Format The texture format to query.
    /// \return The description containing format properties.
    constexpr TextureFormatDescription GetFormatDescription(Graphic::TextureFormat Format)
    {
        static constexpr TextureFormatDescription kMetadata[] = {
            { 0,   0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },  // TextureFormat::Unspecified
            { 4,   4, 3, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC1UIntNorm
            { 4,   4, 3, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::BC1UIntNorm_sRGB
            { 8,   4, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC2UIntNorm
            { 8,   4, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::BC2UIntNorm_sRGB
            { 8,   4, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC3UIntNorm
            { 8,   4, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::BC3UIntNorm_sRGB
            { 4,   4, 1, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC4UIntNorm
            { 4,   4, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::BC4SIntNorm
            { 8,   4, 2, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC5UIntNorm
            { 8,   4, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::BC5SIntNorm
            { 8,   4, 3, 0, 0, 0, 1, 0, 1, 1, 1 },  // TextureFormat::BC6UFloat
            { 8,   4, 3, 0, 0, 0, 1, 0, 1, 1, 1 },  // TextureFormat::BC6SFloat
            { 8,   4, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::BC7UIntNorm
            { 8,   4, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::BC7UIntNorm_sRGB
            { 4,   4, 3, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBUIntNorm
            { 4,   4, 3, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBUIntNorm_sRGB
            { 4,   4, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBA1UIntNorm
            { 4,   4, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBA1UIntNorm_sRGB
            { 8,   4, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBAUIntNorm
            { 8,   4, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::ETC2RGBAUIntNorm_sRGB
            { 4,   4, 1, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::EACR11UIntNorm
            { 4,   4, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::EACR11SIntNorm
            { 8,   4, 2, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::EACRG11UIntNorm
            { 8,   4, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::EACRG11SIntNorm
            { 8,   1, 1, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::R8SInt
            { 8,   1, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::R8SIntNorm
            { 8,   1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::R8UInt
            { 8,   1, 1, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::R8UIntNorm
            { 16,  1, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::R16SInt
            { 16,  1, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::R16SIntNorm
            { 16,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::R16UInt
            { 16,  1, 1, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::R16UIntNorm
            { 16,  1, 1, 0, 0, 0, 1, 0, 0, 0, 1 },  // TextureFormat::R16Float
            { 32,  1, 1, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::R32SInt
            { 32,  1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::R32UInt
            { 32,  1, 1, 0, 0, 0, 1, 0, 0, 0, 1 },  // TextureFormat::R32Float
            { 16,  1, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RG8SInt
            { 16,  1, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RG8SIntNorm
            { 16,  1, 2, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RG8UInt
            { 16,  1, 2, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::RG8UIntNorm
            { 32,  1, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RG16SInt
            { 32,  1, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RG16SIntNorm
            { 32,  1, 2, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RG16UInt
            { 32,  1, 2, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::RG16UIntNorm
            { 32,  1, 2, 0, 0, 0, 1, 0, 0, 0, 1 },  // TextureFormat::RG16Float
            { 64,  1, 2, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RG32SInt
            { 64,  1, 2, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RG32UInt
            { 64,  1, 2, 0, 0, 0, 1, 0, 0, 0, 1 },  // TextureFormat::RG32Float
            { 96,  1, 3, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGB32SInt
            { 96,  1, 3, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RGB32UInt
            { 96,  1, 3, 0, 0, 0, 1, 0, 0, 0, 1 },  // TextureFormat::RGB32Float
            { 32,  1, 4, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGBA8SInt
            { 32,  1, 4, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGBA8SIntNorm
            { 32,  1, 4, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RGBA8UInt
            { 32,  1, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::RGBA8UIntNorm
            { 32,  1, 4, 0, 0, 1, 0, 0, 1, 0, 1 },  // TextureFormat::RGBA8UIntNorm_sRGB
            { 64,  1, 4, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGBA16SInt
            { 64,  1, 4, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGBA16SIntNorm
            { 64,  1, 4, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RGBA16UInt
            { 64,  1, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::RGBA16UIntNorm
            { 64,  1, 4, 0, 0, 0, 1, 0, 0, 1, 1 },  // TextureFormat::RGBA16Float
            { 128, 1, 4, 0, 0, 0, 0, 0, 1, 1, 1 },  // TextureFormat::RGBA32SInt
            { 128, 1, 4, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RGBA32UInt
            { 128, 1, 4, 0, 0, 0, 1, 0, 0, 1, 1 },  // TextureFormat::RGBA32Float
            { 32,  1, 4, 0, 0, 0, 0, 0, 0, 0, 1 },  // TextureFormat::RGB10A2UInt
            { 32,  1, 4, 0, 0, 0, 0, 0, 1, 0, 1 },  // TextureFormat::RGB10A2UIntNorm
            { 16,  1, 1, 1, 0, 0, 0, 0, 1, 0, 0 },  // TextureFormat::D16UIntNorm
            { 32,  1, 1, 1, 0, 0, 1, 0, 0, 0, 0 },  // TextureFormat::D32Float
            { 32,  1, 2, 1, 1, 0, 0, 0, 1, 0, 0 },  // TextureFormat::D24S8UIntNorm
            { 40,  1, 2, 1, 1, 0, 1, 0, 0, 0, 0 },  // TextureFormat::D32S8UIntNorm
        };
        return kMetadata[Enum::Cast(Format)];
    }

    /// \brief Describes the properties of a vertex format.
    struct VertexFormatDescription final
    {
        UInt16 Components     : 4;  ///< The number of components per vertex element.
        UInt16 BytesPerComp   : 4;  ///< The size in bytes of each component.
        UInt16 IsFloat        : 1;  ///< The format uses floating-point representation.
        UInt16 IsInteger      : 1;  ///< The format uses integer representation.
        UInt16 IsNormalized   : 1;  ///< The format values are normalized to [0, 1] or [-1, 1].
        UInt16 IsSigned       : 1;  ///< The format uses signed values.
        UInt16 IsPacked       : 1;  ///< The format uses packed representation.
    };

    /// \brief Gets the description for a vertex format.
    ///
    /// \param Format The vertex format to query.
    /// \return The description containing format properties.
    constexpr VertexFormatDescription GetFormatDescription(Graphic::VertexFormat Format)
    {
        static constexpr VertexFormatDescription kMetadata[] = {
            { 2, 2, 1, 0, 0, 0, 0 },  // VertexFormat::Float16x2
            { 4, 2, 1, 0, 0, 0, 0 },  // VertexFormat::Float16x4
            { 1, 4, 1, 0, 0, 0, 0 },  // VertexFormat::Float32x1
            { 2, 4, 1, 0, 0, 0, 0 },  // VertexFormat::Float32x2
            { 3, 4, 1, 0, 0, 0, 0 },  // VertexFormat::Float32x3
            { 4, 4, 1, 0, 0, 0, 0 },  // VertexFormat::Float32x4
            { 4, 1, 0, 1, 0, 1, 0 },  // VertexFormat::SInt8x4
            { 4, 1, 0, 1, 1, 1, 0 },  // VertexFormat::SIntNorm8x4
            { 4, 1, 0, 1, 0, 0, 0 },  // VertexFormat::UInt8x4
            { 4, 1, 0, 1, 1, 0, 0 },  // VertexFormat::UIntNorm8x4
            { 2, 2, 0, 1, 0, 1, 0 },  // VertexFormat::SInt16x2
            { 2, 2, 0, 1, 1, 1, 0 },  // VertexFormat::SIntNorm16x2
            { 2, 2, 0, 1, 0, 0, 0 },  // VertexFormat::UInt16x2
            { 2, 2, 0, 1, 1, 0, 0 },  // VertexFormat::UIntNorm16x2
            { 4, 2, 0, 1, 0, 1, 0 },  // VertexFormat::SInt16x4
            { 4, 2, 0, 1, 1, 1, 0 },  // VertexFormat::SIntNorm16x4
            { 4, 2, 0, 1, 0, 0, 0 },  // VertexFormat::UInt16x4
            { 4, 2, 0, 1, 1, 0, 0 },  // VertexFormat::UIntNorm16x4
            { 1, 4, 0, 1, 0, 1, 0 },  // VertexFormat::SInt32x1
            { 1, 4, 0, 1, 0, 0, 0 },  // VertexFormat::UInt32x1
            { 2, 4, 0, 1, 0, 1, 0 },  // VertexFormat::SInt32x2
            { 2, 4, 0, 1, 0, 0, 0 },  // VertexFormat::UInt32x2
            { 3, 4, 0, 1, 0, 1, 0 },  // VertexFormat::SInt32x3
            { 3, 4, 0, 1, 0, 0, 0 },  // VertexFormat::UInt32x3
            { 4, 4, 0, 1, 0, 1, 0 },  // VertexFormat::SInt32x4
            { 4, 4, 0, 1, 0, 0, 0 },  // VertexFormat::UInt32x4
            { 4, 4, 0, 1, 0, 0, 1 },  // VertexFormat::UInt10_10_10_2
            { 4, 4, 0, 1, 1, 0, 1 },  // VertexFormat::UIntNorm10_10_10_2
        };
        return kMetadata[Enum::Cast(Format)];
    }
}