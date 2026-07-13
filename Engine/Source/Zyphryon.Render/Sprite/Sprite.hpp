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

#include "Zyphryon.Graphic/Material.hpp"
#include "Zyphryon.Math/Color.hpp"
#include "Zyphryon.Math/Geometry/Rect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief Represents rendering properties of a sprite.
    class Sprite final
    {
    public:

        /// \brief Default constructor initializing default sprite properties.
        ZY_INLINE Sprite() = default;

        /// \brief Constructs a sprite with specified properties.
        ///
        /// \param Material The material used to render the sprite.
        /// \param Size     The size of the sprite in pixels.
        /// \param Tint     The tint color applied to the sprite.
        /// \param Frame    The frame rectangle defining the sprite's texture region.
        ZY_INLINE Sprite(ConstRetainer<Graphic::Material> Material, Vector2 Size, IntColor8 Tint, Rect Frame)
            : mMaterial { Material },
              mSize     { Size },
              mTint     { Tint },
              mFrame    { Frame }
        {
        }

        /// \brief Sets the material used to render the sprite.
        ///
        /// \param Material The material to set.
        ZY_INLINE void SetMaterial(ConstRetainer<Graphic::Material> Material)
        {
            mMaterial = Material;
        }

        /// \brief Gets the material used to render the sprite.
        ///
        /// \return The current material.
        ZY_INLINE ConstRetainer<Graphic::Material> GetMaterial() const
        {
            return mMaterial;
        }

        /// \brief Sets the size of the sprite.
        ///
        /// \param Size The size to set.
        ZY_INLINE void SetSize(Vector2 Size)
        {
            mSize = Size;
        }

        /// \brief Gets the size of the sprite.
        ///
        /// \return The current size.
        ZY_INLINE Vector2 GetSize() const
        {
            return mSize;
        }

        /// \brief Sets the tint color applied to the sprite.
        ///
        /// \param Tint The tint color to set.
        ZY_INLINE void SetTint(IntColor8 Tint)
        {
            mTint = Tint;
        }

        /// \brief Gets the tint color applied to the sprite.
        ///
        /// \return The current tint color.
        ZY_INLINE IntColor8 GetTint() const
        {
            return mTint;
        }

        /// \brief Sets the frame rectangle defining the sprite's texture region.
        ///
        /// \param Frame The frame rectangle to set.
        ZY_INLINE void SetFrame(Rect Frame)
        {
            mFrame = Frame;
        }

        /// \brief Gets the frame rectangle defining the sprite's texture region.
        ///
        /// \return The current frame rectangle.
        ZY_INLINE Rect GetFrame() const
        {
            return mFrame;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<Graphic::Material> mMaterial;
        Vector2                     mSize;
        IntColor8                   mTint;
        Rect                        mFrame;
    };
}