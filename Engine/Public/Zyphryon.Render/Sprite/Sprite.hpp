// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Content/Proxy.hpp"
#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Math/Color.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents a 2D sprite with rendering properties.
    class Sprite final
    {
    public:

        /// \brief Default constructor initializing default sprite properties.
        ZYPHRYON_INLINE Sprite() = default;

        /// \brief Constructs a sprite with specified properties.
        ///
        /// \param Material The material used to render the sprite.
        /// \param Size     The size of the sprite in pixels.
        /// \param Tint     The tint color applied to the sprite.
        /// \param Origin   The origin point for sprite alignment.
        /// \param Frame    The frame rectangle defining the sprite's texture region.
        ZYPHRYON_INLINE Sprite(ConstTracker<Graphic::Material> Material, Vector2 Size, IntColor8 Tint, Pivot Origin, Rect Frame)
            : mMaterial { Material },
              mSize     { Size },
              mTint     { Tint },
              mOrigin   { Origin },
              mFrame    { Frame }
        {
        }

        /// \brief Sets the material used to render the sprite.
        ///
        /// \param Material The material to set.
        ZYPHRYON_INLINE void SetMaterial(ConstTracker<Graphic::Material> Material)
        {
            mMaterial = Content::Proxy(Material);
        }

        /// \brief Returns the material used to render the sprite.
        ///
        /// \return The current material.
        ZYPHRYON_INLINE ConstTracker<Graphic::Material> GetMaterial() const
        {
            return mMaterial.GetResource();
        }

        /// \brief Sets the size of the sprite.
        ///
        /// \param Size The size to set.
        ZYPHRYON_INLINE void SetSize(Vector2 Size)
        {
            mSize = Size;
        }

        /// \brief Returns the size of the sprite.
        ///
        /// \return The current size.
        ZYPHRYON_INLINE Vector2 GetSize() const
        {
            return mSize;
        }

        /// \brief Sets the tint color applied to the sprite.
        ///
        /// \param Tint The tint color to set.
        ZYPHRYON_INLINE void SetTint(IntColor8 Tint)
        {
            mTint = Tint;
        }

        /// \brief Returns the tint color applied to the sprite.
        ///
        /// \return The current tint color.
        ZYPHRYON_INLINE IntColor8 GetTint() const
        {
            return mTint;
        }

        /// \brief Sets the origin point for sprite alignment.
        ///
        /// \param Origin The origin to set.
        ZYPHRYON_INLINE void SetOrigin(Pivot Origin)
        {
            mOrigin = Origin;
        }

        /// \brief Returns the origin point for sprite alignment.
        ///
        /// \return The current origin.
        ZYPHRYON_INLINE Pivot GetOrigin() const
        {
            return mOrigin;
        }

        /// \brief Sets the frame rectangle defining the sprite's texture region.
        ///
        /// \param Frame The frame rectangle to set.
        ZYPHRYON_INLINE void SetFrame(Rect Frame)
        {
            mFrame = Frame;
        }

        /// \brief Returns the frame rectangle defining the sprite's texture region.
        ///
        /// \return The current frame rectangle.
        ZYPHRYON_INLINE Rect GetFrame() const
        {
            return mFrame;
        }

        /// \brief Resolves the deferred material using the provided service.
        ///
        /// \param Service The service used to load the material.
        ZYPHRYON_INLINE void OnResolve(Ref<Content::Service> Service)
        {
            mMaterial.Resolve(Service);
        }

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZYPHRYON_INLINE void OnSerialize(Serializer Archive)
        {
            Archive.SerializeObject(mMaterial);
            Archive.SerializeObject(mSize);
            Archive.SerializeObject(mTint);
            Archive.SerializeObject(mOrigin);
            Archive.SerializeObject(mFrame);
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Content::Proxy<Graphic::Material> mMaterial;
        Vector2                           mSize;
        IntColor8                         mTint;
        Pivot                             mOrigin;
        Rect                              mFrame;
    };
}