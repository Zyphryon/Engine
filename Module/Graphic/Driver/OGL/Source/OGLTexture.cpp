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

#include "OGLTexture.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Bool IsSupported(ConstRef<Device> Device, TextureFormat Format)
    {
        switch (Format)
        {
        case TextureFormat::BC1UIntNorm:
        case TextureFormat::BC2UIntNorm:
        case TextureFormat::BC3UIntNorm:
            return Device.Has(Feature::S3TC);
        case TextureFormat::BC1UIntNorm_sRGB:
        case TextureFormat::BC2UIntNorm_sRGB:
        case TextureFormat::BC3UIntNorm_sRGB:
            return Device.Has(Feature::S3TCsRGB);
        case TextureFormat::BC4UIntNorm:
        case TextureFormat::BC4SIntNorm:
        case TextureFormat::BC5UIntNorm:
        case TextureFormat::BC5SIntNorm:
            return Device.Has(Feature::RGTC);
        case TextureFormat::BC6UFloat:
        case TextureFormat::BC6SFloat:
        case TextureFormat::BC7UIntNorm:
        case TextureFormat::BC7UIntNorm_sRGB:
            return Device.Has(Feature::BPTC);
        case TextureFormat::ETC2RGBUIntNorm:
        case TextureFormat::ETC2RGBUIntNorm_sRGB:
        case TextureFormat::ETC2RGBA1UIntNorm:
        case TextureFormat::ETC2RGBA1UIntNorm_sRGB:
        case TextureFormat::ETC2RGBAUIntNorm:
        case TextureFormat::ETC2RGBAUIntNorm_sRGB:
        case TextureFormat::EACR11UIntNorm:
        case TextureFormat::EACR11SIntNorm:
        case TextureFormat::EACRG11UIntNorm:
        case TextureFormat::EACRG11SIntNorm:
            return Device.Has(Feature::ETC2);
        case TextureFormat::R16SIntNorm:
        case TextureFormat::R16UIntNorm:
        case TextureFormat::RG16SIntNorm:
        case TextureFormat::RG16UIntNorm:
        case TextureFormat::RGBA16SIntNorm:
        case TextureFormat::RGBA16UIntNorm:
            return Device.Has(Feature::Norm16);
        default:
            return true;
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Texture::Texture()
        : mHandle { 0 },
          mTarget { GL_TEXTURE_2D },
          mFormat { TextureFormat::Unspecified }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Create(ConstRef<Device> Device, TextureLayout Layout, TextureFormat Format, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data)
    {
        ZY_ASSERT(IsSupported(Device, Format), "OpenGL: The context lacks the feature the texture's format needs");

        const Bool   IsCube  = (Layout == TextureLayout::TextureCube);
        const Bool   IsArray = (Layout == TextureLayout::Texture2DArray);
        const UInt16 Slices  = IsCube ? 6 : Max<UInt16>(1, Layers);

        mFormat = Format;
        mTarget = IsCube ? GL_TEXTURE_CUBE_MAP : (IsArray ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D);

        ConstRef<OGL::Format> Description = Convert(Format);

        glGenTextures(1, AddressOf(mHandle));
        glBindTexture(mTarget, mHandle);

        ConstPtr<Byte> Bytes = Data.GetData();
        const UInt8    Count = Max<UInt8>(1, Levels);

        // Every level of every slice is allocated at once, so the uploads below only fill what already exists.
        if (IsArray)
        {
            glTexStorage3D(GL_TEXTURE_2D_ARRAY, Count, Description.Internal, Width, Height, Slices);
        }
        else
        {
            glTexStorage2D(mTarget, Count, Description.Internal, Width, Height);
        }

        // The source is ordered slice-major, with every level of one slice before the next slice begins.
        for (UInt16 Slice = 0; Slice < Slices && Bytes; ++Slice)
        {
            const GLenum Face = IsCube ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + Slice) : mTarget;

            for (UInt8 Level = 0; Level < Count; ++Level)
            {
                const UInt16 LevelWidth  = GetLevelExtent(Width, Level);
                const UInt16 LevelHeight = GetLevelExtent(Height, Level);
                const UInt32 Size        = GetLevelSize(Format, Width, Height, Level);

                Upload(Face, Description, Level, Slice, 0, 0, LevelWidth, LevelHeight, Size, Bytes);

                Bytes += Size;
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Update(UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data)
    {
        ConstRef<Format> Description = Convert(mFormat);

        const GLenum Face = (mTarget == GL_TEXTURE_CUBE_MAP) ? (GL_TEXTURE_CUBE_MAP_POSITIVE_X + Layer) : mTarget;

        glBindTexture(mTarget, mHandle);

        if (Description.Compressed)
        {
            const UInt32 Size = GetLevelSize(mFormat, Width, Height, 0);

            Upload(Face, Description, Level, Layer, X, Y, Width, Height, Size, Data.GetData());
        }
        else
        {
            const UInt32 BytesPerPixel = GetTextureMetadata(mFormat).BitsPerPixel / 8;
            const UInt32 Row           = BytesPerPixel ? Pitch / BytesPerPixel : 0;

            if (Row == Width)
            {
                Upload(Face, Description, Level, Layer, X, Y, Width, Height, 0, Data.GetData());
            }
            else
            {
                glPixelStorei(GL_UNPACK_ROW_LENGTH, Row);
                Upload(Face, Description, Level, Layer, X, Y, Width, Height, 0, Data.GetData());
                glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Attach(GLenum Framebuffer, GLenum Point, UInt8 Level, UInt16 Layer) const
    {
        if (mTarget == GL_TEXTURE_2D_ARRAY)
        {
            glFramebufferTextureLayer(Framebuffer, Point, mHandle, Level, Layer);
        }
        else if (mTarget == GL_TEXTURE_CUBE_MAP)
        {
            glFramebufferTexture2D(Framebuffer, Point, GL_TEXTURE_CUBE_MAP_POSITIVE_X + Layer, mHandle, Level);
        }
        else
        {
            glFramebufferTexture2D(Framebuffer, Point, GL_TEXTURE_2D, mHandle, Level);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Delete()
    {
        glDeleteTextures(1, AddressOf(mHandle));

        mHandle = 0;
        mTarget = GL_TEXTURE_2D;
        mFormat = TextureFormat::Unspecified;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Texture::Upload(GLenum Face, ConstRef<Format> Format, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Size, ConstPtr<Byte> Bytes) const
    {
        if (Face == GL_TEXTURE_2D_ARRAY)
        {
            if (Format.Compressed)
            {
                glCompressedTexSubImage3D(
                    GL_TEXTURE_2D_ARRAY,
                    Level,
                    X,
                    Y,
                    Layer,
                    Width,
                    Height,
                    1,
                    Format.Internal,
                    Size,
                    Bytes);
            }
            else
            {
                glTexSubImage3D(
                    GL_TEXTURE_2D_ARRAY,
                    Level,
                    X,
                    Y,
                    Layer,
                    Width,
                    Height,
                    1,
                    Format.External,
                    Format.Type,
                    Bytes);
            }
        }
        else
        {
            if (Format.Compressed)
            {
                glCompressedTexSubImage2D(Face, Level, X, Y, Width, Height, Format.Internal, Size, Bytes);
            }
            else
            {
                glTexSubImage2D(Face, Level, X, Y, Width, Height, Format.External, Format.Type, Bytes);
            }
        }
    }
}