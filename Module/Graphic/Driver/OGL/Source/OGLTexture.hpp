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

#include "OGLDevice.hpp"
#include "OGLCommon.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents an OpenGL texture, a 2D, array or cube texture with all its levels allocated at once.
    class Texture final
    {
    public:

        /// \brief Constructs an empty texture that owns no OpenGL object until \ref Create.
        Texture();

        /// \brief Creates the texture object, allocates every level of every slice and uploads the initial contents.
        ///
        /// \note The context must have the format; its description leaves out S3TC sRGB and 16-bit normalized.
        ///
        /// \param Device The device of the context.
        /// \param Layout The kind of texture to create.
        /// \param Format The format the texels are stored in.
        /// \param Width  The width of the base level, in texels.
        /// \param Height The height of the base level, in texels.
        /// \param Layers The number of array layers, ignored for cube textures.
        /// \param Levels The number of mip levels.
        /// \param Data   The initial contents, slice-major, or empty to leave the texture undefined.
        void Create(ConstRef<Device> Device, TextureLayout Layout, TextureFormat Format, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data);

        /// \brief Writes a region of one level of one slice.
        ///
        /// \param Level  The mip level to write.
        /// \param Layer  The array layer or cube face to write.
        /// \param X      The left edge of the region, in texels.
        /// \param Y      The top edge of the region, in texels.
        /// \param Width  The width of the region, in texels.
        /// \param Height The height of the region, in texels.
        /// \param Pitch  The distance between rows of the source, in bytes.
        /// \param Data   The texels to write.
        void Update(UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data);

        /// \brief Attaches one level of one slice to the framebuffer bound at a target.
        ///
        /// \param Framebuffer The target the framebuffer is bound to.
        /// \param Point       The attachment point the level goes to.
        /// \param Level       The mip level attached.
        /// \param Layer       The array layer or cube face attached.
        void Attach(GLenum Framebuffer, GLenum Point, UInt8 Level, UInt16 Layer) const;

        /// \brief Deletes the texture object and returns to the empty state.
        void Delete();

        /// \brief Gets the texture object.
        ///
        /// \return The OpenGL name of the texture, `0` when none was created.
        ZY_INLINE GLuint GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the target the texture is bound to.
        ///
        /// \return The binding target the texture's layout maps to.
        ZY_INLINE GLenum GetTarget() const
        {
            return mTarget;
        }

        /// \brief Gets the format the texture was created with.
        ///
        /// \return The engine format of the texels.
        ZY_INLINE TextureFormat GetFormat() const
        {
            return mFormat;
        }

    private:

        /// \brief Writes a region of one level of one face or layer, compressed or not.
        ///
        /// \param Face   The target written, a cube face, the 2D target or the array target.
        /// \param Format The description of the format the texels are stored in.
        /// \param Level  The mip level written.
        /// \param Layer  The array layer written, ignored outside an array.
        /// \param X      The left edge of the region, in texels.
        /// \param Y      The top edge of the region, in texels.
        /// \param Width  The width of the region, in texels.
        /// \param Height The height of the region, in texels.
        /// \param Size   The size of the compressed data, in bytes, ignored for an uncompressed format.
        /// \param Bytes  The texels to write.
        void Upload(GLenum Face, ConstRef<Format> Format, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Size, ConstPtr<Byte> Bytes) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLuint        mHandle;
        GLenum        mTarget;
        TextureFormat mFormat;
    };
}