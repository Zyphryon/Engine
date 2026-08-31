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

#include "Encoder.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A render pass that executes a sequence of draw commands through an encoder.
    class Pass
    {
    public:

        /// \brief Names no managed target, which on the first color attachment means the display surface.
        static constexpr UInt32 kNone = 0xFFFFFFFF;

        /// \brief A declared color attachment.
        struct ColorAttachment final
        {
            /// The slot of the target written by this attachment, or \ref kNone for the display surface.
            UInt32          Target  = kNone;

            /// The slot of the target the multisampled result is resolved into, or \ref kNone when not multisampled.
            UInt32          Resolve = kNone;

            /// The operation applied when the pass opens.
            Graphic::Action Load    = Graphic::Action::Clear;

            /// The operation applied when the pass closes.
            Graphic::Action Store   = Graphic::Action::Store;

            /// The color this attachment is cleared to (used when \ref Load is \ref Graphic::Action::Clear).
            Color           Tint    = Color(0.0f, 0.0f, 0.0f, 1.0f);
        };

        /// \brief A declared depth/stencil attachment.
        struct DepthAttachment final
        {
            /// The slot of the depth/stencil target, or \ref kNone when the pass has no depth attachment.
            UInt32          Target       = kNone;

            /// The operation applied to the depth buffer when the pass opens.
            Graphic::Action DepthLoad    = Graphic::Action::Clear;

            /// The operation applied to the depth buffer when the pass closes.
            Graphic::Action DepthStore   = Graphic::Action::Store;

            /// The operation applied to the stencil buffer when the pass opens.
            Graphic::Action StencilLoad  = Graphic::Action::Clear;

            /// The operation applied to the stencil buffer when the pass closes.
            Graphic::Action StencilStore = Graphic::Action::Store;

            /// The value the depth buffer is cleared to (used when \ref DepthLoad is \ref Graphic::Action::Clear).
            Real32          Depth        = 1.0f;

            /// The value the stencil buffer is cleared to (used when \ref StencilLoad is \ref Graphic::Action::Clear).
            UInt8           Stencil      = 0;
        };

    public:

        /// \brief Constructs a pass that is active and draws into the display surface.
        ZY_INLINE Pass()
            : mActive { true },
              mInline { false }
        {
        }

        /// \brief Destroys the pass.
        virtual ~Pass() = default;

        /// \brief Sets the active state of the pass.
        ///
        /// \param Active `true` to enable the pass, `false` to skip it during execution.
        ZY_INLINE void SetActive(Bool Active)
        {
            mActive = Active;
        }

        /// \brief Gets the active state of the pass.
        ///
        /// \return `true` if the pass is active, otherwise `false`.
        ZY_INLINE Bool IsActive() const
        {
            return mActive;
        }

        /// \brief Declares a color attachment and appends it to the pass.
        ///
        /// \param Color The color attachment to append.
        ZY_INLINE void AddColor(ConstRef<ColorAttachment> Color)
        {
            mColorAttachment.Append(Color);
        }

        /// \brief Gets a color attachment.
        ///
        /// \param Index The color attachment index, in declaration order.
        /// \return A read-only reference to the color attachment.
        ZY_INLINE ConstRef<ColorAttachment> GetColor(UInt32 Index = 0)
        {
            return mColorAttachment[Index];
        }

        /// \brief Gets the pass's color attachments.
        ///
        /// \return A read-only reference to the color attachment list.
        ZY_INLINE ConstSpan<ColorAttachment> GetColors() const
        {
            return mColorAttachment;
        }

        /// \brief Sets the depth attachment.
        ///
        /// \param Depth The depth attachment to use.
        ZY_INLINE void SetDepth(ConstRef<DepthAttachment> Depth)
        {
            mDepthAttachment = Depth;
        }

        /// \brief Gets the depth attachment.
        ///
        /// \return A read-only reference to the depth attachment.
        ZY_INLINE ConstRef<DepthAttachment> GetDepth() const
        {
            return mDepthAttachment;
        }

        /// \brief Sets whether the pass appends its draws to the target the pass before it opened.
        ///
        /// \note An inline pass declares no attachments of its own, inheriting the group's clears and viewport.
        ///
        /// \param Inline `true` to draw into the target already open, `false` to open one of its own.
        ZY_INLINE void SetInline(Bool Inline)
        {
            mInline = Inline;
        }

        /// \brief Checks whether the pass appends its draws to the target the pass before it opened.
        ///
        /// \return `true` if the pass draws inline, otherwise `false`.
        ZY_INLINE Bool IsInline() const
        {
            return mInline;
        }

        /// \brief Executes the pass, recording its draw commands through the encoder.
        ///
        /// \param Encoder The encoder used to build this pass's draw commands.
        /// \param Graph   The graph being drawn, which the pass reads every target it samples from.
        virtual void Run(Ref<Encoder> Encoder, ConstRef<class Graph> Graph) = 0;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool                                                mActive;
        Bool                                                mInline;
        Sequence<ColorAttachment, Graphic::kMaxAttachments> mColorAttachment;
        DepthAttachment                                     mDepthAttachment;
    };
}