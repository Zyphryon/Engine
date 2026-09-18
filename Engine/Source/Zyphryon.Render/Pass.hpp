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

namespace ZyRender
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
            UInt32            Target  = kNone;

            /// The slot of the target the multisampled result is resolved into, or \ref kNone when not multisampled.
            UInt32            Resolve = kNone;

            /// The operation applied when the pass opens.
            ZyGraphic::Action Load    = ZyGraphic::Action::Clear;

            /// The operation applied when the pass closes.
            ZyGraphic::Action Store   = ZyGraphic::Action::Store;

            /// The color this attachment is cleared to (used when \ref Load is \ref ZyGraphic::Action::Clear).
            Color             Tint    = Color(0.0f, 0.0f, 0.0f, 1.0f);
        };

        /// \brief A declared depth/stencil attachment.
        struct DepthAttachment final
        {
            /// The slot of the depth/stencil target, or \ref kNone when the pass has no depth attachment.
            UInt32            Target       = kNone;

            /// The operation applied to the depth buffer when the pass opens.
            ZyGraphic::Action DepthLoad    = ZyGraphic::Action::Clear;

            /// The operation applied to the depth buffer when the pass closes.
            ZyGraphic::Action DepthStore   = ZyGraphic::Action::Store;

            /// The operation applied to the stencil buffer when the pass opens.
            ZyGraphic::Action StencilLoad  = ZyGraphic::Action::Clear;

            /// The operation applied to the stencil buffer when the pass closes.
            ZyGraphic::Action StencilStore = ZyGraphic::Action::Store;

            /// The value the depth buffer is cleared to (used when \ref DepthLoad is \ref ZyGraphic::Action::Clear).
            Real32            Depth        = 1.0f;

            /// The value the stencil buffer is cleared to (used when \ref StencilLoad is \ref ZyGraphic::Action::Clear).
            UInt8             Stencil      = 0;
        };

    public:

        /// \brief Constructs a pass that is active and draws into the display surface.
        ZY_INLINE Pass()
            : mActive { true }
        {
        }

        /// \brief Destroys the pass.
        virtual ~Pass() = default;

        /// \brief Sets the name the pass is known by.
        ///
        /// \param Name The name a profiler and a frame capture show the pass under, which nothing drawn reads.
        ZY_INLINE void SetName(Text Name)
        {
            mName = Name;
        }

        /// \brief Gets the name the pass is known by.
        ///
        /// \return The name, which is empty until one is set.
        ZY_INLINE Text GetName() const
        {
            return mName;
        }

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

        /// \brief Executes the pass, recording its draw commands through the encoder.
        ///
        /// \param Encoder The encoder used to build this pass's draw commands.
        /// \param Graph   The graph being drawn, which the pass reads every target it samples from.
        virtual void Run(Ref<Encoder> Encoder, ConstRef<class Graph> Graph) = 0;

    protected:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Str                                                   mName;
        Bool                                                  mActive;
        Sequence<ColorAttachment, ZyGraphic::kMaxAttachments> mColorAttachment;
        DepthAttachment                                       mDepthAttachment;
    };
}