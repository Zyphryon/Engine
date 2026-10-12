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

#include "OGLCommon.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    ConstRef<Format> Convert(TextureFormat Value)
    {
        static constexpr Format kMapping[] = {
            { 0x0000,                          0,                  0,                                 false },    // TextureFormat::Unspecified
            { 0x83F1,                          0,                  0,                                 true  },    // TextureFormat::BC1UIntNorm
            { 0x8C4D,                          0,                  0,                                 true  },    // TextureFormat::BC1UIntNorm_sRGB
            { 0x83F2,                          0,                  0,                                 true  },    // TextureFormat::BC2UIntNorm
            { 0x8C4E,                          0,                  0,                                 true  },    // TextureFormat::BC2UIntNorm_sRGB
            { 0x83F3,                          0,                  0,                                 true  },    // TextureFormat::BC3UIntNorm
            { 0x8C4F,                          0,                  0,                                 true  },    // TextureFormat::BC3UIntNorm_sRGB
            { 0x8DBB,                          0,                  0,                                 true  },    // TextureFormat::BC4UIntNorm
            { 0x8DBC,                          0,                  0,                                 true  },    // TextureFormat::BC4SIntNorm
            { 0x8DBD,                          0,                  0,                                 true  },    // TextureFormat::BC5UIntNorm
            { 0x8DBE,                          0,                  0,                                 true  },    // TextureFormat::BC5SIntNorm
            { 0x8E8F,                          0,                  0,                                 true  },    // TextureFormat::BC6UFloat
            { 0x8E8E,                          0,                  0,                                 true  },    // TextureFormat::BC6SFloat
            { 0x8E8C,                          0,                  0,                                 true  },    // TextureFormat::BC7UIntNorm
            { 0x8E8D,                          0,                  0,                                 true  },    // TextureFormat::BC7UIntNorm_sRGB
            { GL_COMPRESSED_RGB8_ETC2,         0,                  0,                                 true  },    // TextureFormat::ETC2RGBUIntNorm
            { GL_COMPRESSED_SRGB8_ETC2,        0,                  0,                                 true  },    // TextureFormat::ETC2RGBUIntNorm_sRGB
            { 0x9276,                          0,                  0,                                 true  },    // TextureFormat::ETC2RGBA1UIntNorm
            { 0x9277,                          0,                  0,                                 true  },    // TextureFormat::ETC2RGBA1UIntNorm_sRGB
            { GL_COMPRESSED_RGBA8_ETC2_EAC,    0,                  0,                                 true  },    // TextureFormat::ETC2RGBAUIntNorm
            { 0x9279,                          0,                  0,                                 true  },    // TextureFormat::ETC2RGBAUIntNorm_sRGB
            { GL_COMPRESSED_R11_EAC,           0,                  0,                                 true  },    // TextureFormat::EACR11UIntNorm
            { GL_COMPRESSED_SIGNED_R11_EAC,    0,                  0,                                 true  },    // TextureFormat::EACR11SIntNorm
            { GL_COMPRESSED_RG11_EAC,          0,                  0,                                 true  },    // TextureFormat::EACRG11UIntNorm
            { GL_COMPRESSED_SIGNED_RG11_EAC,   0,                  0,                                 true  },    // TextureFormat::EACRG11SIntNorm
            { GL_R8I,                          GL_RED_INTEGER,     GL_BYTE,                           false },    // TextureFormat::R8SInt
            { GL_R8_SNORM,                     GL_RED,             GL_BYTE,                           false },    // TextureFormat::R8SIntNorm
            { GL_R8UI,                         GL_RED_INTEGER,     GL_UNSIGNED_BYTE,                  false },    // TextureFormat::R8UInt
            { GL_R8,                           GL_RED,             GL_UNSIGNED_BYTE,                  false },    // TextureFormat::R8UIntNorm
            { GL_R16I,                         GL_RED_INTEGER,     GL_SHORT,                          false },    // TextureFormat::R16SInt
            { 0x8F98,                          GL_RED,             GL_SHORT,                          false },    // TextureFormat::R16SIntNorm
            { GL_R16UI,                        GL_RED_INTEGER,     GL_UNSIGNED_SHORT,                 false },    // TextureFormat::R16UInt
            { 0x822A,                          GL_RED,             GL_UNSIGNED_SHORT,                 false },    // TextureFormat::R16UIntNorm
            { GL_R16F,                         GL_RED,             GL_HALF_FLOAT,                     false },    // TextureFormat::R16Float
            { GL_R32I,                         GL_RED_INTEGER,     GL_INT,                            false },    // TextureFormat::R32SInt
            { GL_R32UI,                        GL_RED_INTEGER,     GL_UNSIGNED_INT,                   false },    // TextureFormat::R32UInt
            { GL_R32F,                         GL_RED,             GL_FLOAT,                          false },    // TextureFormat::R32Float
            { GL_RG8I,                         GL_RG_INTEGER,      GL_BYTE,                           false },    // TextureFormat::RG8SInt
            { GL_RG8_SNORM,                    GL_RG,              GL_BYTE,                           false },    // TextureFormat::RG8SIntNorm
            { GL_RG8UI,                        GL_RG_INTEGER,      GL_UNSIGNED_BYTE,                  false },    // TextureFormat::RG8UInt
            { GL_RG8,                          GL_RG,              GL_UNSIGNED_BYTE,                  false },    // TextureFormat::RG8UIntNorm
            { GL_RG16I,                        GL_RG_INTEGER,      GL_SHORT,                          false },    // TextureFormat::RG16SInt
            { 0x8F99,                          GL_RG,              GL_SHORT,                          false },    // TextureFormat::RG16SIntNorm
            { GL_RG16UI,                       GL_RG_INTEGER,      GL_UNSIGNED_SHORT,                 false },    // TextureFormat::RG16UInt
            { 0x822C,                          GL_RG,              GL_UNSIGNED_SHORT,                 false },    // TextureFormat::RG16UIntNorm
            { GL_RG16F,                        GL_RG,              GL_HALF_FLOAT,                     false },    // TextureFormat::RG16Float
            { GL_RG32I,                        GL_RG_INTEGER,      GL_INT,                            false },    // TextureFormat::RG32SInt
            { GL_RG32UI,                       GL_RG_INTEGER,      GL_UNSIGNED_INT,                   false },    // TextureFormat::RG32UInt
            { GL_RG32F,                        GL_RG,              GL_FLOAT,                          false },    // TextureFormat::RG32Float
            { GL_RGB32I,                       GL_RGB_INTEGER,     GL_INT,                            false },    // TextureFormat::RGB32SInt
            { GL_RGB32UI,                      GL_RGB_INTEGER,     GL_UNSIGNED_INT,                   false },    // TextureFormat::RGB32UInt
            { GL_RGB32F,                       GL_RGB,             GL_FLOAT,                          false },    // TextureFormat::RGB32Float
            { GL_RGBA8I,                       GL_RGBA_INTEGER,    GL_BYTE,                           false },    // TextureFormat::RGBA8SInt
            { GL_RGBA8_SNORM,                  GL_RGBA,            GL_BYTE,                           false },    // TextureFormat::RGBA8SIntNorm
            { GL_RGBA8UI,                      GL_RGBA_INTEGER,    GL_UNSIGNED_BYTE,                  false },    // TextureFormat::RGBA8UInt
            { GL_RGBA8,                        GL_RGBA,            GL_UNSIGNED_BYTE,                  false },    // TextureFormat::RGBA8UIntNorm
            { GL_SRGB8_ALPHA8,                 GL_RGBA,            GL_UNSIGNED_BYTE,                  false },    // TextureFormat::RGBA8UIntNorm_sRGB
            { GL_RGBA16I,                      GL_RGBA_INTEGER,    GL_SHORT,                          false },    // TextureFormat::RGBA16SInt
            { 0x8F9B,                          GL_RGBA,            GL_SHORT,                          false },    // TextureFormat::RGBA16SIntNorm
            { GL_RGBA16UI,                     GL_RGBA_INTEGER,    GL_UNSIGNED_SHORT,                 false },    // TextureFormat::RGBA16UInt
            { 0x805B,                          GL_RGBA,            GL_UNSIGNED_SHORT,                 false },    // TextureFormat::RGBA16UIntNorm
            { GL_RGBA16F,                      GL_RGBA,            GL_HALF_FLOAT,                     false },    // TextureFormat::RGBA16Float
            { GL_RGBA32I,                      GL_RGBA_INTEGER,    GL_INT,                            false },    // TextureFormat::RGBA32SInt
            { GL_RGBA32UI,                     GL_RGBA_INTEGER,    GL_UNSIGNED_INT,                   false },    // TextureFormat::RGBA32UInt
            { GL_RGBA32F,                      GL_RGBA,            GL_FLOAT,                          false },    // TextureFormat::RGBA32Float
            { GL_RGB10_A2UI,                   GL_RGBA_INTEGER,    GL_UNSIGNED_INT_2_10_10_10_REV,    false },    // TextureFormat::RGB10A2UInt
            { GL_RGB10_A2,                     GL_RGBA,            GL_UNSIGNED_INT_2_10_10_10_REV,    false },    // TextureFormat::RGB10A2UIntNorm
            { GL_R11F_G11F_B10F,               GL_RGB,             GL_UNSIGNED_INT_10F_11F_11F_REV,   false },    // TextureFormat::R11G11B10Float
            { GL_RGB9_E5,                      GL_RGB,             GL_UNSIGNED_INT_5_9_9_9_REV,       false },    // TextureFormat::RGB9E5Float
            { GL_DEPTH_COMPONENT16,            GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT,                 false },    // TextureFormat::D16UIntNorm
            { GL_DEPTH_COMPONENT32F,           GL_DEPTH_COMPONENT, GL_FLOAT,                          false },    // TextureFormat::D32Float
            { GL_DEPTH24_STENCIL8,             GL_DEPTH_STENCIL,   GL_UNSIGNED_INT_24_8,              false },    // TextureFormat::D24S8UIntNorm
            { GL_DEPTH32F_STENCIL8,            GL_DEPTH_STENCIL,   GL_FLOAT_32_UNSIGNED_INT_24_8_REV, false },    // TextureFormat::D32S8UIntNorm
        };
        return kMapping[ZyEnum::Cast(Value)];
    }
}