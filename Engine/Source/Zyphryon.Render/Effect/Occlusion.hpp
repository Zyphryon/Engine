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

#include "OcclusionSettle.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the occlusion chain, which finds how much of the sky each surface sees past what closes it in.
    class ZY_API Occlusion final
    {
    public:

        /// \brief Names how finely the horizons are searched.
        using Detail  = OcclusionHorizon::Detail;

        /// \brief Names how much the sky is held back where surfaces close in on each other.
        using Shading = OcclusionHorizon::Shading;

    public:

        /// \brief Constructs the chain, declaring its targets and appending its passes to the blueprint.
        ///
        /// \param Host      The service host the technique is loaded through.
        /// \param Blueprint The blueprint the chain declares its targets and passes into.
        /// \param Reach     The slot of the half-size depth past the near plane per texel, 1e6 or more for none.
        /// \param Normal    The slot of the full-size normals, encoded as a normal map is.
        Occlusion(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Reach, UInt32 Normal);

        /// \brief Sets how finely the horizons are searched.
        ///
        /// \param Detail The detail to search at, or \ref Detail::Off to search nothing.
        void SetDetail(Detail Detail);

        /// \brief Sets how much the sky is held back, and how far a surface looks for what closes it in.
        ///
        /// \param Shading The shading to set, whose zero strength searches nothing.
        void SetShading(ConstRef<Shading> Shading);

        /// \brief Sets the view the reach was laid along, orthographic or perspective as its matrix tells.
        ///
        /// \param Inverse The matrix that carries clip space back into the space the surfaces are searched in.
        ZY_INLINE void SetView(ConstRef<Matrix4x4> Inverse)
        {
            mHorizon->SetView(Inverse);
        }

        /// \brief Gets the slot the settled occlusion is written into.
        ///
        /// \return The slot, holding the way the open sky lies in `RGB` and its share in alpha, at half size.
        ZY_INLINE UInt32 GetOutput() const
        {
            return mOutput;
        }

        /// \brief Checks whether the chain searches anything this frame.
        ///
        /// \return `true` when both a detail and some strength were set, `false` when the output is left as it was.
        ZY_INLINE Bool IsActive() const
        {
            return mHorizon->IsActive();
        }

    private:

        /// \brief Switches both passes on or off by the detail and the strength set.
        void Apply();

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Ptr<OcclusionHorizon> mHorizon;
        Ptr<OcclusionSettle>  mSettle;
        UInt32                mOutput;
        Detail                mDetail;
        Real32                mStrength;
    };
}