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

#include "Zyphryon.Graphic/Format.hpp"

#if   defined(ZY_PLATFORM_WINDOWS)
#include <glad/gl.h>
#elif defined(ZY_PLATFORM_WEB)
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#endif

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Describes how a \ref VertexFormat feeds a generic vertex attribute.
    struct GLES3Attribute final
    {
        /// Number of components (1..4).
        GLint     Size;

        /// Component data type.
        GLenum    Type;

        /// Whether fixed-point data is normalized on read.
        GLboolean Normalized;

        /// Whether the attribute is consumed as an integer (glVertexAttribIPointer).
        Bool      Integer;
    };

    /// \brief Describes the GLES 3.0 representation of a texture format.
    struct GLES3Format final
    {
        /// Sized internal format.
        GLenum Internal;

        /// Client pixel format (ignored for compressed formats).
        GLenum External;

        /// Client pixel component type (ignored for compressed formats).
        GLenum Type;

        /// `true` when the format is block-compressed.
        Bool   Compressed;
    };

    /// \brief Converts \ref ShaderStage into the matching GLES 3.0 shader type enumeration.
    constexpr GLenum GLES3Convert(ShaderStage Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_VERTEX_SHADER),      // ShaderStage::Vertex
            static_cast<GLenum>(GL_FRAGMENT_SHADER),    // ShaderStage::Fragment
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Storage into the matching GLES 3.0 buffer usage hint.
    constexpr GLenum GLES3Convert(Storage Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_STATIC_DRAW),    // Storage::Immutable
            static_cast<GLenum>(GL_DYNAMIC_DRAW),   // Storage::Dynamic
            static_cast<GLenum>(GL_STREAM_DRAW),    // Storage::Stream
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref BlendFactor into the matching GLES 3.0 enumeration.
    constexpr GLenum GLES3Convert(BlendFactor Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_ZERO),                       // BlendFactor::Zero
            static_cast<GLenum>(GL_ONE),                        // BlendFactor::One
            static_cast<GLenum>(GL_SRC_COLOR),                  // BlendFactor::SrcColor
            static_cast<GLenum>(GL_ONE_MINUS_SRC_COLOR),        // BlendFactor::OneMinusSrcColor
            static_cast<GLenum>(GL_SRC_ALPHA),                  // BlendFactor::SrcAlpha
            static_cast<GLenum>(GL_ONE_MINUS_SRC_ALPHA),        // BlendFactor::OneMinusSrcAlpha
            static_cast<GLenum>(GL_DST_COLOR),                  // BlendFactor::DstColor
            static_cast<GLenum>(GL_ONE_MINUS_DST_COLOR),        // BlendFactor::OneMinusDstColor
            static_cast<GLenum>(GL_DST_ALPHA),                  // BlendFactor::DstAlpha
            static_cast<GLenum>(GL_ONE_MINUS_DST_ALPHA),        // BlendFactor::OneMinusDstAlpha
            static_cast<GLenum>(0x88F9),                        // BlendFactor::Src1Color
            static_cast<GLenum>(0x88FA),                        // BlendFactor::OneMinusSrc1Color
            static_cast<GLenum>(0x8589),                        // BlendFactor::Src1Alpha
            static_cast<GLenum>(0x88FB),                        // BlendFactor::OneMinusSrc1Alpha
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref BlendFunction into the matching GLES 3.0 enumeration.
    constexpr GLenum GLES3Convert(BlendFunction Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_FUNC_ADD),                   // BlendFunction::Add
            static_cast<GLenum>(GL_FUNC_SUBTRACT),              // BlendFunction::Subtract
            static_cast<GLenum>(GL_FUNC_REVERSE_SUBTRACT),      // BlendFunction::ReverseSubtract
            static_cast<GLenum>(GL_MIN),                        // BlendFunction::Min
            static_cast<GLenum>(GL_MAX),                        // BlendFunction::Max
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Cull into the matching GLES 3.0 face enumeration.
    constexpr GLenum GLES3Convert(Cull Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_BACK),                       // Cull::None (unused)
            static_cast<GLenum>(GL_BACK),                       // Cull::Back
            static_cast<GLenum>(GL_FRONT),                      // Cull::Front
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Primitive into the matching GLES 3.0 topology enumeration.
    constexpr GLenum GLES3Convert(Primitive Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_POINTS),                     // Primitive::PointList
            static_cast<GLenum>(GL_LINES),                      // Primitive::LineList
            static_cast<GLenum>(GL_LINE_STRIP),                 // Primitive::LineStrip
            static_cast<GLenum>(GL_TRIANGLES),                  // Primitive::TriangleList
            static_cast<GLenum>(GL_TRIANGLE_STRIP),             // Primitive::TriangleStrip
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TestCondition into the matching GLES 3.0 comparison enumeration.
    constexpr GLenum GLES3Convert(TestCondition Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_ALWAYS),                     // TestCondition::Always
            static_cast<GLenum>(GL_NEVER),                      // TestCondition::Never
            static_cast<GLenum>(GL_GREATER),                    // TestCondition::Greater
            static_cast<GLenum>(GL_GEQUAL),                     // TestCondition::GreaterEqual
            static_cast<GLenum>(GL_EQUAL),                      // TestCondition::Equal
            static_cast<GLenum>(GL_NOTEQUAL),                   // TestCondition::NotEqual
            static_cast<GLenum>(GL_LESS),                       // TestCondition::Less
            static_cast<GLenum>(GL_LEQUAL),                     // TestCondition::LessEqual
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TestAction into the matching GLES 3.0 stencil operation enumeration.
    constexpr GLenum GLES3Convert(TestAction Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_KEEP),                       // TestAction::Keep
            static_cast<GLenum>(GL_REPLACE),                    // TestAction::Replace
            static_cast<GLenum>(GL_ZERO),                       // TestAction::Zero
            static_cast<GLenum>(GL_DECR_WRAP),                  // TestAction::Decrement
            static_cast<GLenum>(GL_DECR),                       // TestAction::DecrementSaturate
            static_cast<GLenum>(GL_INVERT),                     // TestAction::Invert
            static_cast<GLenum>(GL_INCR_WRAP),                  // TestAction::Increment
            static_cast<GLenum>(GL_INCR),                       // TestAction::IncrementSaturate
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TextureAddress into the matching GLES 3.0 wrap enumeration.
    constexpr GLenum GLES3Convert(TextureAddress Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_CLAMP_TO_EDGE),              // TextureAddress::Clamp
            static_cast<GLenum>(0x812D),                        // TextureAddress::Border
            static_cast<GLenum>(GL_REPEAT),                     // TextureAddress::Repeat
            static_cast<GLenum>(GL_MIRRORED_REPEAT),            // TextureAddress::Mirror
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Resolves the GLES 3.0 minification filter for a \ref TextureFilter.
    constexpr GLenum GLES3ConvertMinFilter(TextureFilter Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_NEAREST),                    // Point
            static_cast<GLenum>(GL_NEAREST_MIPMAP_NEAREST),     // PointMipPoint
            static_cast<GLenum>(GL_NEAREST_MIPMAP_LINEAR),      // PointMipLinear
            static_cast<GLenum>(GL_LINEAR),                     // Linear
            static_cast<GLenum>(GL_LINEAR_MIPMAP_NEAREST),      // LinearMipPoint
            static_cast<GLenum>(GL_LINEAR_MIPMAP_LINEAR),       // LinearMipLinear
            static_cast<GLenum>(GL_LINEAR_MIPMAP_LINEAR),       // Anisotropic2x
            static_cast<GLenum>(GL_LINEAR_MIPMAP_LINEAR),       // Anisotropic4x
            static_cast<GLenum>(GL_LINEAR_MIPMAP_LINEAR),       // Anisotropic8x
            static_cast<GLenum>(GL_LINEAR_MIPMAP_LINEAR),       // Anisotropic16x
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Resolves the GLES 3.0 magnification filter for a \ref TextureFilter.
    constexpr GLenum GLES3ConvertMagFilter(TextureFilter Value)
    {
        switch (Value)
        {
        case TextureFilter::Point:
        case TextureFilter::PointMipPoint:
        case TextureFilter::PointMipLinear:
            return GL_NEAREST;
        default:
            return GL_LINEAR;
        }
    }

    /// \brief Resolves the maximum anisotropy requested by a \ref TextureFilter.
    constexpr GLfloat GLES3ConvertAnisotropy(TextureFilter Value)
    {
        switch (Value)
        {
        case TextureFilter::Anisotropic2x:
            return 2.0f;
        case TextureFilter::Anisotropic4x:
            return 4.0f;
        case TextureFilter::Anisotropic8x:
            return 8.0f;
        case TextureFilter::Anisotropic16x:
            return 16.0f;
        default:
            return 1.0f;
        }
    }

    /// \brief Resolves the border color (RGBA) associated with a \ref TextureBorder.
    constexpr ConstPtr<GLfloat> GLES3Convert(TextureBorder Value)
    {
        static constexpr GLfloat kMapping[][4] = {
            { 0.0f, 0.0f, 0.0f, 1.0f },     // OpaqueBlack
            { 1.0f, 1.0f, 1.0f, 1.0f },     // OpaqueWhite
            { 0.0f, 0.0f, 0.0f, 0.0f },     // TransparentBlack
            { 1.0f, 1.0f, 1.0f, 0.0f },     // TransparentWhite
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TextureFormat into its GLES 3.0 internal/external/type triple.
    constexpr GLES3Format GLES3Convert(TextureFormat Value)
    {
        switch (Value)
        {
        case TextureFormat::Unspecified:
            return { 0x0000,                          0,                  0,                                 false };
        case TextureFormat::BC1UIntNorm:
            return { 0x83F1,                          0,                  0,                                 true  };
        case TextureFormat::BC1UIntNorm_sRGB:
            return { 0x8C4D,                          0,                  0,                                 true  };
        case TextureFormat::BC2UIntNorm:
            return { 0x83F2,                          0,                  0,                                 true  };
        case TextureFormat::BC2UIntNorm_sRGB:
            return { 0x8C4E,                          0,                  0,                                 true  };
        case TextureFormat::BC3UIntNorm:
            return { 0x83F3,                          0,                  0,                                 true  };
        case TextureFormat::BC3UIntNorm_sRGB:
            return { 0x8C4F,                          0,                  0,                                 true  };
        case TextureFormat::BC4UIntNorm:
            return { 0x8DBB,                          0,                  0,                                 true  };
        case TextureFormat::BC4SIntNorm:
            return { 0x8DBC,                          0,                  0,                                 true  };
        case TextureFormat::BC5UIntNorm:
            return { 0x8DBD,                          0,                  0,                                 true  };
        case TextureFormat::BC5SIntNorm:
            return { 0x8DBE,                          0,                  0,                                 true  };
        case TextureFormat::BC6UFloat:
            return { 0x8E8F,                          0,                  0,                                 true  };
        case TextureFormat::BC6SFloat:
            return { 0x8E8E,                          0,                  0,                                 true  };
        case TextureFormat::BC7UIntNorm:
            return { 0x8E8C,                          0,                  0,                                 true  };
        case TextureFormat::BC7UIntNorm_sRGB:
            return { 0x8E8D,                          0,                  0,                                 true  };
        case TextureFormat::ETC2RGBUIntNorm:
            return { GL_COMPRESSED_RGB8_ETC2,         0,                  0,                                 true  };
        case TextureFormat::ETC2RGBUIntNorm_sRGB:
            return { GL_COMPRESSED_SRGB8_ETC2,        0,                  0,                                 true  };
        case TextureFormat::ETC2RGBA1UIntNorm:
            return { 0x9276,                          0,                  0,                                 true  };
        case TextureFormat::ETC2RGBA1UIntNorm_sRGB:
            return { 0x9277,                          0,                  0,                                 true  };
        case TextureFormat::ETC2RGBAUIntNorm:
            return { GL_COMPRESSED_RGBA8_ETC2_EAC,    0,                  0,                                 true  };
        case TextureFormat::ETC2RGBAUIntNorm_sRGB:
            return { 0x9279,                          0,                  0,                                 true  };
        case TextureFormat::EACR11UIntNorm:
            return { GL_COMPRESSED_R11_EAC,           0,                  0,                                 true  };
        case TextureFormat::EACR11SIntNorm:
            return { GL_COMPRESSED_SIGNED_R11_EAC,    0,                  0,                                 true  };
        case TextureFormat::EACRG11UIntNorm:
            return { GL_COMPRESSED_RG11_EAC,          0,                  0,                                 true  };
        case TextureFormat::EACRG11SIntNorm:
            return { GL_COMPRESSED_SIGNED_RG11_EAC,   0,                  0,                                 true  };
        case TextureFormat::R8SInt:
            return { GL_R8I,                          GL_RED_INTEGER,     GL_BYTE,                           false };
        case TextureFormat::R8SIntNorm:
            return { GL_R8_SNORM,                     GL_RED,             GL_BYTE,                           false };
        case TextureFormat::R8UInt:
            return { GL_R8UI,                         GL_RED_INTEGER,     GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::R8UIntNorm:
            return { GL_R8,                           GL_RED,             GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::R16SInt:
            return { GL_R16I,                         GL_RED_INTEGER,     GL_SHORT,                          false };
        case TextureFormat::R16SIntNorm:
            return { 0x8F98,                          GL_RED,             GL_SHORT,                          false };
        case TextureFormat::R16UInt:
            return { GL_R16UI,                        GL_RED_INTEGER,     GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::R16UIntNorm:
            return { 0x822A,                          GL_RED,             GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::R16Float:
            return { GL_R16F,                         GL_RED,             GL_HALF_FLOAT,                     false };
        case TextureFormat::R32SInt:
            return { GL_R32I,                         GL_RED_INTEGER,     GL_INT,                            false };
        case TextureFormat::R32UInt:
            return { GL_R32UI,                        GL_RED_INTEGER,     GL_UNSIGNED_INT,                   false };
        case TextureFormat::R32Float:
            return { GL_R32F,                         GL_RED,             GL_FLOAT,                          false };
        case TextureFormat::RG8SInt:
            return { GL_RG8I,                         GL_RG_INTEGER,      GL_BYTE,                           false };
        case TextureFormat::RG8SIntNorm:
            return { GL_RG8_SNORM,                    GL_RG,              GL_BYTE,                           false };
        case TextureFormat::RG8UInt:
            return { GL_RG8UI,                        GL_RG_INTEGER,      GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::RG8UIntNorm:
            return { GL_RG8,                          GL_RG,              GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::RG16SInt:
            return { GL_RG16I,                        GL_RG_INTEGER,      GL_SHORT,                          false };
        case TextureFormat::RG16SIntNorm:
            return { 0x8F99,                          GL_RG,              GL_SHORT,                          false };
        case TextureFormat::RG16UInt:
            return { GL_RG16UI,                       GL_RG_INTEGER,      GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::RG16UIntNorm:
            return { 0x822C,                          GL_RG,              GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::RG16Float:
            return { GL_RG16F,                        GL_RG,              GL_HALF_FLOAT,                     false };
        case TextureFormat::RG32SInt:
            return { GL_RG32I,                        GL_RG_INTEGER,      GL_INT,                            false };
        case TextureFormat::RG32UInt:
            return { GL_RG32UI,                       GL_RG_INTEGER,      GL_UNSIGNED_INT,                   false };
        case TextureFormat::RG32Float:
            return { GL_RG32F,                        GL_RG,              GL_FLOAT,                          false };
        case TextureFormat::RGB32SInt:
            return { GL_RGB32I,                       GL_RGB_INTEGER,     GL_INT,                            false };
        case TextureFormat::RGB32UInt:
            return { GL_RGB32UI,                      GL_RGB_INTEGER,     GL_UNSIGNED_INT,                   false };
        case TextureFormat::RGB32Float:
            return { GL_RGB32F,                       GL_RGB,             GL_FLOAT,                          false };
        case TextureFormat::RGBA8SInt:
            return { GL_RGBA8I,                       GL_RGBA_INTEGER,    GL_BYTE,                           false };
        case TextureFormat::RGBA8SIntNorm:
            return { GL_RGBA8_SNORM,                  GL_RGBA,            GL_BYTE,                           false };
        case TextureFormat::RGBA8UInt:
            return { GL_RGBA8UI,                      GL_RGBA_INTEGER,    GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::RGBA8UIntNorm:
            return { GL_RGBA8,                        GL_RGBA,            GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::RGBA8UIntNorm_sRGB:
            return { GL_SRGB8_ALPHA8,                 GL_RGBA,            GL_UNSIGNED_BYTE,                  false };
        case TextureFormat::RGBA16SInt:
            return { GL_RGBA16I,                      GL_RGBA_INTEGER,    GL_SHORT,                          false };
        case TextureFormat::RGBA16SIntNorm:
            return { 0x8F9B,                          GL_RGBA,            GL_SHORT,                          false };
        case TextureFormat::RGBA16UInt:
            return { GL_RGBA16UI,                     GL_RGBA_INTEGER,    GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::RGBA16UIntNorm:
            return { 0x805B,                          GL_RGBA,            GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::RGBA16Float:
            return { GL_RGBA16F,                      GL_RGBA,            GL_HALF_FLOAT,                     false };
        case TextureFormat::RGBA32SInt:
            return { GL_RGBA32I,                      GL_RGBA_INTEGER,    GL_INT,                            false };
        case TextureFormat::RGBA32UInt:
            return { GL_RGBA32UI,                     GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                   false };
        case TextureFormat::RGBA32Float:
            return { GL_RGBA32F,                      GL_RGBA,            GL_FLOAT,                          false };
        case TextureFormat::RGB10A2UInt:
            return { GL_RGB10_A2UI,                   GL_RGBA_INTEGER,    GL_UNSIGNED_INT_2_10_10_10_REV,    false };
        case TextureFormat::RGB10A2UIntNorm:
            return { GL_RGB10_A2,                     GL_RGBA,            GL_UNSIGNED_INT_2_10_10_10_REV,    false };
        case TextureFormat::D16UIntNorm:
            return { GL_DEPTH_COMPONENT16,            GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 false };
        case TextureFormat::D32Float:
            return { GL_DEPTH_COMPONENT32F,           GL_DEPTH_COMPONENT, GL_FLOAT,                          false };
        case TextureFormat::D24S8UIntNorm:
            return { GL_DEPTH24_STENCIL8,             GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              false };
        case TextureFormat::D32S8UIntNorm:
            return { GL_DEPTH32F_STENCIL8,            GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, false };
        default:
            return { GL_RGBA8,                        GL_RGBA,            GL_UNSIGNED_BYTE,                  false };
        }
    }

    /// \brief Converts \ref VertexFormat into its generic vertex attribute description.
    constexpr GLES3Attribute GLES3Convert(VertexFormat Value)
    {
        const VertexFormatDescription Description = GetFormatDescription(Value);

        GLenum Type;

        if (Description.IsPacked)
        {
            Type = Description.IsSigned ? GL_INT_2_10_10_10_REV : GL_UNSIGNED_INT_2_10_10_10_REV;
        }
        else if (Description.IsFloat)
        {
            Type = (Description.BytesPerComp == 2) ? GL_HALF_FLOAT : GL_FLOAT;
        }
        else
        {
            switch (Description.BytesPerComp)
            {
            case 1:
                Type = Description.IsSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
                break;
            case 2:
                Type = Description.IsSigned ? GL_SHORT : GL_UNSIGNED_SHORT;
                break;
            default:
                Type = Description.IsSigned ? GL_INT : GL_UNSIGNED_INT;
                break;
            }
        }

        const GLboolean Normalized = Description.IsNormalized ? GL_TRUE : GL_FALSE;
        const Bool      Integer    = Description.IsInteger && !Description.IsNormalized && !Description.IsPacked;
        return GLES3Attribute(static_cast<GLint>(Description.Components), Type, Normalized, Integer);
    }

    /// \brief Selects the GLES 3.0 index element type for the given index stride, in bytes.
    constexpr GLenum GLES3ConvertIndex(UInt16 Stride)
    {
        switch (Stride)
        {
        case 1:
            return GL_UNSIGNED_BYTE;
        case 2:
            return GL_UNSIGNED_SHORT;
        default:
            return GL_UNSIGNED_INT;
        }
    }
}
