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

#include "OcclusionHorizon.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the settle of the occlusion, which weighs the search's noise away over texels at its depth.
    class ZY_API OcclusionSettle final : public Pass
    {
    public:

        /// \brief Constructs the settle of the occlusion.
        ///
        /// \param Host The service host the technique is loaded through.
        explicit OcclusionSettle(Ref<ZyEngine::Subsystem::Host> Host);

        /// \brief Sets how much the sky is held back, and how far a surface looks for what closes it in.
        ///
        /// \param Shading The shading to set.
        ZY_INLINE void SetShading(OcclusionHorizon::Shading Shading)
        {
            mShading = Shading;
        }

        /// \copydoc Pass::Run
        void Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph) override;

    private:

        /// \brief The technique the settle draws with.
        static constexpr Text   kTechnique = "Embedded://Technique/Occlusion/Settle.vfx";

        /// \brief The depth step the settle keeps across, as a share of the radius.
        static constexpr Real32 kEdge      = 0.25f;

        /// \brief Represents the per-pass data the settle technique consumes.
        struct GpuSettleLayout final
        {
            /// The strength, the depth step kept across in world units, and two spares.
            Vector4 Settle;
        };

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Retainer<ZyGraphic::Technique> mTechnique;
        OcclusionHorizon::Shading      mShading;
    };
}