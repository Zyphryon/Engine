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

#include "Service.hpp"
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents an immutable, GPU-backed texture asset intended for sampling.
    class Image final : public Content::AbstractResource<Image>
    {
    public:

        /// \brief Constructs an image resource with the given content key.
        ///
        /// \param Key The unique content key identifying this image.
        explicit Image(AnyRef<Content::Uri> Key);

        /// \brief Sets up the image with specified parameters and raw data.
        ///
        /// \param Layout  The memory layout of the image data.
        /// \param Format  The pixel format of the image data.
        /// \param Width   The width of the image in pixels.
        /// \param Height  The height of the image in pixels.
        /// \param Mipmaps The number of mipmap levels in the image data.
        /// \param Data    The blob containing the raw image pixel data.
        void Setup(TextureLayout Layout, TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Mipmaps, AnyRef<Blob> Data);

        /// \brief Gets the native GPU handle for this image.
        ///
        /// \return The GPU object handle.
        ZY_INLINE Object GetHandle() const
        {
            return mHandle;
        }

        /// \brief Gets the texture layout of this image.
        ///
        /// \return The texture layout.
        ZY_INLINE TextureLayout GetLayout() const
        {
            return mLayout;
        }

        /// \brief Gets the texture format of this image.
        ///
        /// \return The texture format.
        ZY_INLINE TextureFormat GetFormat() const
        {
            return mFormat;
        }

        /// \brief Gets the width of this image in pixels.
        ///
        /// \return The image width.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the height of this image in pixels.
        ///
        /// \return The image height.
        ZY_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Gets the number of mipmap levels in this image.
        ///
        /// \return The mipmap count.
        ZY_INLINE UInt8 GetMipmaps() const
        {
            return mMipmaps;
        }

    public:

        /// \brief Uploads the image data to the GPU, creating the texture resource.
        ///
        /// \param Service The graphic service used to create the resource.
        /// \return `true` if the upload succeeded, `false` otherwise.
        Bool Upload(Ref<Service> Service);

        /// \brief Unloads the image data from the GPU, destroying the texture resource.
        ///
        /// \param Service The graphic service used to destroy the resource.
        void Unload(Ref<Service> Service);

        /// \see Content::Resource::OnCreate(Ref<Engine::Subsystem::Host>)
        Bool OnCreate(Ref<Engine::Subsystem::Host> Host) override
        {
            return Upload(* Host.GetService<Service>());
        }

        /// \see Content::Resource::OnDelete(Ref<Engine::Subsystem::Host>)
        void OnDelete(Ref<Engine::Subsystem::Host> Host) override
        {
            Unload(* Host.GetService<Service>());
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object        mHandle;
        TextureLayout mLayout;
        TextureFormat mFormat;
        UInt16        mWidth;
        UInt16        mHeight;
        UInt8         mMipmaps;
        Blob          mData;
    };
}