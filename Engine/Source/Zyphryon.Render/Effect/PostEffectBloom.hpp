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
    /// \brief Represents one step of the bloom, which carries what the image holds above white down a chain of halves.
    class ZY_API PostEffectBloom final : public Pass
    {
    public:

        /// \brief Enumerates the step of the bloom an instance of the pass takes.
        enum class Sweep : UInt8
        {
            Sift, ///< Keeps only what clears the threshold, as it halves the image into the first level.
            Down, ///< Halves the level above into the next one down, filtered so a small bright point never flickers.
            Up,   ///< Spreads the level below over the next one up, and adds what that level held on the way down.
        };

    public:

        /// \brief Constructs a step of the bloom.
        ///
        /// \param Host   The service host the technique is loaded through.
        /// \param Source The slot of the level the step reads, whose size sets the step between taps.
        /// \param Axis   The step this instance takes.
        PostEffectBloom(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source, Sweep Axis);

        /// \brief Sets the brightness a pixel has to pass before it spills at all.
        ///
        /// \param Threshold The brightness to pass, in scene units where one is white, read by the sift alone.
        ZY_INLINE void SetThreshold(Real32 Threshold)
        {
            mThreshold = Threshold;
        }

        /// \brief Gets the brightness a pixel has to pass before it spills at all.
        ///
        /// \return The brightness to pass, in scene units where one is white.
        ZY_INLINE Real32 GetThreshold() const
        {
            return mThreshold;
        }

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

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

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        UInt32                         mSource;
        Sweep                          mSweep;
        Real32                         mThreshold;
    };
}