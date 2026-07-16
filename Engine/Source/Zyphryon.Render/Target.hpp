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

#include "Zyphryon.Graphic/Service.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A GPU texture the \ref Renderer owns and resizes automatically.
    class Target final
    {
        friend class Renderer;

    public:

        /// \brief Describes how a target's dimensions track the frame's output size.
        enum class Scale : UInt8
        {
            Full,    ///< Full output resolution.
            Half,    ///< Half the output resolution in each axis.
            Quarter, ///< Quarter the output resolution in each axis.
            Fixed,   ///< Explicit, independent of the output size.
        };

        /// \brief Describes a renderer-managed render target.
        struct Description final
        {
            /// The pixel format of the target texture.
            Graphic::TextureFormat Format;

            /// How the target's dimensions track the frame's output size.
            Scale                  Sizing  = Scale::Full;

            /// The width, in pixels, used only when \ref Sizing is \ref Scale::Fixed.
            UInt16                 Width   = 0;

            /// The height, in pixels, used only when \ref Sizing is \ref Scale::Fixed.
            UInt16                 Height  = 0;

            /// The multisample count of the target texture.
            Graphic::Multisample   Samples = Graphic::Multisample::X1;
        };

    public:

        /// \brief Constructs a target from a description.
        ///
        /// \param Description The target's format and sizing policy.
        ZY_INLINE explicit Target(ConstRef<Description> Description)
            : mDescription { Description },
              mTexture     { 0 },
              mWidth       { 0 },
              mHeight      { 0 }
        {
        }

        /// \brief Gets the current texture handle (valid until the next resize).
        ///
        /// \return The texture object, or `0` before the first resize.
        ZY_INLINE Graphic::Object GetTexture() const
        {
            return mTexture;
        }

        /// \brief Gets the target's current width, in pixels.
        ///
        /// \return The width resolved at the last resize.
        ZY_INLINE UInt16 GetWidth() const
        {
            return mWidth;
        }

        /// \brief Gets the target's current height, in pixels.
        ///
        /// \return The height resolved at the last resize.
        ZY_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Gets the description this target was created with.
        ///
        /// \return A read-only reference to the description.
        ZY_INLINE ConstRef<Description> GetDescription() const
        {
            return mDescription;
        }

    private:

        /// \brief re-creates the texture at the size derived from the given output dimensions.
        ///
        /// \param Service The graphic service used to create the texture.
        /// \param Width   The frame's output width, in pixels.
        /// \param Height  The frame's output height, in pixels.
        void Realize(Ref<Graphic::Service> Service, UInt16 Width, UInt16 Height);

        /// \brief Destroys the texture.
        ///
        /// \param Service The graphic service used to destroy the texture.
        void Release(Ref<Graphic::Service> Service);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Description     mDescription;
        Graphic::Object mTexture;
        UInt16          mWidth;
        UInt16          mHeight;
    };
}
