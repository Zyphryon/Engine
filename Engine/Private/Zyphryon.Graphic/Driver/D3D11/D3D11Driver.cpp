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

#include "D3D11Driver.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool CheckIfSucceed(HRESULT Result)
    {
        if (FAILED(Result))
        {
            constexpr UINT Flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

            TCHAR Message[1024];
            ::FormatMessage(Flags, nullptr, Result, 0, Message, sizeof(Message), nullptr);

            LOG_CRITICAL("D3D11Driver: {}", Message);
            return false;
        }
        return true;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Access Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_USAGE_IMMUTABLE,           // Access::Immutable
            D3D11_USAGE_DYNAMIC,             // Access::Dynamic
            D3D11_USAGE_DEFAULT,             // Access::Stream
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(BlendFactor Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_BLEND_ZERO,                       // BlendFactor::Zero
            D3D11_BLEND_ONE,                        // BlendFactor::One
            D3D11_BLEND_SRC_COLOR,                  // BlendFactor::SrcColor
            D3D11_BLEND_INV_SRC_COLOR,              // BlendFactor::OneMinusSrcColor
            D3D11_BLEND_SRC_ALPHA,                  // BlendFactor::SrcAlpha
            D3D11_BLEND_INV_SRC_ALPHA,              // BlendFactor::OneMinusSrcAlpha
            D3D11_BLEND_DEST_COLOR,                 // BlendFactor::DstColor
            D3D11_BLEND_INV_DEST_COLOR,             // BlendFactor::OneMinusDstColor
            D3D11_BLEND_DEST_ALPHA,                 // BlendFactor::DstAlpha
            D3D11_BLEND_INV_DEST_ALPHA,             // BlendFactor::OneMinusDstAlpha
            D3D11_BLEND_SRC1_COLOR,                 // BlendFactor::Src1Color
            D3D11_BLEND_INV_SRC1_COLOR,             // BlendFactor::OneMinusSrc1Color
            D3D11_BLEND_SRC1_ALPHA,                 // BlendFactor::Src1Alpha
            D3D11_BLEND_INV_SRC1_ALPHA,             // BlendFactor::OneMinusSrc1Alpha
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(BlendFunction Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_BLEND_OP_ADD,                     // BlendFunction::Add
            D3D11_BLEND_OP_SUBTRACT,                // BlendFunction::Subtract
            D3D11_BLEND_OP_REV_SUBTRACT,            // BlendFunction::ReverseSubtract
            D3D11_BLEND_OP_MIN,                     // BlendFunction::Min
            D3D11_BLEND_OP_MAX                      // BlendFunction::Max
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Cull Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_CULL_NONE,                // Cull::None
            D3D11_CULL_BACK,                // Cull::Back
            D3D11_CULL_FRONT,               // Cull::Front
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Fill Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_FILL_SOLID,               // Fill::Solid
            D3D11_FILL_WIREFRAME,           // Fill::Wireframe
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Primitive Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,        // Primitive::PointList
            D3D11_PRIMITIVE_TOPOLOGY_LINELIST,         // Primitive::LineList
            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,        // Primitive::LineStrip
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,     // Primitive::TriangleList
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,    // Primitive::TriangleStrip
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Multisample Value) noexcept
    {
        static constexpr Array kMapping = {
            1,        // Multisample::X1
            2,        // Multisample::X2
            4,        // Multisample::X4
            8,        // Multisample::X8
            16,       // Multisample::X16
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(TestAction Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_STENCIL_OP_KEEP,                  // TestAction::Keep
            D3D11_STENCIL_OP_REPLACE,               // TestAction::Replace
            D3D11_STENCIL_OP_ZERO,                  // TestAction::Zero
            D3D11_STENCIL_OP_DECR,                  // TestAction::Decrease
            D3D11_STENCIL_OP_DECR_SAT,              // TestAction::DecrementSaturate
            D3D11_STENCIL_OP_INVERT,                // TestAction::Invert
            D3D11_STENCIL_OP_INCR,                  // TestAction::Increase
            D3D11_STENCIL_OP_INCR_SAT               // TestAction::IncreaseSaturate
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(TestCondition Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_COMPARISON_ALWAYS,                // TestCondition::Always
            D3D11_COMPARISON_NEVER,                 // TestCondition::Never
            D3D11_COMPARISON_GREATER,               // TestCondition::Greater
            D3D11_COMPARISON_GREATER_EQUAL,         // TestCondition::GreaterEqual
            D3D11_COMPARISON_EQUAL,                 // TestCondition::Equal
            D3D11_COMPARISON_NOT_EQUAL,             // TestCondition::NotEqual
            D3D11_COMPARISON_LESS,                  // TestCondition::Less
            D3D11_COMPARISON_LESS_EQUAL             // TestCondition::LessEqual
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(TextureAddress Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_TEXTURE_ADDRESS_CLAMP,            // TextureAddress::Clamp
            D3D11_TEXTURE_ADDRESS_BORDER,           // TextureAddress::Border
            D3D11_TEXTURE_ADDRESS_WRAP,             // TextureAddress::Repeat
            D3D11_TEXTURE_ADDRESS_MIRROR            // TextureAddress::Mirror
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(TextureBorder Value) noexcept
    {
        static constexpr Array<Array<const Real32, 4>, Enum::Count<TextureBorder>()> kMapping = {{
            { { 0.0f, 0.0f, 0.0f, 0.0f } }, // TextureBorder::OpaqueBlack
            { { 1.0f, 1.0f, 1.0f, 0.0f } }, // TextureBorder::OpaqueWhite
            { { 0.0f, 0.0f, 0.0f, 1.0f } }, // TextureBorder::TransparentBlack
            { { 1.0f, 1.0f, 1.0f, 1.0f } }, // TextureBorder::TransparentWhite
        }};
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(TextureFilter Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_FILTER_MIN_MAG_MIP_POINT,          // TextureFilter::Point
            D3D11_FILTER_MIN_MAG_MIP_POINT,          // TextureFilter::PointMipPoint
            D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR,   // TextureFilter::PointMipLinear
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,         // TextureFilter::Linear
            D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,   // TextureFilter::LinearMipPoint
            D3D11_FILTER_MIN_MAG_MIP_LINEAR,         // TextureFilter::LinearMipLinear
            D3D11_FILTER_ANISOTROPIC,                // TextureFilter::Anisotropic2x
            D3D11_FILTER_ANISOTROPIC,                // TextureFilter::Anisotropic4x
            D3D11_FILTER_ANISOTROPIC,                // TextureFilter::Anisotropic8x
            D3D11_FILTER_ANISOTROPIC,                // TextureFilter::Anisotropic16x
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto AsResourceFormat(TextureFormat Value) noexcept
    {
        constexpr static Array kMapping = {
            DXGI_FORMAT_BC1_UNORM,              // TextureFormat::BC1UIntNorm
            DXGI_FORMAT_BC1_UNORM_SRGB,         // TextureFormat::BC1UIntNorm_sRGB
            DXGI_FORMAT_BC2_UNORM,              // TextureFormat::BC2UIntNorm
            DXGI_FORMAT_BC2_UNORM_SRGB,         // TextureFormat::BC2UIntNorm_sRGB
            DXGI_FORMAT_BC3_UNORM,              // TextureFormat::BC3UIntNorm
            DXGI_FORMAT_BC3_UNORM_SRGB,         // TextureFormat::BC3UIntNorm_sRGB
            DXGI_FORMAT_BC4_UNORM,              // TextureFormat::BC4UIntNorm
            DXGI_FORMAT_BC4_SNORM,              // TextureFormat::BC4SIntNorm
            DXGI_FORMAT_BC5_UNORM,              // TextureFormat::BC5UIntNorm
            DXGI_FORMAT_BC5_SNORM,              // TextureFormat::BC5SIntNorm
            DXGI_FORMAT_BC6H_UF16,              // TextureFormat::BC6UFloat
            DXGI_FORMAT_BC6H_SF16,              // TextureFormat::BC6SFloat
            DXGI_FORMAT_BC7_UNORM,              // TextureFormat::BC7UIntNorm
            DXGI_FORMAT_BC7_UNORM_SRGB,         // TextureFormat::BC7UIntNorm_sRGB
            DXGI_FORMAT_R8_SINT,                // TextureFormat::R8SInt
            DXGI_FORMAT_R8_SNORM,               // TextureFormat::R8SIntNorm
            DXGI_FORMAT_R8_UINT,                // TextureFormat::R8UInt
            DXGI_FORMAT_R8_UNORM,               // TextureFormat::R8UIntNorm
            DXGI_FORMAT_R16_SINT,               // TextureFormat::R16SInt
            DXGI_FORMAT_R16_SNORM,              // TextureFormat::R16SIntNorm
            DXGI_FORMAT_R16_UINT,               // TextureFormat::R16UInt
            DXGI_FORMAT_R16_UNORM,              // TextureFormat::R16UIntNorm
            DXGI_FORMAT_R16_FLOAT,              // TextureFormat::R16Float
            DXGI_FORMAT_R32_SINT,               // TextureFormat::R32SInt
            DXGI_FORMAT_R32_UINT,               // TextureFormat::R32UInt
            DXGI_FORMAT_R32_FLOAT,              // TextureFormat::R32Float
            DXGI_FORMAT_R8G8_SINT,              // TextureFormat::RG8SInt
            DXGI_FORMAT_R8G8_SNORM,             // TextureFormat::RG8SIntNorm
            DXGI_FORMAT_R8G8_UINT,              // TextureFormat::RG8UInt
            DXGI_FORMAT_R8G8_UNORM,             // TextureFormat::RG8UIntNorm
            DXGI_FORMAT_R16G16_SINT,            // TextureFormat::RG16SInt
            DXGI_FORMAT_R16G16_SNORM,           // TextureFormat::RG16SIntNorm
            DXGI_FORMAT_R16G16_UINT,            // TextureFormat::RG16UInt
            DXGI_FORMAT_R16G16_UNORM,           // TextureFormat::RG16UIntNorm
            DXGI_FORMAT_R16G16_FLOAT,           // TextureFormat::RG16Float
            DXGI_FORMAT_R32G32_SINT,            // TextureFormat::RG32SInt
            DXGI_FORMAT_R32G32_UINT,            // TextureFormat::RG32UInt
            DXGI_FORMAT_R32G32_FLOAT,           // TextureFormat::RG32Float
            DXGI_FORMAT_R32G32B32_SINT,         // TextureFormat::RGB32SInt
            DXGI_FORMAT_R32G32B32_UINT,         // TextureFormat::RGB32UInt
            DXGI_FORMAT_R32G32B32_FLOAT,        // TextureFormat::RGB32Float
            DXGI_FORMAT_R8G8B8A8_SINT,          // TextureFormat::RGBA8SInt
            DXGI_FORMAT_R8G8B8A8_SNORM,         // TextureFormat::RGBA8SIntNorm
            DXGI_FORMAT_R8G8B8A8_UINT,          // TextureFormat::RGBA8UInt
            DXGI_FORMAT_R8G8B8A8_UNORM,         // TextureFormat::RGBA8UIntNorm
            DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,    // TextureFormat::RGBA8UIntNorm_sRGB
            DXGI_FORMAT_R16G16B16A16_SINT,      // TextureFormat::RGBA16SInt
            DXGI_FORMAT_R16G16B16A16_SNORM,     // TextureFormat::RGBA16SIntNorm
            DXGI_FORMAT_R16G16B16A16_UINT,      // TextureFormat::RGBA16UInt
            DXGI_FORMAT_R16G16B16A16_UNORM,     // TextureFormat::RGBA16UIntNorm
            DXGI_FORMAT_R16G16B16A16_FLOAT,     // TextureFormat::RGBA16Float
            DXGI_FORMAT_R32G32B32A32_SINT,      // TextureFormat::RGBA32SInt
            DXGI_FORMAT_R32G32B32A32_UINT,      // TextureFormat::RGBA32UInt
            DXGI_FORMAT_R32G32B32A32_FLOAT,     // TextureFormat::RGBA32Float
            DXGI_FORMAT_R10G10B10A2_UINT,       // TextureFormat::RGB10A2UInt
            DXGI_FORMAT_R10G10B10A2_UNORM,      // TextureFormat::RGB10A2UIntNorm
            DXGI_FORMAT_R16_TYPELESS,           // TextureFormat::D16Float
            DXGI_FORMAT_R16_TYPELESS,           // TextureFormat::D16UIntNorm
            DXGI_FORMAT_R32_TYPELESS,           // TextureFormat::D32Float
            DXGI_FORMAT_R24G8_TYPELESS,         // TextureFormat::D24S8UIntNorm
            DXGI_FORMAT_R32G8X24_TYPELESS,      // TextureFormat::D32S8UIntNorm
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto AsColorResourceFormat(TextureFormat Value) noexcept
    {
        switch (Value)
        {
        case TextureFormat::D16Float:
            return DXGI_FORMAT_R16_FLOAT;
        case TextureFormat::D16UIntNorm:
            return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::D32Float:
            return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::D24S8UIntNorm:
            return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case TextureFormat::D32S8UIntNorm:
            return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default:
            return AsResourceFormat(Value);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto AsDepthResourceFormat(TextureFormat Value) noexcept
    {
        switch (Value)
        {
        case TextureFormat::D16Float:
            return DXGI_FORMAT_R16_FLOAT;
        case TextureFormat::D16UIntNorm:
            return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::D32Float:
            return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::D24S8UIntNorm:
            return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32S8UIntNorm:
            return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:
            return DXGI_FORMAT_UNKNOWN;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(Usage Value) noexcept
    {
        static constexpr Array kMapping = {
            D3D11_BIND_VERTEX_BUFFER,          // Usage::Vertex
            D3D11_BIND_INDEX_BUFFER,           // Usage::Index
            D3D11_BIND_CONSTANT_BUFFER,        // Usage::Uniform
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto As(VertexFormat Value) noexcept
    {
        static constexpr Array kMapping = {
            DXGI_FORMAT_R16G16_FLOAT,           // VertexFormat::Float16x2
            DXGI_FORMAT_R16G16B16A16_FLOAT,     // VertexFormat::Float16x4
            DXGI_FORMAT_R32_FLOAT,              // VertexFormat::Float32x1
            DXGI_FORMAT_R32G32_FLOAT,           // VertexFormat::Float32x2
            DXGI_FORMAT_R32G32B32_FLOAT,        // VertexFormat::Float32x3
            DXGI_FORMAT_R32G32B32A32_FLOAT,     // VertexFormat::Float32x4
            DXGI_FORMAT_R8G8B8A8_SINT,          // VertexFormat::SInt8x4
            DXGI_FORMAT_R8G8B8A8_SNORM,         // VertexFormat::SIntNorm8x4
            DXGI_FORMAT_R8G8B8A8_UINT,          // VertexFormat::UInt8x4
            DXGI_FORMAT_R8G8B8A8_UNORM,         // VertexFormat::UIntNorm8x4
            DXGI_FORMAT_R16G16_SINT,            // VertexFormat::SInt16x2
            DXGI_FORMAT_R16G16_SNORM,           // VertexFormat::SIntNorm16x2
            DXGI_FORMAT_R16G16_UINT,            // VertexFormat::UInt16x2
            DXGI_FORMAT_R16G16_UNORM,           // VertexFormat::UIntNorm16x2
            DXGI_FORMAT_R16G16B16A16_SINT,      // VertexFormat::SInt16x4
            DXGI_FORMAT_R16G16B16A16_SNORM,     // VertexFormat::SIntNorm16x4
            DXGI_FORMAT_R16G16B16A16_UINT,      // VertexFormat::UInt16x4
            DXGI_FORMAT_R16G16B16A16_UNORM,     // VertexFormat::UIntNorm16x4
            DXGI_FORMAT_R32_SINT,               // VertexFormat::SInt32x1
            DXGI_FORMAT_R32_UINT,               // VertexFormat::UInt32x1
            DXGI_FORMAT_R32G32_SINT,            // VertexFormat::SInt32x2
            DXGI_FORMAT_R32G32_UINT,            // VertexFormat::UInt32x2
            DXGI_FORMAT_R32G32B32_SINT,         // VertexFormat::SInt32x3
            DXGI_FORMAT_R32G32B32_UINT,         // VertexFormat::UInt32x3
            DXGI_FORMAT_R32G32B32A32_SINT,      // VertexFormat::SInt32x4
            DXGI_FORMAT_R32G32B32A32_UINT,      // VertexFormat::UInt32x4
            DXGI_FORMAT_R10G10B10A2_UINT,       // VertexFormat::UInt10_10_10_2
            DXGI_FORMAT_R10G10B10A2_UNORM,      // VertexFormat::UIntNorm10_10_10_2
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto AsVertexName(VertexSemantic Value) noexcept
    {
        constexpr static Array kMapping = {
            "POSITION",         // VertexSemantic::Position
            "NORMAL",           // VertexSemantic::Normal
            "TANGENT",          // VertexSemantic::Tangent
            "BITANGENT",        // VertexSemantic::Bitangent
            "COLOR",            // VertexSemantic::Color0
            "COLOR",            // VertexSemantic::Color1
            "COLOR",            // VertexSemantic::Color2
            "COLOR",            // VertexSemantic::Color3
            "BLENDINDICES",     // VertexSemantic::BlendIndices
            "BLENDWEIGHTS",     // VertexSemantic::BlendWeights
            "TEXCOORD",         // VertexSemantic::TexCoord0
            "TEXCOORD",         // VertexSemantic::TexCoord1
            "TEXCOORD",         // VertexSemantic::TexCoord2
            "TEXCOORD",         // VertexSemantic::TexCoord3
            "TEXCOORD",         // VertexSemantic::TexCoord4
            "TEXCOORD",         // VertexSemantic::TexCoord5
            "TEXCOORD",         // VertexSemantic::TexCoord6
            "TEXCOORD",         // VertexSemantic::TexCoord7
            "CUSTOM",           // VertexSemantic::Custom0
            "CUSTOM",           // VertexSemantic::Custom1
            "CUSTOM",           // VertexSemantic::Custom2
            "CUSTOM",           // VertexSemantic::Custom3
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static constexpr auto AsVertexIndex(VertexSemantic Value) noexcept
    {
        constexpr static Array kMapping = {
            0,            // VertexSemantic::Position
            0,            // VertexSemantic::Normal
            0,            // VertexSemantic::Tangent
            0,            // VertexSemantic::Bitangent
            0,            // VertexSemantic::Color0
            1,            // VertexSemantic::Color1
            2,            // VertexSemantic::Color2
            3,            // VertexSemantic::Color3
            0,            // VertexSemantic::BlendIndices
            0,            // VertexSemantic::BlendWeights
            0,            // VertexSemantic::TexCoord0
            1,            // VertexSemantic::TexCoord1
            2,            // VertexSemantic::TexCoord2
            3,            // VertexSemantic::TexCoord3
            4,            // VertexSemantic::TexCoord4
            5,            // VertexSemantic::TexCoord5
            6,            // VertexSemantic::TexCoord6
            7,            // VertexSemantic::TexCoord7
            0,            // VertexSemantic::Custom0
            1,            // VertexSemantic::Custom1
            2,            // VertexSemantic::Custom2
            3,            // VertexSemantic::Custom3
        };
        return kMapping[Enum::Cast(Value)];
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static auto Fill(ConstPtr<UInt8> Data, UInt8 Layer, UInt16 Width, UInt16 Height, TextureFormat Layout)
    {
        constexpr static UInt8 kBitsPerPixel[] = {
            4,      // TextureFormat::BC1UIntNorm
            4,      // TextureFormat::BC1UIntNorm_sRGB
            8,      // TextureFormat::BC2UIntNorm
            8,      // TextureFormat::BC2UIntNorm_sRGB
            8,      // TextureFormat::BC3UIntNorm
            8,      // TextureFormat::BC3UIntNorm_sRGB
            4,      // TextureFormat::BC4UIntNorm
            4,      // TextureFormat::BC4SIntNorm
            8,      // TextureFormat::BC5UIntNorm
            8,      // TextureFormat::BC5SIntNorm
            8,      // TextureFormat::BC6UFloat
            8,      // TextureFormat::BC6SFloat
            8,      // TextureFormat::BC7UIntNorm
            8,      // TextureFormat::BC7UIntNorm_sRGB
            8,      // TextureFormat::R8SInt
            8,      // TextureFormat::R8SIntNorm
            8,      // TextureFormat::R8UInt
            8,      // TextureFormat::R8UIntNorm
            16,     // TextureFormat::R16SInt
            16,     // TextureFormat::R16SIntNorm
            16,     // TextureFormat::R16UInt
            16,     // TextureFormat::R16UIntNorm
            16,     // TextureFormat::R16Float
            32,     // TextureFormat::R32SInt
            32,     // TextureFormat::R32UInt
            32,     // TextureFormat::R32Float
            16,     // TextureFormat::RG8SInt
            16,     // TextureFormat::RG8SIntNorm
            16,     // TextureFormat::RG8UInt
            16,     // TextureFormat::RG8UIntNorm
            32,     // TextureFormat::RG16SInt
            32,     // TextureFormat::RG16SIntNorm
            32,     // TextureFormat::RG16UInt
            32,     // TextureFormat::RG16UIntNorm
            32,     // TextureFormat::RG16Float
            64,     // TextureFormat::RG32SInt
            64,     // TextureFormat::RG32UInt
            64,     // TextureFormat::RG32Float
            96,     // TextureFormat::RGB32SInt
            96,     // TextureFormat::RGB32UInt
            96,     // TextureFormat::RGB32Float
            32,     // TextureFormat::RGBA8SInt
            32,     // TextureFormat::RGBA8SIntNorm
            32,     // TextureFormat::RGBA8UInt
            32,     // TextureFormat::RGBA8UIntNorm
            32,     // TextureFormat::RGBA8UIntNorm_sRGB
            32,     // TextureFormat::BGRA8UIntNorm
            32,     // TextureFormat::BGRA8UIntNorm_sRGB
            64,     // TextureFormat::RGBA16SInt
            64,     // TextureFormat::RGBA16SIntNorm
            64,     // TextureFormat::RGBA16UInt
            64,     // TextureFormat::RGBA16UIntNorm
            64,     // TextureFormat::RGBA16Float
            128,    // TextureFormat::RGBA32SInt
            128,    // TextureFormat::RGBA32UInt
            128,    // TextureFormat::RGBA32Float
            16,     // TextureFormat::RGB10A2UIntNorm
            16,     // TextureFormat::RGB10A2UInt
            16,     // TextureFormat::D16Float
            16,     // TextureFormat::D16UIntNorm
            32,     // TextureFormat::D32Float
            32,     // TextureFormat::D24S8UIntNorm
            64,     // TextureFormat::D32S8UIntNorm
        };

        static D3D11_SUBRESOURCE_DATA Content[kMaxMipmaps];

        if (Data)
        {
            const UInt32 BitsPerPixel = kBitsPerPixel[Enum::Cast(Layout)];
            const UInt32 BlockSize    = Enum::Cast(Layout) > Enum::Cast(TextureFormat::BC7UIntNorm_sRGB) ? 1 : 4;

            for (UInt32 Level = 0; Level < Layer; ++Level)
            {
                const UInt32 Pitch = (Width + BlockSize - 1) / BlockSize * (BitsPerPixel * BlockSize * BlockSize / 8);

                Content[Level].pSysMem          = Data;
                Content[Level].SysMemPitch      = Pitch;
                Content[Level].SysMemSlicePitch = Pitch * ((Height + BlockSize - 1) / BlockSize);

                Data  += Pitch * ((Height + BlockSize - 1) / BlockSize);
                Width  = Max(1U, Width  >> 1);
                Height = Max(1U, Height >> 1);
            }
            return Content;
        }
        return static_cast<Ptr<D3D11_SUBRESOURCE_DATA>>(nullptr);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    D3D11Driver::D3D11Driver()
        : mProperties   { },
          mCapabilities { }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Bool D3D11Driver::Initialize(Ptr<SDL_Window> Window, UInt16 Width, UInt16 Height, Multisample Samples)
    {
        decltype(& D3D11CreateDevice)  D3D11CreateDevicePtr = nullptr;
        decltype(& CreateDXGIFactory1) CreateDXGIFactoryPtr = nullptr;

        if (const auto Dll = ::LoadLibraryEx("D3D11.DLL", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); Dll != nullptr)
        {
            D3D11CreateDevicePtr
                = reinterpret_cast<decltype(&D3D11CreateDevice)>(GetProcAddress(Dll, "D3D11CreateDevice"));
        }
        if (const auto Dll = ::LoadLibraryEx("DXGI.DLL", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32); Dll != nullptr)
        {
            CreateDXGIFactoryPtr
                = reinterpret_cast<decltype(&CreateDXGIFactory1)>(GetProcAddress(Dll, "CreateDXGIFactory1"));
        }

        Bool Successful = D3D11CreateDevicePtr && CreateDXGIFactoryPtr;

        if (Successful)
        {
            ComPtr<ID3D11Device>        Device;
            ComPtr<ID3D11DeviceContext> DeviceImmediate;

            constexpr UInt Flags = D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_BGRA_SUPPORT;

            constexpr D3D_FEATURE_LEVEL Direct3DFeatureLevels[] = {
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1,
                D3D_FEATURE_LEVEL_10_0,
                D3D_FEATURE_LEVEL_9_3,
                D3D_FEATURE_LEVEL_9_2,
                D3D_FEATURE_LEVEL_9_1,
            };

            // Tries Direct3D 11.1
            HRESULT Result = D3D11CreateDevicePtr(
                nullptr,
                D3D_DRIVER_TYPE_HARDWARE,
                nullptr,
                Flags,
                Direct3DFeatureLevels,
                _countof(Direct3DFeatureLevels) - 3, // 11.1 ... 10.0
                D3D11_SDK_VERSION,
                Device.GetAddressOf(),
                nullptr,
                DeviceImmediate.GetAddressOf());

            // Tries Direct3D 10.0 and below.
            if (FAILED(Result))
            {
                Result = D3D11CreateDevicePtr(
                    nullptr,
                    D3D_DRIVER_TYPE_HARDWARE,
                    nullptr,
                    Flags,
                    Direct3DFeatureLevels + 1,
                    _countof(Direct3DFeatureLevels) - 1, // 11.0 ... 9.1
                    D3D11_SDK_VERSION,
                    Device.GetAddressOf(),
                    nullptr,
                    DeviceImmediate.GetAddressOf());
            }

            // Tries Direct3D WARP mode.
            if (FAILED(Result))
            {
                Result = D3D11CreateDevicePtr(
                    nullptr,
                    D3D_DRIVER_TYPE_WARP,
                    nullptr,
                    Flags,
                    Direct3DFeatureLevels,
                    _countof(Direct3DFeatureLevels),
                    D3D11_SDK_VERSION,
                    Device.GetAddressOf(),
                    nullptr,
                    DeviceImmediate.GetAddressOf());

                if (SUCCEEDED(Result))
                {
                    LOG_INFO("D3D11Driver: Enabling software mode (WARP)");
                }
            }

            Successful = CheckIfSucceed(Result);

            if (Successful)
            {
                Successful = CheckIfSucceed(Device.As<ID3D11Device1>(& mDevice));
                Successful = Successful && CheckIfSucceed(DeviceImmediate.As<ID3D11DeviceContext1>(& mDeviceImmediate));
                Successful = Successful && CheckIfSucceed(CreateDXGIFactoryPtr(IID_PPV_ARGS(& mDeviceFactory)));

                if (Successful)
                {
                    LoadAdapters();
                    LoadCapabilities();
                    LoadStates();

                    if (Window)
                    {
                        const Ptr<void> Handle = SDL_GetPointerProperty(
                            SDL_GetWindowProperties(Window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
                        CreateSwapchain(mPasses[0], static_cast<HWND>(Handle), Width, Height, As(Samples));
                    }
                }
            }
        }

        return Successful;

    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Reset(UInt16 Width, UInt16 Height, Multisample Samples)
    {
        if (mDeviceImmediate)
        {
            mDeviceImmediate->ClearState();
        }

        /// Deletes the current display pass to release associated render targets.
        DeletePass(kDisplay);

        /// Resizes the swap chain buffers with the new resolution and format.
        const UINT Flags = mProperties.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        CheckIfSucceed(mDisplay->ResizeBuffers(0, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, Flags));

        /// Recreates swap chain resources, including color and depth-stencil attachments.
        CreateSwapchainResources(mPasses[0], Width, Height, As(Samples));

        /// Restores GPU pipeline states to their default configuration.
        LoadStates();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Device> D3D11Driver::GetDevice() const
    {
        return mCapabilities;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateBuffer(Object ID, Access Access, Usage Usage, UInt32 Capacity, ConstSpan<Byte> Data)
    {
        const D3D11_BUFFER_DESC Descriptor = CD3D11_BUFFER_DESC(
            Usage  == Usage::Uniform ? Align(Capacity, kUniformBlockAlignment) : Capacity,
            As(Usage),
            As(Access),
            Access == Access::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0);

        D3D11_SUBRESOURCE_DATA Content {
            Data.data(),
            static_cast<UINT>(Data.size_bytes())
        };

        const Ptr<D3D11_SUBRESOURCE_DATA> Memory = (Data.empty() ? nullptr : &Content);
        CheckIfSucceed(mDevice->CreateBuffer(&Descriptor, Memory, mBuffers[ID].Object.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UpdateBuffer(Object ID, UInt32 Offset, Bool Invalidate, ConstSpan<Byte> Data)
    {
        const D3D11_BOX Destination  = CD3D11_BOX(Offset, 0, 0, Offset + Data.size(), 1, 1);
        const D3D11_COPY_FLAGS Flags = (Invalidate ? D3D11_COPY_DISCARD : D3D11_COPY_NO_OVERWRITE);
        mDeviceImmediate->UpdateSubresource1(mBuffers[ID].Object.Get(), 0, &Destination, Data.data(), 0, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteBuffer(Object ID)
    {
        InPlaceDelete(mBuffers[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ResizeBuffer(Object ID, UInt32 Capacity)
    {
        // Query the existing buffer description to preserve all current settings.
        D3D11_BUFFER_DESC Descriptor;
        mBuffers[ID].Object->GetDesc(&Descriptor);

        // Update the buffer size while leaving other properties unchanged.
        Descriptor.ByteWidth = Capacity;

        // Recreate the buffer with the updated description.
        CheckIfSucceed(mDevice->CreateBuffer(&Descriptor, nullptr, mBuffers[ID].Object.ReleaseAndGetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CopyBuffer(Object SrcBuffer, UInt32 SrcOffset, Object DstBuffer, UInt32 DstOffset, Bool Invalidate, UInt32 Size)
    {
        const D3D11_BOX Offset       = CD3D11_BOX(SrcOffset, 0, 0, SrcOffset + Size, 1, 1);
        const D3D11_COPY_FLAGS Flags = Invalidate ? D3D11_COPY_DISCARD : D3D11_COPY_NO_OVERWRITE;

        const Ptr<ID3D11Buffer> Target = mBuffers[DstBuffer].Object.Get();
        const Ptr<ID3D11Buffer> Source = mBuffers[SrcBuffer].Object.Get();
        mDeviceImmediate->CopySubresourceRegion1(Target, 0, DstOffset, 0, 0, Source, 0, &Offset, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob D3D11Driver::ReadBuffer(Object ID, UInt32 Offset, UInt32 Size)
    {
        return { }; // TODO
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ptr<Byte> D3D11Driver::MapBuffer(Object ID, UInt32 Offset, UInt32 Size, Bool Invalidate)
    {
        const D3D11_MAP          Mode = (Invalidate ? D3D11_MAP_WRITE_DISCARD : D3D11_MAP_WRITE_NO_OVERWRITE);
        D3D11_MAPPED_SUBRESOURCE Memory;

        if (CheckIfSucceed(mDeviceImmediate->Map(mBuffers[ID].Object.Get(), 0, Mode, 0, &Memory)))
        {
            return static_cast<Ptr<Byte>>(Memory.pData) + Offset;
        }
        return nullptr;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UnmapBuffer(Object ID)
    {
        mDeviceImmediate->Unmap(mBuffers[ID].Object.Get(), 0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreatePass(Object ID, ConstSpan<Attachment> Colors, ConstRef<Attachment> Auxiliary)
    {
        Ref<D3D11Pass> Pass = mPasses[ID];

        // Configure and create render target views (RTVs) for each color attachment.
        for (ConstRef<Attachment> Color : Colors)
        {
            Ref<D3D11Attachment> Attachment = Pass.Colors.emplace_back();
            Attachment.Source       = Color.SourceTexture ? mTextures[Color.SourceTexture].Object : nullptr;
            Attachment.SourceLevel  = Color.SourceLevel;
            Attachment.Target       = mTextures[Color.TargetTexture].Object;
            Attachment.TargetLevel  = Color.TargetLevel;
            Attachment.TargetFormat = AsColorResourceFormat(mTextures[Color.TargetTexture].Format);

            if (Color.SourceTexture)
            {
                const DXGI_FORMAT Format = AsColorResourceFormat(mTextures[Color.SourceTexture].Format);
                const CD3D11_RENDER_TARGET_VIEW_DESC Description(D3D11_RTV_DIMENSION_TEXTURE2DMS, Format, Color.SourceLevel);
                CheckIfSucceed(mDevice->CreateRenderTargetView(
                    Attachment.Source.Get(), & Description, Attachment.Resource.GetAddressOf()));
            }
            else
            {
                const DXGI_FORMAT Format = AsColorResourceFormat(mTextures[Color.TargetTexture].Format);
                const CD3D11_RENDER_TARGET_VIEW_DESC Description(D3D11_RTV_DIMENSION_TEXTURE2D, Format, Color.TargetLevel);
                CheckIfSucceed(mDevice->CreateRenderTargetView(
                    Attachment.Target.Get(), & Description, Attachment.Resource.GetAddressOf()));
            }
        }

        // Create and assign its depth-stencil view (DSV) if an auxiliary attachment is provided.
        if (Auxiliary.TargetTexture > 0)
        {
            const Ptr<ID3D11Texture2D> Attachment = mTextures[Auxiliary.TargetTexture].Object.Get();

            const DXGI_FORMAT Format = AsDepthResourceFormat(mTextures[Auxiliary.TargetTexture].Format);
            const CD3D11_DEPTH_STENCIL_VIEW_DESC Description(
                D3D11_DSV_DIMENSION_TEXTURE2D, Format, Auxiliary.TargetLevel);
            CheckIfSucceed(mDevice->CreateDepthStencilView(Attachment, & Description, Pass.Auxiliary.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePass(Object ID)
    {
        InPlaceDelete(mPasses[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreatePipeline(Object ID, ConstRef<Shaders> Shaders, ConstRef<States> States)
    {
        Ref<D3D11Pipeline> Pipeline = mPipelines[ID];

        ConstRef<Blob> VS = Shaders[Enum::Cast(Stage::Vertex)];
        CheckIfSucceed(mDevice->CreateVertexShader(VS.GetData(), VS.GetSize(), nullptr, Pipeline.VS.GetAddressOf()));

        ConstRef<Blob> PS = Shaders[Enum::Cast(Stage::Fragment)];
        CheckIfSucceed(mDevice->CreatePixelShader(PS.GetData(), PS.GetSize(), nullptr, Pipeline.PS.GetAddressOf()));

        {
            D3D11_BLEND_DESC Description = CD3D11_BLEND_DESC(CD3D11_DEFAULT());

            Description.RenderTarget[0].BlendEnable           = !(
                   States.BlendSrcColor       == BlendFactor::One
                && States.BlendSrcAlpha       == BlendFactor::One
                && States.BlendDstColor       == BlendFactor::Zero
                && States.BlendDstAlpha       == BlendFactor::Zero
                && States.BlendEquationColor  == BlendFunction::Add
                && States.BlendEquationAlpha  == BlendFunction::Add);
            Description.RenderTarget[0].SrcBlend              = As(States.BlendSrcColor);
            Description.RenderTarget[0].DestBlend             = As(States.BlendDstColor);
            Description.RenderTarget[0].BlendOp               = As(States.BlendEquationColor);
            Description.RenderTarget[0].SrcBlendAlpha         = As(States.BlendSrcAlpha);
            Description.RenderTarget[0].DestBlendAlpha        = As(States.BlendDstAlpha);
            Description.RenderTarget[0].BlendOpAlpha          = As(States.BlendEquationAlpha);
            Description.RenderTarget[0].RenderTargetWriteMask = static_cast<D3D11_COLOR_WRITE_ENABLE>(States.Channel);
            Description.AlphaToCoverageEnable                 = States.AlphaToCoverage;

            CheckIfSucceed(mDevice->CreateBlendState(& Description, Pipeline.BS.GetAddressOf()));
        }

        {
            D3D11_DEPTH_STENCIL_DESC Description = CD3D11_DEPTH_STENCIL_DESC(CD3D11_DEFAULT());

            Description.DepthEnable    = States.DepthTest != TestCondition::Always || States.DepthMask;
            Description.DepthFunc      = As(States.DepthTest);
            Description.DepthWriteMask = static_cast<D3D11_DEPTH_WRITE_MASK>(States.DepthMask);
            Description.StencilEnable  = States.StencilFrontTest      != TestCondition::Always
                                      || States.StencilFrontFail      != TestAction::Keep
                                      || States.StencilFrontDepthFail != TestAction::Keep
                                      || States.StencilFrontDepthPass != TestAction::Keep
                                      || States.StencilBackTest       != TestCondition::Always
                                      || States.StencilBackFail       != TestAction::Keep
                                      || States.StencilBackDepthFail  != TestAction::Keep
                                      || States.StencilBackDepthPass  != TestAction::Keep;
            Description.StencilReadMask              = States.StencilReadMask;
            Description.StencilWriteMask             = States.StencilWriteMask;
            Description.FrontFace.StencilFunc        = As(States.StencilFrontTest);
            Description.FrontFace.StencilFailOp      = As(States.StencilFrontFail);
            Description.FrontFace.StencilDepthFailOp = As(States.StencilFrontDepthFail);
            Description.FrontFace.StencilPassOp      = As(States.StencilFrontDepthPass);
            Description.BackFace.StencilFunc         = As(States.StencilBackTest);
            Description.BackFace.StencilFailOp       = As(States.StencilBackFail);
            Description.BackFace.StencilDepthFailOp  = As(States.StencilBackDepthFail);
            Description.BackFace.StencilPassOp       = As(States.StencilBackDepthPass);

            CheckIfSucceed(mDevice->CreateDepthStencilState(& Description, Pipeline.DS.GetAddressOf()));
        }

        {
            D3D11_RASTERIZER_DESC Description = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

            Description.FrontCounterClockwise = TRUE;
            Description.DepthBias             = States.DepthBias;
            Description.DepthBiasClamp        = States.DepthBiasClamp;
            Description.SlopeScaledDepthBias  = States.DepthBiasSlope;
            Description.DepthClipEnable       = States.DepthClip;
            Description.CullMode              = As(States.Cull);
            Description.ScissorEnable         = TRUE;
            Description.MultisampleEnable     = TRUE;
            Description.AntialiasedLineEnable = TRUE;
            Description.FillMode              = As(States.Fill);

            CheckIfSucceed(mDevice->CreateRasterizerState(& Description, Pipeline.RS.GetAddressOf()));
        }

        {
            D3D11_INPUT_ELEMENT_DESC Description[kMaxAttributes];
            UInt                     Count = 0;

            for (ConstRef<Attribute> Attribute : States.InputAttributes)
            {
                Ref<D3D11_INPUT_ELEMENT_DESC> Element = Description[Count++];

                Element.SemanticName         = AsVertexName(Attribute.Semantic);
                Element.SemanticIndex        = AsVertexIndex(Attribute.Semantic);
                Element.Format               = As(Attribute.Format);
                Element.AlignedByteOffset    = Attribute.Offset;
                Element.InputSlot            = Attribute.Stream;
                Element.InputSlotClass       = Attribute.Divisor ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
                Element.InstanceDataStepRate = Attribute.Divisor;
            }

            CheckIfSucceed(mDevice->CreateInputLayout(Description, Count, VS.GetData(), VS.GetSize(), Pipeline.IL.GetAddressOf()));
        }

        Pipeline.PT = As(States.InputPrimitive);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeletePipeline(Object ID)
    {
        InPlaceDelete(mPipelines[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateTexture(Object ID, Access Access, TextureType Type, TextureFormat Format, TextureLayout Layout, UInt16 Width, UInt16 Height, UInt8 Mipmaps, Multisample Samples, ConstSpan<Byte> Data)
    {
        CD3D11_TEXTURE2D_DESC Description(AsResourceFormat(Format), Width, Height, 1, Mipmaps);
        Description.Usage      = As(Access);
        Description.BindFlags  = Layout != TextureLayout::Target ? D3D11_BIND_SHADER_RESOURCE : 0;
        Description.MiscFlags  = Mipmaps < 1 ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
        Description.SampleDesc = mProperties.Multisample[As(Samples)];

        if (Layout != TextureLayout::Source)
        {
            switch (Format)
            {
            case TextureFormat::D16Float:
            case TextureFormat::D16UIntNorm:
            case TextureFormat::D32Float:
            case TextureFormat::D24S8UIntNorm:
            case TextureFormat::D32S8UIntNorm:
                Description.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
                break;
            default:
                Description.BindFlags |= D3D11_BIND_RENDER_TARGET;
                break;
            }
        }

        Ref<D3D11Texture> Texture = mTextures[ID];
        Texture.Format = Format;

        const ConstPtr<D3D11_SUBRESOURCE_DATA> Memory = Fill(
            Data.data(), Mipmaps, Width, Height, Format);
        CheckIfSucceed(mDevice->CreateTexture2D(&Description, Memory, Texture.Object.GetAddressOf()));

        if (Layout != TextureLayout::Target)
        {
            const D3D11_SRV_DIMENSION Dimension = Samples != Multisample::X1
                ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                : D3D11_SRV_DIMENSION_TEXTURE2D;

            const CD3D11_SHADER_RESOURCE_VIEW_DESC View(Dimension, AsColorResourceFormat(Format), 0, Mipmaps);
            CheckIfSucceed(mDevice->CreateShaderResourceView(Texture.Object.Get(), &View, Texture.Resource.GetAddressOf()));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::UpdateTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        const D3D11_COPY_FLAGS Flags = D3D11_COPY_NO_OVERWRITE;
        const D3D11_BOX Offset       = CD3D11_BOX(X, Y, 0, X + Width, Y + Height, 1);
        mDeviceImmediate->UpdateSubresource1(mTextures[ID].Object.Get(), Level, &Offset, Data.data(), Pitch, 0, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::DeleteTexture(Object ID)
    {
        InPlaceDelete(mTextures[ID]);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CopyTexture(Object SrcTexture, UInt8 SrcLevel, UInt16 SrcX, UInt16  SrcY, Object DstTexture, UInt8 DstLevel, UInt16 DstX, UInt16 DstY, Bool Invalidate, UInt16 Width, UInt16 Height)
    {
        const D3D11_BOX Offset       = CD3D11_BOX(SrcX, SrcY, 0, SrcX + Width, SrcY + Height, 1);
        const D3D11_COPY_FLAGS Flags = Invalidate ? D3D11_COPY_DISCARD : D3D11_COPY_NO_OVERWRITE;

        const Ptr<ID3D11Texture2D> Target = mTextures[DstTexture].Object.Get();
        const Ptr<ID3D11Texture2D> Source = mTextures[SrcTexture].Object.Get();
        mDeviceImmediate->CopySubresourceRegion1(Target, DstLevel, DstX, DstY, 0, Source, SrcLevel, &Offset, Flags);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Blob D3D11Driver::ReadTexture(Object ID, UInt8 Level, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height)
    {
        return { }; // TODO
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Prepare(Object Pass, ConstRef<Viewport> Viewport, Clear Target, Color Tint, Real32 Depth, UInt8 Stencil)
    {
        // Clear color attachments as specified.
        Vector<Ptr<ID3D11RenderTargetView>, kMaxAttachments> Attachments;

        for (ConstRef<D3D11Attachment> Attachment : mPasses[Pass].Colors)
        {
            Attachments.emplace_back(Attachment.Resource.Get());

            if (Enum::Includes(Target, Clear::Color))
            {
                mDeviceImmediate->ClearRenderTargetView(Attachments.back(), reinterpret_cast<ConstPtr<FLOAT>>(&Tint));
            }
        }

        // Clear depth/stencil attachment as specified.
        UINT Mode = 0;
        if (Enum::Includes(Target, Clear::Depth))
        {
            Mode |= D3D11_CLEAR_DEPTH;
        }
        if (Enum::Includes(Target, Clear::Stencil))
        {
            Mode |= D3D11_CLEAR_STENCIL;
        }

        if (Mode != 0)
        {
            mDeviceImmediate->ClearDepthStencilView(mPasses[Pass].Auxiliary.Get(), Mode, Depth, Stencil);
        }

        // Bind the render targets for the rendering pass.
        const Ptr<ID3D11DepthStencilView> DSAttachment = (mPasses[Pass].Auxiliary ? mPasses[Pass].Auxiliary.Get() : nullptr);
        mDeviceImmediate->OMSetRenderTargets(Attachments.size(), Attachments.data(), DSAttachment);

        // Set the viewport for rendering.
        const CD3D11_VIEWPORT Rect(
            Viewport.X,
            Viewport.Y,
            Viewport.Width,
            Viewport.Height,
            Viewport.MinDepth,
            Viewport.MaxDepth);
        mDeviceImmediate->RSSetViewports(1, &Rect);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Submit(ConstSpan<DrawPacket> Submissions)
    {
        for (UInt32 Batch = 0; Batch < Submissions.size(); ++Batch)
        {
            ConstRef<DrawPacket> NewestSubmission = Submissions[Batch];
            ConstRef<DrawPacket> OldestSubmission = Batch > 0 ? Submissions[Batch - 1] : mStates;

            // Apply vertices
            ApplyVertexResources(OldestSubmission, NewestSubmission);

            // Apply indices
            if (OldestSubmission.Indices.Buffer != NewestSubmission.Indices.Buffer ||
                OldestSubmission.Indices.Offset != NewestSubmission.Indices.Offset ||
                OldestSubmission.Indices.Stride != NewestSubmission.Indices.Stride)
            {
                ConstRef<D3D11Buffer> Buffer = mBuffers[NewestSubmission.Indices.Buffer];
                const DXGI_FORMAT     Format =
                    NewestSubmission.Indices.Stride == 1 ? DXGI_FORMAT_R8_UINT  :
                    NewestSubmission.Indices.Stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
                mDeviceImmediate->IASetIndexBuffer(Buffer.Object.Get(), Format, NewestSubmission.Indices.Offset);
            }

            // Apply the scissor rect
            const Bool ScissorDirty = (OldestSubmission.Scissor.X      != NewestSubmission.Scissor.X)
                                   || (OldestSubmission.Scissor.Y      != NewestSubmission.Scissor.Y)
                                   || (OldestSubmission.Scissor.Width  != NewestSubmission.Scissor.Width)
                                   || (OldestSubmission.Scissor.Height != NewestSubmission.Scissor.Height);

            if (ScissorDirty)
            {
                const RECT Scissor = CD3D11_RECT(
                    NewestSubmission.Scissor.X,
                    NewestSubmission.Scissor.Y,
                    NewestSubmission.Scissor.X + NewestSubmission.Scissor.Width,
                    NewestSubmission.Scissor.Y + NewestSubmission.Scissor.Height);
                mDeviceImmediate->RSSetScissorRects(1, &Scissor);
            }

            // Apply pipeline or stencil value
            if (OldestSubmission.Pipeline != NewestSubmission.Pipeline)
            {
                ConstRef<D3D11Pipeline> Old = mPipelines[OldestSubmission.Pipeline];
                ConstRef<D3D11Pipeline> New = mPipelines[NewestSubmission.Pipeline];

                if (Old.VS != New.VS)
                {
                    mDeviceImmediate->VSSetShader(New.VS.Get(), nullptr, 0);
                }
                if (Old.PS != New.PS)
                {
                    mDeviceImmediate->PSSetShader(New.PS.Get(), nullptr, 0);
                }
                if (Old.BS != New.BS)
                {
                    mDeviceImmediate->OMSetBlendState(New.BS.Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);
                }
                if (Old.DS != New.DS || OldestSubmission.Stencil != NewestSubmission.Stencil)
                {
                    mDeviceImmediate->OMSetDepthStencilState(New.DS.Get(), NewestSubmission.Stencil);
                }
                if (Old.RS != New.RS)
                {
                    mDeviceImmediate->RSSetState(New.RS.Get());
                }
                if (Old.IL != New.IL)
                {
                    mDeviceImmediate->IASetInputLayout(New.IL.Get());
                }
                if (Old.PT != New.PT)
                {
                    mDeviceImmediate->IASetPrimitiveTopology(New.PT);
                }
            }
            else if (OldestSubmission.Stencil != NewestSubmission.Stencil)
            {
                mDeviceImmediate->OMSetDepthStencilState(
                    mPipelines[NewestSubmission.Pipeline].DS.Get(), NewestSubmission.Stencil);
            }

            // Apply stage(s) resources
            ApplySamplerResources(OldestSubmission, NewestSubmission);
            ApplyTextureResources(OldestSubmission, NewestSubmission);
            ApplyUniformResources(OldestSubmission, NewestSubmission);

            // Issue draw command
            const UInt32 Count     = NewestSubmission.Command.Count;
            const UInt32 Offset    = NewestSubmission.Command.Offset;
            const SInt32 Base      = NewestSubmission.Command.Base;
            const UInt32 Instances = NewestSubmission.Command.Instances;

            if (NewestSubmission.Indices.Buffer)
            {
                if (Instances > 1)
                {
                    mDeviceImmediate->DrawIndexedInstanced(Count, Instances, Offset, Base, 0);
                }
                else
                {
                    mDeviceImmediate->DrawIndexed(Count, Offset, Base);
                }
            }
            else
            {
                if (Instances > 1)
                {
                    mDeviceImmediate->DrawInstanced(Count, Instances, Offset, 0);
                }
                else
                {
                    mDeviceImmediate->Draw(Count, Offset);
                }
            }
        }

        // Apply cache
        mStates = Submissions.back();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::Commit(Object Pass)
    {
        /// Resolve multisample color attachments into their corresponding single-sample targets.
        for (ConstRef<D3D11Attachment> Attachment : mPasses[Pass].Colors)
        {
            if (Attachment.Source)
            {
                mDeviceImmediate->ResolveSubresource(
                    Attachment.Target.Get(), Attachment.TargetLevel,
                    Attachment.Source.Get(), Attachment.SourceLevel, Attachment.TargetFormat);
            }
        }

        /// Present the swap chain if this is the primary rendering pass.
        if (Pass == kDisplay)
        {
            const UInt Flag = mProperties.Tearing ? DXGI_PRESENT_ALLOW_TEARING : 0;
            CheckIfSucceed(mDisplay->Present(0, Flag));
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadAdapters()
    {
        ComPtr<IDXGIAdapter1> DXGIAdapter;
        for (UInt Index = 0; SUCCEEDED(mDeviceFactory->EnumAdapters1(Index, DXGIAdapter.GetAddressOf())); ++Index)
        {
            DXGI_ADAPTER_DESC1 DXGIDescription;
            if (SUCCEEDED(DXGIAdapter->GetDesc1(&DXGIDescription)))
            {
                if ((DXGIDescription.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0)
                {
                    Ref<Adapter> AdapterInfo = mCapabilities.Adapters.emplace_back();

                    AdapterInfo.Description = ConvertUTF16ToUTF8(DXGIDescription.Description);
                    AdapterInfo.Memory      = DXGIDescription.DedicatedVideoMemory >> 20;
                }
            }

            if (ComPtr<IDXGIOutput> DXGIOutput; SUCCEEDED(DXGIAdapter->EnumOutputs(0, DXGIOutput.GetAddressOf())))
            {
                UINT              Length = 0;
                const DXGI_FORMAT Format = DXGI_FORMAT_R8G8B8A8_UNORM;

                if (SUCCEEDED(DXGIOutput->GetDisplayModeList(Format, 0, &Length, nullptr)))
                {
                    Vector<DXGI_MODE_DESC> Descriptions(Length);
                    CheckIfSucceed(DXGIOutput->GetDisplayModeList(Format, 0, &Length, Descriptions.data()));

                    for (Ref<DXGI_MODE_DESC> Description : Descriptions)
                    {
                        Ref<Resolution> Resolution = mCapabilities.Adapters[Index].Resolutions.emplace_back();
                        Resolution.Width     = Description.Width;
                        Resolution.Height    = Description.Height;
                        Resolution.Frequency = Description.RefreshRate.Numerator / Description.RefreshRate.Denominator;
                    }
                }
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadCapabilities()
    {
        mCapabilities.Backend = Backend::D3D11;

        // Determine the supported shader model based on the Direct3D feature level.
        switch (mDevice->GetFeatureLevel())
        {
            case D3D_FEATURE_LEVEL_12_1:
            case D3D_FEATURE_LEVEL_12_0:
                mCapabilities.Language                         = Language::SM6;
                mCapabilities.Capabilities.MaxTextureDimension = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
                break;
            case D3D_FEATURE_LEVEL_11_1:
            case D3D_FEATURE_LEVEL_11_0:
                mCapabilities.Language                         = Language::SM5;
                mCapabilities.Capabilities.MaxTextureDimension = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
                break;
            case D3D_FEATURE_LEVEL_10_1:
            case D3D_FEATURE_LEVEL_10_0:
                mCapabilities.Language                         = Language::SM4;
                mCapabilities.Capabilities.MaxTextureDimension = D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = D3D10_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
                break;
            case D3D_FEATURE_LEVEL_9_3:
                mCapabilities.Language                         = Language::SM3;
                mCapabilities.Capabilities.MaxTextureDimension = D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = 0;
                break;
            case D3D_FEATURE_LEVEL_9_2:
                mCapabilities.Language                         = Language::SM2;
                mCapabilities.Capabilities.MaxTextureDimension = D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = 0;
                break;
            case D3D_FEATURE_LEVEL_9_1:
                mCapabilities.Language                         = Language::SM1;
                mCapabilities.Capabilities.MaxTextureDimension = D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION;
                mCapabilities.Capabilities.MaxTextureLayers    = 0;
                break;
            default:
                break;
        }

        // Check if we support tearing mode
        ComPtr<IDXGIFactory5> DXGIFactory5;
        if (SUCCEEDED(mDeviceFactory.As<IDXGIFactory5>(& DXGIFactory5)))
        {
            BOOL AllowAdaptive = FALSE;
            CheckIfSucceed(DXGIFactory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING, & AllowAdaptive, sizeof(AllowAdaptive)));
            mProperties.Tearing = AllowAdaptive;
        }

        // Query supported multisample anti-aliasing (MSAA) levels for the device.
        for (UInt32 Level = 1, Last = 1; Level <= kMaxMultisamples; ++Level)
        {
            UInt32 Quality = 0;

            // TODO Check other formats
            const HRESULT Result = mDevice->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, Level, &Quality);

            if (SUCCEEDED(Result) && Quality > 0)
            {
                mProperties.Multisample[Level] = DXGI_SAMPLE_DESC { Level, Quality - 1 };
            }
            else
            {
                mProperties.Multisample[Level] = mProperties.Multisample[Last];
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::LoadStates()
    {
        mStates = DrawPacket();
        mStates.Scissor = Scissor(0, 0, 0, 0);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchain(Ref<D3D11Pass> Pass, HWND Window, UInt16 Width, UInt16 Height, UInt8 Samples)
    {
        const UINT Flags  = (mProperties.Tearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0);

        DXGI_SWAP_CHAIN_DESC Description { };
        Description.BufferCount       = kMaxFrames;
        Description.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Description.Flags             = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | Flags;
        Description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Description.BufferDesc.Width  = Width;
        Description.BufferDesc.Height = Height;
        Description.SampleDesc        = { 1, 0 };
        Description.OutputWindow      = Window;
        Description.Windowed          = true;

        ComPtr<IDXGIFactory4> DXGIFactory4;
        if (SUCCEEDED(mDeviceFactory.As<IDXGIFactory4>(&DXGIFactory4)))
        {
            Description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        }
        else
        {
            Description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        }

        CheckIfSucceed(mDeviceFactory->CreateSwapChain(mDevice.Get(), &Description, mDisplay.GetAddressOf()));
        CheckIfSucceed(mDeviceFactory->MakeWindowAssociation(Description.OutputWindow, DXGI_MWA_NO_WINDOW_CHANGES));

        CreateSwapchainResources(Pass, Width, Height, Samples);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::CreateSwapchainResources(Ref<D3D11Pass> Pass, UInt16 Width, UInt16 Height, UInt8 Samples) const
    {
        Ref<D3D11Attachment> Attachment = Pass.Colors.emplace_back();
        Attachment.SourceLevel  = 0;
        Attachment.TargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
        Attachment.TargetLevel  = 0;

        // Acquire the swapchain back-buffer and create a render target view for it.
        CheckIfSucceed(mDisplay->GetBuffer(
            0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(Attachment.Target.GetAddressOf())));

        // Create an MSAA render target if multisampling is enabled.
        if (Samples > 1)
        {
            D3D11_TEXTURE2D_DESC ColorTextureDescription { };
            ColorTextureDescription.Width      = Width;
            ColorTextureDescription.Height     = Height;
            ColorTextureDescription.MipLevels  = 1;
            ColorTextureDescription.ArraySize  = 1;
            ColorTextureDescription.Format     = DXGI_FORMAT_R8G8B8A8_UNORM;
            ColorTextureDescription.SampleDesc = mProperties.Multisample[Samples];
            ColorTextureDescription.Usage      = D3D11_USAGE_DEFAULT;
            ColorTextureDescription.BindFlags  = D3D11_BIND_RENDER_TARGET;

            CheckIfSucceed(mDevice->CreateTexture2D(
                &ColorTextureDescription, nullptr, reinterpret_cast<ID3D11Texture2D **>(Attachment.Source.GetAddressOf())));

            const CD3D11_RENDER_TARGET_VIEW_DESC ColorViewDescription(
                D3D11_RTV_DIMENSION_TEXTURE2DMS, DXGI_FORMAT_R8G8B8A8_UNORM);
            CheckIfSucceed(mDevice->CreateRenderTargetView(
                Attachment.Source.Get(), &ColorViewDescription, Attachment.Resource.GetAddressOf()));
        }
        else
        {
            CheckIfSucceed(mDevice->CreateRenderTargetView(
                Attachment.Target.Get(), nullptr, Attachment.Resource.GetAddressOf()));
        }

        // Create a depth-stencil buffer matching the swapchain dimensions and sample count.
        D3D11_TEXTURE2D_DESC DepthTextureDescription { };
        DepthTextureDescription.Width      = Width;
        DepthTextureDescription.Height     = Height;
        DepthTextureDescription.MipLevels  = 1;
        DepthTextureDescription.ArraySize  = 1;
        DepthTextureDescription.Format     = DXGI_FORMAT_D24_UNORM_S8_UINT;
        DepthTextureDescription.SampleDesc = mProperties.Multisample[Samples];
        DepthTextureDescription.Usage      = D3D11_USAGE_DEFAULT;
        DepthTextureDescription.BindFlags  = D3D11_BIND_DEPTH_STENCIL;

        ComPtr<ID3D11Texture2D> Depth;
        CheckIfSucceed(mDevice->CreateTexture2D(&DepthTextureDescription, nullptr, Depth.GetAddressOf()));
        CheckIfSucceed(mDevice->CreateDepthStencilView(Depth.Get(), nullptr, Pass.Auxiliary.GetAddressOf()));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyVertexResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const
    {
        Ptr<ID3D11Buffer> Array[DrawPacket::kMaxVertexStreams];
        UINT ArrayOffset[DrawPacket::kMaxVertexStreams];
        UINT ArrayStride[DrawPacket::kMaxVertexStreams];
        UInt Min = DrawPacket::kMaxVertexStreams;
        UInt Max = 0u;

        for (UInt Element = 0; Element < DrawPacket::kMaxVertexStreams; ++Element)
        {
            ConstRef<Stream> Old = Oldest.Vertices[Element];
            ConstRef<Stream> New = Newest.Vertices[Element];

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = Math::Min(Element, Min);
                Max = Math::Max(Element + 1, Max);
            }

            Array[Element]       = mBuffers[New.Buffer].Object.Get();
            ArrayOffset[Element] = New.Offset;
            ArrayStride[Element] = New.Stride;
        }

        if (Min < Max)
        {
            const UInt Count = Max - Min;
            mDeviceImmediate->IASetVertexBuffers(Min, Count, Array + Min, ArrayStride + Min, ArrayOffset + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplySamplerResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest)
    {
        Ptr<ID3D11SamplerState> OldSlots[kMaxResources] { nullptr };
        Ptr<ID3D11SamplerState> NewSlots[kMaxResources] { nullptr };

        UInt MinSlot = kMaxResources;
        UInt MaxSlot = 0;

        for (ConstRef<Entry<Sampler>> Sampler : Oldest.Samplers)
        {
            OldSlots[Sampler.Register] = GetOrCreateSampler(Sampler.Resource).Object.Get();
        }

        for (ConstRef<Entry<Sampler>> Sampler : Newest.Samplers)
        {
            NewSlots[Sampler.Register] = GetOrCreateSampler(Sampler.Resource).Object.Get();
            MinSlot = Min(MinSlot, Sampler.Register);
            MaxSlot = Max(MaxSlot, Sampler.Register + 1);
        }

        UInt Min = kMaxResources;
        UInt Max = 0u;

        for (UInt Slot = MinSlot; Slot < MaxSlot; ++Slot)
        {
            if (OldSlots[Slot] != NewSlots[Slot])
            {
                Min = Math::Min(Slot, Min);
                Max = Math::Max(Slot + 1, Max);
            }
        }

        if (Min < Max)
        {
            const UInt Count = Max - Min;
            mDeviceImmediate->VSSetSamplers(Min, Count, NewSlots + Min);
            mDeviceImmediate->PSSetSamplers(Min, Count, NewSlots + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyTextureResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const
    {
        Ptr<ID3D11ShaderResourceView> OldSlots[kMaxResources] { nullptr };
        Ptr<ID3D11ShaderResourceView> NewSlots[kMaxResources] { nullptr };

        UInt MinSlot = kMaxResources;
        UInt MaxSlot = 0;

        for (ConstRef<Entry<Object>> Texture : Oldest.Textures)
        {
            OldSlots[Texture.Register] = mTextures[Texture.Resource].Resource.Get();
        }

        for (ConstRef<Entry<Object>> Texture : Newest.Textures)
        {
            NewSlots[Texture.Register] = mTextures[Texture.Resource].Resource.Get();
            MinSlot = Min(MinSlot, Texture.Register);
            MaxSlot = Max(MaxSlot, Texture.Register + 1);
        }

        UInt Min = kMaxResources;
        UInt Max = 0u;

        for (UInt Slot = MinSlot; Slot < MaxSlot; ++Slot)
        {
            if (OldSlots[Slot] != NewSlots[Slot])
            {
                Min = Math::Min(Slot, Min);
                Max = Math::Max(Slot + 1, Max);
            }
        }

        if (Min < Max)
        {
            const UInt Count = Max - Min;
            mDeviceImmediate->VSSetShaderResources(Min, Count, NewSlots + Min);
            mDeviceImmediate->PSSetShaderResources(Min, Count, NewSlots + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void D3D11Driver::ApplyUniformResources(ConstRef<DrawPacket> Oldest, ConstRef<DrawPacket> Newest) const
    {
        Ptr<ID3D11Buffer> Array[DrawPacket::kMaxUniformStreams];
        UINT ArrayOffset[DrawPacket::kMaxUniformStreams];
        UINT ArrayLength[DrawPacket::kMaxUniformStreams];
        UInt Min = DrawPacket::kMaxUniformStreams;
        UInt Max = 0u;

        for (UInt Element = 0; Element < DrawPacket::kMaxUniformStreams; ++Element)
        {
            ConstRef<Stream> Old = Oldest.Uniforms[Element];
            ConstRef<Stream> New = Newest.Uniforms[Element];

            if (Old.Buffer != New.Buffer || Old.Offset != New.Offset || Old.Stride != New.Stride)
            {
                Min = Math::Min(Element, Min);
                Max = Math::Max(Element + 1, Max);
            }

            Array[Element]       = mBuffers[New.Buffer].Object.Get();
            ArrayOffset[Element] = New.Offset / 16;
            ArrayLength[Element] = New.Stride / 16;
        }

        if (Min < Max)
        {
            const UInt Count = Max - Min;

            mDeviceImmediate->VSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
            mDeviceImmediate->PSSetConstantBuffers1(Min, Count, Array + Min, ArrayOffset + Min, ArrayLength + Min);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Ref<D3D11Driver::D3D11Sampler> D3D11Driver::GetOrCreateSampler(Sampler Descriptor)
    {
        Ref<D3D11Sampler> Sampler = mSamplers[Hash(Descriptor)];

        if (Sampler.Object == nullptr)
        {
            UINT   Anisotropic = 1;
            Real32 MinLOD      = -FLT_MAX;
            Real32 MaxLOD      = +FLT_MAX;

            switch (Descriptor.Filter)
            {
            case TextureFilter::Point:
            case TextureFilter::Linear:
                MinLOD = 0.0f;
                MaxLOD = 0.0f;
                break;
            case TextureFilter::Anisotropic2x:
                Anisotropic = 2;
                break;
            case TextureFilter::Anisotropic4x:
                Anisotropic = 4;
                break;
            case TextureFilter::Anisotropic8x:
                Anisotropic = 8;
                break;
            case TextureFilter::Anisotropic16x:
                Anisotropic = 16;
                break;
            default:
                break;
            }

            const CD3D11_SAMPLER_DESC SamplerDescriptor(
                As(Descriptor.Filter),
                As(Descriptor.AddressModeU),
                As(Descriptor.AddressModeV),
                As(Descriptor.AddressModeW),
                0,
                Anisotropic,
                As(Descriptor.Comparison),
                As(Descriptor.Border).data(),
                MinLOD,
                MaxLOD);

            CheckIfSucceed(mDevice->CreateSamplerState(&SamplerDescriptor, Sampler.Object.GetAddressOf()));
        }
        return Sampler;
    }
}
