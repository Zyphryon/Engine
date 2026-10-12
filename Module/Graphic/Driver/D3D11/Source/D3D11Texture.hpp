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

#include "D3D11Device.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents a Direct3D 11 texture and the view shaders sample it through.
    class Texture final
    {
    public:

        /// \brief Constructs an empty texture that owns no resource until \ref Create.
        Texture();

        /// \brief Creates the texture with all its levels, and the view shaders sample it through.
        ///
        /// \param Device  The device the texture is created on.
        /// \param Layout  The layout of the texture (2D, array or cube).
        /// \param Format  The pixel format of the texture.
        /// \param Storage The update frequency the texture is created for.
        /// \param Usage   The bindings the texture is drawn from or rendered into.
        /// \param Width   The width of the top level, in pixels.
        /// \param Height  The height of the top level, in pixels.
        /// \param Layers  The number of array slices, ignored for a cube.
        /// \param Levels  The number of mip levels.
        /// \param Data    The initial contents, slice-major, or empty to leave the texture undefined.
        void Create(ConstRef<Device> Device, TextureLayout Layout, TextureFormat Format, Storage Storage, Usage Usage, UInt16 Width, UInt16 Height, UInt16 Layers, UInt8 Levels, ConstSpan<Byte> Data);

        /// \brief Writes a region of one level of one slice.
        ///
        /// \param Device The device whose context records the write.
        /// \param Level  The mip level written.
        /// \param Layer  The array slice written.
        /// \param X      The left edge of the region, in pixels.
        /// \param Y      The top edge of the region, in pixels.
        /// \param Width  The width of the region, in pixels.
        /// \param Height The height of the region, in pixels.
        /// \param Pitch  The distance between rows of the data, in bytes.
        /// \param Data   The pixels to write.
        void Update(ConstRef<Device> Device, UInt8 Level, UInt16 Layer, UInt16 X, UInt16 Y, UInt16 Width, UInt16 Height, UInt32 Pitch, ConstSpan<Byte> Data);

        /// \brief Releases the texture and its view and returns to the empty state.
        void Delete();

        /// \brief Gets the texture resource.
        ///
        /// \return The texture, or `nullptr` when none was created.
        ZY_INLINE Ptr<ID3D11Texture2D> GetHandle() const
        {
            return mHandle.Get();
        }

        /// \brief Gets the view shaders sample the texture through.
        ///
        /// \return The view, or `nullptr` when the texture is not sampled.
        ZY_INLINE Ptr<ID3D11ShaderResourceView> GetView() const
        {
            return mView.Get();
        }

        /// \brief Gets the format the texture was created with.
        ///
        /// \return The pixel format of the texture.
        ZY_INLINE TextureFormat GetFormat() const
        {
            return mFormat;
        }

        /// \brief Gets the number of array slices.
        ///
        /// \return The number of slices, six for a cube.
        ZY_INLINE UInt16 GetLayers() const
        {
            return mLayers;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        ComPtr<ID3D11Texture2D>          mHandle;
        ComPtr<ID3D11ShaderResourceView> mView;
        TextureFormat                    mFormat;
        UInt8                            mLevels;
        UInt16                           mLayers;
    };
}