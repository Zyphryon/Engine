// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Occlusion.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Occlusion::Occlusion(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Reach, UInt32 Normal)
        : mDetail   { Detail::Standard },
          mStrength { Shading().Strength }
    {
        constexpr Target Half(ZyGraphic::TextureFormat::RGBA8UIntNorm, Target::Scale::Half);

        const UInt32 Horizon = Blueprint.AddTarget(Half);
        mOutput = Blueprint.AddTarget(Half);

        // The search finds each texel's horizons, and the settle weighs the noise away over texels at its depth.
        mHorizon = AddressOf(Blueprint.AddPass<OcclusionHorizon>("Horizon", Host));
        mHorizon->AddColor({ .Target = Horizon, .Load = ZyGraphic::Action::Discard });
        mHorizon->AddInput({ .Name = "Reach"_Hash,  .Target = Reach  });
        mHorizon->AddInput({ .Name = "Normal"_Hash, .Target = Normal });

        mSettle = AddressOf(Blueprint.AddPass<OcclusionSettle>("Occlusion", Host));
        mSettle->AddColor({ .Target = mOutput, .Load = ZyGraphic::Action::Discard });
        mSettle->AddInput({ .Name = "Reach"_Hash,     .Target = Reach   });
        mSettle->AddInput({ .Name = "Occlusion"_Hash, .Target = Horizon });

        Apply();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Occlusion::SetDetail(Detail Detail)
    {
        mDetail = Detail;
        mHorizon->SetDetail(Detail);

        Apply();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Occlusion::SetShading(ConstRef<Shading> Shading)
    {
        mStrength = Shading.Strength;
        mHorizon->SetRadius(Shading.Radius);
        mSettle->SetShading(Shading);

        Apply();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Occlusion::Apply()
    {
        const Bool Searched = (mDetail != Detail::Off && mStrength > 0.0f);

        mHorizon->SetActive(Searched);
        mSettle->SetActive(Searched);
    }
}