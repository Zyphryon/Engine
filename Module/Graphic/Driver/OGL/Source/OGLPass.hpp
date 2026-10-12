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
#include "OGLTexture.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::OGL
{
    /// \brief Represents a color attachment of a pass, what happens to it at either end.
    struct Attachment final
    {
        /// The operation performed on the attachment when the pass begins.
        Action Load  = Action::Clear;

        /// The operation performed on the attachment when the pass ends.
        Action Store = Action::Store;
    };

    /// \brief Represents an OpenGL render pass, a framebuffer plus what happens to its attachments at either end.
    class Pass final
    {
    public:

        /// \brief Constructs an empty pass that owns no framebuffer until \ref Create.
        Pass();

        /// \brief Describes the window's own framebuffer, which the display pass draws into.
        ///
        /// \param Config The configuration the window was granted, its height and depth format.
        void Create(ConstRef<Configuration> Config);

        /// \brief Creates a framebuffer over the given textures.
        ///
        /// \param Colors   The color attachments, in binding order.
        /// \param Depth    The depth attachment, whose target is `0` when the pass has none.
        /// \param Textures The driver's textures, indexed by the attachments' targets.
        /// \return `true` when the framebuffer is complete, `false` otherwise.
        Bool Create(ConstSpan<ColorAttachment> Colors, DepthAttachment Depth, ConstSpan<Texture> Textures);

        /// \brief Deletes the framebuffer and returns to the empty state.
        void Delete();

        /// \brief Binds the framebuffer, sets the clip origin, viewport and depth range, and clears what the pass loads clear.
        ///
        /// \param Device   The device of the context, whose profile decides whether encoding is switched.
        /// \param Viewport The viewport, with its origin at the top-left corner.
        /// \param Colors   The clear color of each color attachment, in binding order.
        /// \param Depth    The depth the depth attachment is cleared to.
        /// \param Stencil  The value the stencil attachment is cleared to.
        void Prepare(ConstRef<Device> Device, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil);

        /// \brief Discards every attachment whose store action asks for it.
        ///
        /// \param Device The device of the context.
        void Commit(ConstRef<Device> Device);

        /// \brief Gets the height of the window, which the display's window coordinates are flipped against.
        ///
        /// \return The height of the window in pixels, or `0` for a pass that draws into a framebuffer.
        ZY_INLINE UInt16 GetHeight() const
        {
            return mHeight;
        }

        /// \brief Checks whether the pass draws into the window, whose rows run from the bottom up.
        ///
        /// \return `true` for the display pass, which flips window coordinates, `false` for a framebuffer.
        ZY_INLINE Bool IsDisplay() const
        {
            return mHandle == 0;
        }

    private:

        /// \brief Clears every attachment whose load action asks for it, opening the scissor and write masks first.
        ///
        /// \param Colors  The clear color of each color attachment, in binding order.
        /// \param Depth   The depth the depth attachment is cleared to.
        /// \param Stencil The value the stencil attachment is cleared to.
        void Clear(ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil) const;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        GLuint                                mHandle;
        UInt16                                mHeight;
        Sequence<Attachment, kMaxAttachments> mColors;
        DepthAttachment                       mDepth;
        Bool                                  mEncode;
    };
}