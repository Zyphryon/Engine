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
#include "Target.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Render
{
    /// \brief A render pass that executes a sequence of draw commands through an encoder.
    class Pass
    {
    public:

        /// \brief A declared color attachment.
        struct ColorAttachment final
        {
            /// The managed target written by this attachment, or `nullptr` for the display surface.
            Ptr<Render::Target> Target  = nullptr;

            /// The managed target the multisampled result is resolved into, or `nullptr` when not multisampled.
            Ptr<Render::Target> Resolve = nullptr;

            /// The operation applied when the pass opens.
            Graphic::Action     Load    = Graphic::Action::Clear;

            /// The operation applied when the pass closes.
            Graphic::Action     Store   = Graphic::Action::Store;

            /// The color this attachment is cleared to (used when \ref Load is \ref Graphic::Action::Clear).
            Color               Tint    = Color(0.0f, 0.0f, 0.0f, 1.0f);
        };

        /// \brief A declared depth/stencil attachment.
        struct DepthAttachment final
        {
            /// The managed depth/stencil target, or `nullptr` when the pass has no depth attachment.
            Ptr<Target>     Target       = nullptr;

            /// The operation applied to the depth buffer when the pass opens.
            Graphic::Action DepthLoad    = Graphic::Action::Clear;

            /// The operation applied to the depth buffer when the pass closes.
            Graphic::Action DepthStore   = Graphic::Action::Discard;

            /// The operation applied to the stencil buffer when the pass opens.
            Graphic::Action StencilLoad  = Graphic::Action::Clear;

            /// The operation applied to the stencil buffer when the pass closes.
            Graphic::Action StencilStore = Graphic::Action::Discard;

            /// The value the depth buffer is cleared to (used when \ref DepthLoad is \ref Graphic::Action::Clear).
            Real32          Depth        = 1.0f;

            /// The value the stencil buffer is cleared to (used when \ref StencilLoad is \ref Graphic::Action::Clear).
            UInt8           Stencil      = 0;
        };

    public:

        /// \brief Constructs a pass with the display surface as its target.
        ZY_INLINE Pass()
            : mActive { true },
              mTarget { Graphic::kDisplay }
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

        /// \brief Sets the graphic target this pass renders into.
        ///
        /// \param Target The target handle (the display surface by default).
        ZY_INLINE void SetTarget(Graphic::Object Target)
        {
            mTarget = Target;
        }

        /// \brief Gets the graphic target this pass renders into.
        ///
        /// \return The target handle.
        ZY_INLINE Graphic::Object GetTarget() const
        {
            return mTarget;
        }

        /// \brief Sets the viewport, in target pixels, the pass renders with.
        ///
        /// \param Viewport The viewport rectangle and depth range.
        ZY_INLINE void SetViewport(ConstRef<Graphic::Viewport> Viewport)
        {
            mViewport = Viewport;
        }

        /// \brief Gets the viewport the pass renders with.
        ///
        /// \return The viewport rectangle and depth range.
        ZY_INLINE ConstRef<Graphic::Viewport> GetViewport() const
        {
            return mViewport;
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

        /// \brief Executes the pass, recording its draw commands through the encoder.
        ///
        /// \param Encoder The encoder used to build this pass's draw commands.
        virtual void Run(Ref<Encoder> Encoder) = 0;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Bool                                                mActive;
        Graphic::Object                                     mTarget;
        Graphic::Viewport                                   mViewport;
        Sequence<ColorAttachment, Graphic::kMaxAttachments> mColorAttachment;
        DepthAttachment                                     mDepthAttachment;
    };
}
