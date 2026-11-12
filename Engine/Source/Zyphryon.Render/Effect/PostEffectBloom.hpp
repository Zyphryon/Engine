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
    /// \brief Represents the bloom, which carries what an image holds above white down a chain of halves and back up.
    class ZY_API PostEffectBloom final
    {
    public:

        /// \brief The levels the image is halved through, each spreading the glow twice as far as the one above.
        static constexpr UInt32 kLevels = 4;

    public:

        /// \brief Constructs the bloom, declaring its targets and appending its passes to the blueprint.
        ///
        /// \param Host      The service host the technique is loaded through.
        /// \param Blueprint The blueprint the bloom declares its targets and passes into.
        /// \param Image     The slot of the image the bloom reads, whose format the glow is gathered in.
        PostEffectBloom(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Image);

        /// \brief Sets whether the bloom gathers the glow.
        ///
        /// \param Active `true` to gather the glow, `false` to leave the output as it was.
        void SetActive(Bool Active);

        /// \brief Sets the image the bloom reads.
        ///
        /// \param Image The slot of the image to read, of the size of the one the bloom was made over.
        ZY_INLINE void SetInput(UInt32 Image)
        {
            mSteps[0]->SetInput(0, Image);
        }

        /// \brief Sets the fraction of the output the first level is gathered at.
        ///
        /// \param Sizing The fraction to gather it at, which every level below halves again.
        void SetScale(Target::Scale Sizing);

        /// \brief Sets the brightness a pixel has to pass before it spills at all.
        ///
        /// \param Threshold The brightness to pass, in scene units where one is white.
        ZY_INLINE void SetThreshold(Real32 Threshold)
        {
            mSteps[0]->SetThreshold(Threshold);
        }

        /// \brief Gets the slot the glow is written into.
        ///
        /// \return The slot, holding the sum of every level at the size of the first.
        ZY_INLINE UInt32 GetOutput() const
        {
            return mUp[0];
        }

    private:

        /// \brief The width of the knee under the threshold, over which a pixel eases into blooming.
        static constexpr Real32 kKnee      = 0.5f;

        /// \brief The technique every step of the bloom draws with.
        static constexpr Text   kTechnique = "Embedded://Technique/PostEffect/Bloom.vfx";

        /// \brief Represents the per-pass data the bloom technique consumes.
        struct GpuBloomLayout final
        {
            /// The threshold and the knee, then a texel of the level read across in `Z` and up in `W`.
            Vector4 Filter;
        };

        /// \brief Enumerates the step of the bloom a pass takes.
        enum class Sweep : UInt8
        {
            Sift, ///< Keeps only what clears the threshold, as it halves the image into the first level.
            Down, ///< Halves the level above into the next one down, filtered so a small bright point never flickers.
            Up,   ///< Spreads the level below over the next one up, and adds what that level held on the way down.
        };

        /// \brief Represents one step of the bloom, drawn as a pass of its own.
        class Step final : public Pass
        {
        public:

            /// \brief Constructs a step of the bloom.
            ///
            /// \param Host   The service host the technique is loaded through.
            /// \param Source The slot of the level the step reads, whose size sets the step between taps.
            /// \param Axis   The step this instance takes.
            Step(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source, Sweep Axis);

            /// \brief Sets the brightness a pixel has to pass before it spills at all.
            ///
            /// \param Threshold The brightness to pass, in scene units where one is white, read by the sift alone.
            ZY_INLINE void SetThreshold(Real32 Threshold)
            {
                mThreshold = Threshold;
            }

            /// \copydoc Pass::Run
            void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

        private:

            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
            // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

            Retainer<ZyGraphic::Technique> mTechnique;
            UInt32                         mSource;
            Sweep                          mSweep;
            Real32                         mThreshold;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Blueprint>             mBlueprint;
        Array<UInt32, kLevels>     mDown;
        Array<UInt32, kLevels - 1> mUp;
        Sequence<Ptr<Step>>        mSteps;
    };
}