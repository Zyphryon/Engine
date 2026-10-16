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

#include "PostEffectBloom.hpp"
#include "PostEffectFinal.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the post effect chain, which carries a scene image through the bloom onto the output.
    class ZY_API PostEffect final
    {
    public:

        /// \brief Constructs the chain, declaring its targets and appending its passes to the blueprint.
        ///
        /// \param Host      The service host the techniques are loaded through.
        /// \param Blueprint The blueprint the chain declares its targets and passes into.
        /// \param Scene     The slot of the scene image the chain reads, whose format the bloom is gathered in.
        /// \param Output    The slot the final image is written into, or \ref Pass::kNone for the display surface.
        PostEffect(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Scene, UInt32 Output);

        /// \brief Stirs the grain by the time given.
        ///
        /// \param Time The time of the frame, in seconds.
        void Advance(Real64 Time);

        /// \brief Sets the glow the brightest of the image is allowed to spill.
        ///
        /// \param Strength  The share of the glow laid back over the image, or zero to gather none at all.
        /// \param Threshold The brightness a pixel has to pass before it spills, in scene units where one is white.
        void SetBloom(Real32 Strength, Real32 Threshold);

        /// \brief Sets the fraction of the output the first level of the bloom is gathered at.
        ///
        /// \param Sizing The fraction to gather it at, which every level below halves again.
        void SetBloomScale(Target::Scale Sizing);

        /// \brief Gets the final pass, which the exposure, the grade, the lens and the antialiasing are set on.
        ///
        /// \return The final pass of the chain.
        ZY_INLINE Ref<PostEffectFinal> GetFinal()
        {
            return * mFinal;
        }

    private:

        /// \brief The levels the bloom halves the image through, each spreading the glow twice as wide as the one above.
        static constexpr UInt32 kLevels  = 4;

        /// \brief The frames a second the grain is stirred at, which is a film's cadence.
        static constexpr Real64 kCadence = 24.0;

        /// \brief The frames the grain counts before it wraps, the most a float carries whole.
        static constexpr Real64 kFrames  = 16777216.0;

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ref<Blueprint>                 mBlueprint;
        Array<UInt32, kLevels>         mDown;
        Array<UInt32, kLevels - 1>     mUp;
        Sequence<Ptr<PostEffectBloom>> mBloom;
        Ptr<PostEffectFinal>           mFinal;
    };
}