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

#include "Zyphryon.Content/Service.hpp"
#include "Zyphryon.Render/Graph.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the temporal smoothing, which steadies a scene image over the ones drawn before it.
    class ZY_API PostEffectTemporal final
    {
    public:

        /// \brief Constructs the smoothing, declaring its targets and appending its passes to the blueprint.
        ///
        /// \param Host      The service host the technique is loaded through.
        /// \param Blueprint The blueprint the smoothing declares its targets and passes into.
        /// \param Scene     The slot of the scene image to steady, whose format the steadied one is kept in.
        PostEffectTemporal(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Scene);

        /// \brief Takes the smoothing's turn for the frame, its two passes blending into each other's image by turns.
        ///
        /// \return The slot whatever follows reads this frame, the scene itself while the smoothing is off.
        UInt32 Advance();

        /// \brief Sets whether the smoothing steadies the image.
        ///
        /// \param Active `true` to steady the image, `false` to hand the scene on as it lies.
        ZY_INLINE void SetActive(Bool Active)
        {
            mActive = Active;
        }

        /// \brief Sets where a point standing still was on the screen in the image before.
        ///
        /// \param Shift The move from this image back to the last, in clip space.
        /// \param Scale The size the last image stood at against this one along each axis, one when the zoom held.
        ZY_INLINE void SetShift(Vector2 Shift, Vector2 Scale)
        {
            mBlend[0]->SetShift(Shift, Scale);
            mBlend[1]->SetShift(Shift, Scale);
        }

        /// \brief Gets where the next image is to be drawn for the smoothing to blend it back.
        ///
        /// \return The offset from where the view stands, in pixels, or zero while the smoothing is off.
        Vector2 GetJitter() const;

    private:

        /// \brief The share of each new image taken into the blend while the view stands still.
        static constexpr Real32 kStill     = 0.1f;

        /// \brief The share of each new image taken into the blend once the view moves a pixel or more a frame.
        static constexpr Real32 kMoving    = 0.25f;

        /// \brief The technique the smoothing draws with.
        static constexpr Text   kTechnique = "Embedded://Technique/PostEffect/Temporal.vfx";

        /// \brief Represents the per-pass data the temporal technique consumes.
        struct GpuTemporalLayout final
        {
            /// The clip-space move back to the image before in `XY`, and the share of the new image in `Z`.
            Vector4 Shift;

            /// The texel of the image read across and up in `XY`, and the size of the image before in `ZW`.
            Vector4 Texel;
        };

        /// \brief Represents the blend of the scene into the image steadied before it, drawn as a pass of its own.
        class Step final : public Pass
        {
        public:

            /// \brief Constructs the blend.
            ///
            /// \param Host   The service host the technique is loaded through.
            /// \param Source The slot of the image the pass reads, whose size sets the step between taps.
            Step(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source);

            /// \brief Sets where a point standing still was on the screen in the image before.
            ///
            /// \param Shift The move from this image back to the last, in clip space.
            /// \param Scale The size the last image stood at against this one along each axis.
            ZY_INLINE void SetShift(Vector2 Shift, Vector2 Scale)
            {
                mShift = Shift;
                mScale = Scale;
            }

            /// \copydoc Pass::Run
            void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Retainer<ZyGraphic::Technique> mTechnique;
            UInt32                         mSource;
            Vector2                        mShift;
            Vector2                        mScale;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<Ptr<Step>, 2> mBlend;
        Array<UInt32, 2>    mSteady;
        UInt32              mScene;
        UInt32              mRound;
        Bool                mActive;
    };
}