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

#include "Common.hpp"
#include "Zyphryon.Content/Resource.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Graphic
{
    /// \brief Represents a texture asset within the content system.
    class Texture final : public Content::AbstractResource<Texture>
    {
        friend class AbstractResource;

    public:

        /// \brief Constructs a texture resource with the given content key.
        ///
        /// \param Key The unique content key identifying this texture.
        explicit Texture(AnyRef<Content::Uri> Key);

        /// \brief Loads the texture with specified parameters and raw data.
        ///
        /// \param Format  The pixel format used for storage and sampling.
        /// \param Access  The access mode for the texture (e.g., static, dynamic, stream).
        /// \param Usage   The intended usage of the texture (e.g., sample, render target).
        /// \param Width   The width of the texture in pixels.
        /// \param Height  The height of the texture in pixels.
        /// \param Level   The mipmap level of the texture to load.
        /// \param Samples The multisample count for multisampled textures.
        /// \param Data    The raw pixel data blob to load into the texture.
        void Load(TextureFormat Format, Access Access, Usage Usage, UInt16 Width, UInt16 Height, UInt8 Level, Multisample Samples, AnyRef<Blob> Data);

        /// \brief Loads the texture with default access and usage, using the provided parameters and raw data.
        ///
        /// \param Format The pixel format used for storage and sampling.
        /// \param Width  The width of the texture in pixels.
        /// \param Height The height of the texture in pixels.
        /// \param Level  The mipmap level of the texture to load.
        /// \param Data   The raw pixel data blob to load into the texture.
        ZYPHRYON_INLINE void Load(TextureFormat Format, UInt16 Width, UInt16 Height, UInt8 Level, AnyRef<Blob> Data)
        {
            Load(Format, Access::Stream, Usage::Sample, Width, Height, Level, Multisample::X1, Move(Data));
        }

        /// \brief Returns the GPU object ID associated with this texture.
        ///
        /// \return The driver-level texture handle.
        ZYPHRYON_INLINE Object GetID() const
        {
            return mID;
        }

        /// \brief Returns the access mode of the texture.
        ///
        /// \return The access mode.
        ZYPHRYON_INLINE Access GetAccess() const
        {
            return mAccess;
        }

        /// \brief Returns the type of the texture.
        ///
        /// \return The texture type.
        ZYPHRYON_INLINE TextureType GetType() const
        {
            return mType;
        }

        /// \brief Returns the pixel format of the texture.
        ///
        /// \return The format used for storage and sampling.
        ZYPHRYON_INLINE TextureFormat GetFormat() const
        {
            return mFormat;
        }

        /// \brief Returns the usage pattern of the texture.
        ///
        /// \return The intended usage of the texture.
        ZYPHRYON_INLINE Usage GetUsage() const
        {
            return mUsage;
        }

        /// \brief Returns the width of the texture in pixels.
        ///
        /// \return The width in pixels.
        ZYPHRYON_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Returns the height of the texture in pixels.
        ///
        /// \return The height in pixels.
        ZYPHRYON_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Returns the mipmap level.
        ///
        /// \return The mipmap level in the texture.
        ZYPHRYON_INLINE UInt8 GetLevel() const
        {
            return mLevel;
        }

        /// \brief Returns the multisample count used for multisampling.
        ///
        /// \return The multisample count.
        ZYPHRYON_INLINE Multisample GetSamples() const
        {
            return mSamples;
        }

    private:

        /// \copydoc Resource::OnCreate(Ref<Service::Host>)
        Bool OnCreate(Ref<Service::Host> Host) override;

        /// \copydoc Resource::OnDelete(Ref<Service::Host>)
        void OnDelete(Ref<Service::Host> Host) override;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Object        mID;
        TextureType   mType;
        TextureFormat mFormat;
        Access        mAccess;
        Usage         mUsage;
        UInt16        mWidth;
        UInt16        mHeight;
        UInt8         mLevel;
        Multisample   mSamples;
        Blob          mData;
    };
}