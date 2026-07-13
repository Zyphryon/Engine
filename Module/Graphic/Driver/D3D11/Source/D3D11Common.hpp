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

#include "Zyphryon.Graphic/Types.hpp"
#include <d3d11.h>
#include <wrl/client.h>

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief A smart pointer for COM objects.
    template<typename Type>
    using ComPtr = Microsoft::WRL::ComPtr<Type>;

    /// \brief Converts \ref BlendFactor to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(BlendFactor Value)
    {
        constexpr Array kMapping = {
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

    /// \brief Converts \ref BlendFunction to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(BlendFunction Value)
    {
        constexpr Array kMapping = {
            D3D11_BLEND_OP_ADD,                     // BlendFunction::Add
            D3D11_BLEND_OP_SUBTRACT,                // BlendFunction::Subtract
            D3D11_BLEND_OP_REV_SUBTRACT,            // BlendFunction::ReverseSubtract
            D3D11_BLEND_OP_MIN,                     // BlendFunction::Min
            D3D11_BLEND_OP_MAX                      // BlendFunction::Max
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Cull to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(Cull Value)
    {
        constexpr Array kMapping = {
            D3D11_CULL_NONE,                // Cull::None
            D3D11_CULL_BACK,                // Cull::Back
            D3D11_CULL_FRONT,               // Cull::Front
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Fill to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(Fill Value)
    {
        constexpr Array kMapping = {
            D3D11_FILL_SOLID,               // Fill::Solid
            D3D11_FILL_WIREFRAME,           // Fill::Wireframe
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Primitive to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(Primitive Value)
    {
        constexpr Array kMapping = {
            D3D11_PRIMITIVE_TOPOLOGY_POINTLIST,        // Primitive::PointList
            D3D11_PRIMITIVE_TOPOLOGY_LINELIST,         // Primitive::LineList
            D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP,        // Primitive::LineStrip
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST,     // Primitive::TriangleList
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,    // Primitive::TriangleStrip
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Storage to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(Storage Value)
    {
        constexpr Array kMapping = {
            D3D11_USAGE_IMMUTABLE,           // Storage::Immutable
            D3D11_USAGE_DYNAMIC,             // Storage::Dynamic
            D3D11_USAGE_DEFAULT,             // Storage::Stream
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TestAction to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TestAction Value)
    {
        constexpr Array kMapping = {
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

    /// \brief Converts \ref TestCondition to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TestCondition Value)
    {
        constexpr Array kMapping = {
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

    /// \brief Converts \ref TextureAddress to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TextureAddress Value)
    {
        constexpr Array kMapping = {
            D3D11_TEXTURE_ADDRESS_CLAMP,            // TextureAddress::Clamp
            D3D11_TEXTURE_ADDRESS_BORDER,           // TextureAddress::Border
            D3D11_TEXTURE_ADDRESS_WRAP,             // TextureAddress::Repeat
            D3D11_TEXTURE_ADDRESS_MIRROR            // TextureAddress::Mirror
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TextureBorder to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TextureBorder Value)
    {
        static constexpr FLOAT kMapping[][4] = {
            { 0.0f, 0.0f, 0.0f, 0.0f },     // OpaqueBlack
            { 1.0f, 1.0f, 1.0f, 0.0f },     // OpaqueWhite
            { 0.0f, 0.0f, 0.0f, 1.0f },     // TransparentBlack
            { 1.0f, 1.0f, 1.0f, 1.0f }      // TransparentWhite
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref TextureFilter to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TextureFilter Value)
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

    /// \brief Converts \ref TextureFormat to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(TextureFormat Value)
    {
        constexpr static Array kMapping = {
            DXGI_FORMAT_UNKNOWN,                // TextureFormat::Unspecified
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
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBUIntNorm
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBUIntNorm_sRGB
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBA1UIntNorm
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBA1UIntNorm_sRGB
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBAUIntNorm
            DXGI_FORMAT_UNKNOWN,                // ETC2RGBAUIntNorm_sRGB
            DXGI_FORMAT_UNKNOWN,                // EACR11UIntNorm
            DXGI_FORMAT_UNKNOWN,                // EACR11SIntNorm
            DXGI_FORMAT_UNKNOWN,                // EACRG11UIntNorm
            DXGI_FORMAT_UNKNOWN,                // EACRG11SIntNorm
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
            DXGI_FORMAT_R16_TYPELESS,           // TextureFormat::D16UIntNorm
            DXGI_FORMAT_R32_TYPELESS,           // TextureFormat::D32Float
            DXGI_FORMAT_R24G8_TYPELESS,         // TextureFormat::D24S8UIntNorm
            DXGI_FORMAT_R32G8X24_TYPELESS,      // TextureFormat::D32S8UIntNorm
        };
        return kMapping[Enum::Cast(Value)];
    }

    /// \brief Converts \ref Usage to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(Usage Value)
    {
        UINT Flags = 0;

        if (HasBit(Value, Usage::Vertex))
        {
            Flags |= D3D11_BIND_VERTEX_BUFFER;
        }
        if (HasBit(Value, Usage::Index))
        {
            Flags |= D3D11_BIND_INDEX_BUFFER;
        }
        if (HasBit(Value, Usage::Uniform))
        {
            Flags |= D3D11_BIND_CONSTANT_BUFFER;
        }
        if (HasBit(Value, Usage::Sample))
        {
            Flags |= D3D11_BIND_SHADER_RESOURCE;
        }
        if (HasBit(Value, Usage::Target))
        {
            Flags |= D3D11_BIND_RENDER_TARGET;
        }
        return static_cast<D3D11_BIND_FLAG>(Flags);
    }

    /// \brief Converts \ref VertexFormat to the corresponding D3D11 enumeration.
    constexpr auto D3D11Convert(VertexFormat Value)
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
}