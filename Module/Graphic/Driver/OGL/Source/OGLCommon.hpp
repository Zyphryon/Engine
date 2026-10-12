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

#include "OGLApi.hpp"
#include "Zyphryon.Graphic/Metadata.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Describes how a \ref VertexFormat feeds a generic vertex attribute.
    struct AttributeFormat final
    {
        /// The number of components, one to four.
        GLint     Size;

        /// The component data type.
        GLenum    Type;

        /// `true` when fixed-point data is normalized on read.
        GLboolean Normalized;

        /// `true` when the attribute is read as an integer, through glVertexAttribIPointer.
        Bool      Integer;
    };

    /// \brief Describes the OpenGL representation of a texture format.
    struct Format final
    {
        /// The sized internal format.
        GLenum Internal;

        /// The client pixel format, ignored for compressed formats.
        GLenum External;

        /// The client component type, ignored for compressed formats.
        GLenum Type;

        /// `true` when the format is block-compressed.
        Bool   Compressed;
    };

    /// \brief Converts \ref ShaderStage into the matching OpenGL shader type enumeration.
    constexpr GLenum Convert(ShaderStage Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_VERTEX_SHADER),      // ShaderStage::Vertex
            static_cast<GLenum>(GL_FRAGMENT_SHADER),    // ShaderStage::Fragment
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref Storage into the matching OpenGL buffer usage hint.
    constexpr GLenum Convert(Storage Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_STATIC_DRAW),    // Storage::Immutable
            static_cast<GLenum>(GL_DYNAMIC_DRAW),   // Storage::Dynamic
            static_cast<GLenum>(GL_STREAM_DRAW),    // Storage::Stream
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref BlendFactor into the matching OpenGL enumeration.
    constexpr GLenum Convert(BlendFactor Value)
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
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref BlendFunction into the matching OpenGL enumeration.
    constexpr GLenum Convert(BlendFunction Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_FUNC_ADD),                   // BlendFunction::Add
            static_cast<GLenum>(GL_FUNC_SUBTRACT),              // BlendFunction::Subtract
            static_cast<GLenum>(GL_FUNC_REVERSE_SUBTRACT),      // BlendFunction::ReverseSubtract
            static_cast<GLenum>(GL_MIN),                        // BlendFunction::Min
            static_cast<GLenum>(GL_MAX),                        // BlendFunction::Max
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref Cull into the matching OpenGL face enumeration.
    constexpr GLenum Convert(Cull Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_BACK),                       // Cull::None (unused)
            static_cast<GLenum>(GL_BACK),                       // Cull::Back
            static_cast<GLenum>(GL_FRONT),                      // Cull::Front
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref Primitive into the matching OpenGL topology enumeration.
    constexpr GLenum Convert(Primitive Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_POINTS),                     // Primitive::PointList
            static_cast<GLenum>(GL_LINES),                      // Primitive::LineList
            static_cast<GLenum>(GL_LINE_STRIP),                 // Primitive::LineStrip
            static_cast<GLenum>(GL_TRIANGLES),                  // Primitive::TriangleList
            static_cast<GLenum>(GL_TRIANGLE_STRIP),             // Primitive::TriangleStrip
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref Usage into the matching OpenGL buffer binding target.
    constexpr GLenum Convert(Usage Value)
    {
        if (HasBit(Value, Usage::Vertex))
        {
            return GL_ARRAY_BUFFER;
        }
        if (HasBit(Value, Usage::Index))
        {
            return GL_ELEMENT_ARRAY_BUFFER;
        }
        if (HasBit(Value, Usage::Uniform))
        {
            return GL_UNIFORM_BUFFER;
        }
        return 0;
    }

    /// \brief Converts \ref TestCondition into the matching OpenGL comparison enumeration.
    constexpr GLenum Convert(TestCondition Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_ALWAYS),                     // TestCondition::None
            static_cast<GLenum>(GL_ALWAYS),                     // TestCondition::Always
            static_cast<GLenum>(GL_NEVER),                      // TestCondition::Never
            static_cast<GLenum>(GL_GREATER),                    // TestCondition::Greater
            static_cast<GLenum>(GL_GEQUAL),                     // TestCondition::GreaterEqual
            static_cast<GLenum>(GL_EQUAL),                      // TestCondition::Equal
            static_cast<GLenum>(GL_NOTEQUAL),                   // TestCondition::NotEqual
            static_cast<GLenum>(GL_LESS),                       // TestCondition::Less
            static_cast<GLenum>(GL_LEQUAL),                     // TestCondition::LessEqual
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref TestAction into the matching OpenGL stencil operation enumeration.
    constexpr GLenum Convert(TestAction Value)
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
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref TextureAddress into the matching OpenGL wrap enumeration.
    constexpr GLenum Convert(TextureAddress Value)
    {
        static constexpr Array kMapping = {
            static_cast<GLenum>(GL_CLAMP_TO_EDGE),              // TextureAddress::Clamp
            static_cast<GLenum>(GL_CLAMP_TO_BORDER),            // TextureAddress::Border
            static_cast<GLenum>(GL_REPEAT),                     // TextureAddress::Repeat
            static_cast<GLenum>(GL_MIRRORED_REPEAT),            // TextureAddress::Mirror
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref TextureAddress into the matching OpenGL wrap enumeration, for a context with or without border clamping.
    constexpr GLenum ConvertAddress(TextureAddress Value, Bool Border)
    {
        // Without border clamping the nearest a context can do is repeat the edge texel.
        return (Value == TextureAddress::Border && !Border) ? GL_CLAMP_TO_EDGE : Convert(Value);
    }

    /// \brief Resolves the OpenGL minification filter for a \ref TextureFilter.
    constexpr GLenum ConvertMinFilter(TextureFilter Value)
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
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Resolves the OpenGL magnification filter for a \ref TextureFilter.
    constexpr GLenum ConvertMagFilter(TextureFilter Value)
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
    constexpr GLfloat ConvertAnisotropy(TextureFilter Value)
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
    constexpr ConstPtr<GLfloat> Convert(TextureBorder Value)
    {
        static constexpr GLfloat kMapping[][4] = {
            { 0.0f, 0.0f, 0.0f, 1.0f },     // OpaqueBlack
            { 1.0f, 1.0f, 1.0f, 1.0f },     // OpaqueWhite
            { 0.0f, 0.0f, 0.0f, 0.0f },     // TransparentBlack
        };
        return kMapping[ZyEnum::Cast(Value)];
    }

    /// \brief Converts \ref VertexFormat into its generic vertex attribute description.
    constexpr AttributeFormat Convert(VertexFormat Value)
    {
        const VertexMetadata Description = GetVertexMetadata(Value);

        GLenum Type;

        if (Description.IsPacked)
        {
            Type = Description.IsSigned ? GL_INT_2_10_10_10_REV : GL_UNSIGNED_INT_2_10_10_10_REV;
        }
        else if (Description.IsFloat)
        {
            Type = (Description.BytesPerComponent == 2) ? GL_HALF_FLOAT : GL_FLOAT;
        }
        else
        {
            switch (Description.BytesPerComponent)
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
        const Bool      Integer    = Description.IsInteger() && !Description.IsPacked;
        return AttributeFormat(static_cast<GLint>(Description.Components), Type, Normalized, Integer);
    }

    /// \brief Selects the OpenGL index element type for the given index stride, in bytes.
    constexpr GLenum ConvertIndex(UInt16 Stride)
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

    /// \brief Converts \ref TextureFormat into the matching OpenGL representation.
    ///
    /// \param Value The engine format to look up.
    /// \return The internal, external and type triple the format uploads and stores with.
    ConstRef<Format> Convert(TextureFormat Value);
}