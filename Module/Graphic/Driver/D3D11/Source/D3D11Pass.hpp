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

#include "D3D11Swapchain.hpp"
#include "D3D11Texture.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyGraphic::D3D11
{
    /// \brief Represents a color attachment of a pass, the view it renders into and what happens to it at either end.
    struct Attachment final
    {
        /// The view the pass renders into.
        ComPtr<ID3D11RenderTargetView> View;

        /// The operation performed on the attachment when the pass begins.
        Action                         Load  = Action::Clear;

        /// The operation performed on the attachment when the pass ends.
        Action                         Store = Action::Store;
    };

    /// \brief Represents a Direct3D 11 render pass, the views it renders into and what happens to them at either end.
    class Pass final
    {
    public:

        /// \brief Constructs an empty pass that renders into nothing until \ref Create.
        Pass();

        /// \brief Creates the display pass over the swapchain's back buffer, with a depth buffer of its own if asked for.
        ///
        /// \param Device    The device the views are created on.
        /// \param Swapchain The swapchain whose back buffer and configuration the pass renders with.
        /// \return `true` if every view was created, `false` otherwise.
        Bool Create(ConstRef<Device> Device, ConstRef<Swapchain> Swapchain);

        /// \brief Creates an offscreen pass over the given textures.
        ///
        /// \param Device   The device the views are created on.
        /// \param Colors   The color attachments, in binding order.
        /// \param Depth    The depth attachment, whose target is `0` when the pass has none.
        /// \param Textures The texture table the attachments' targets index.
        /// \return `true` if every view was created, `false` otherwise.
        Bool Create(ConstRef<Device> Device, ConstSpan<ColorAttachment> Colors, DepthAttachment Depth, ConstSpan<Texture> Textures);

        /// \brief Releases the views and returns to the empty state.
        void Delete();

        /// \brief Binds the pass's views, clears what the pass loads by clearing and sets the viewport.
        ///
        /// \param Device   The device whose context the pass begins on.
        /// \param Viewport The viewport the pass renders through.
        /// \param Colors   The clear values of the color attachments, in binding order.
        /// \param Depth    The clear value of the depth plane.
        /// \param Stencil  The clear value of the stencil plane.
        void Prepare(ConstRef<Device> Device, ConstRef<Viewport> Viewport, ConstSpan<Color> Colors, Real32 Depth, UInt8 Stencil);

        /// \brief Discards what the pass does not keep.
        ///
        /// \param Device The device whose context the pass ends on.
        void Commit(ConstRef<Device> Device);

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Attachment, kMaxAttachments> mColors;
        ComPtr<ID3D11DepthStencilView>        mView;
        DepthAttachment                       mDepth;
        Bool                                  mStencil;
    };
}