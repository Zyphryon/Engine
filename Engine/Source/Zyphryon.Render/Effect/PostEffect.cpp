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

#include "PostEffect.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffect::PostEffect(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Scene, UInt32 Output)
        : mTemporal { Host, Blueprint, Scene },
          mBloom    { Host, Blueprint, Scene },
          mExposure { Host, Blueprint, Scene },
          mFinal    { AddressOf(Blueprint.AddPass<PostEffectFinal>("Final", Host, Scene)) }
    {
        mFinal->AddColor({ .Target = Output, .Load = ZyGraphic::Action::Discard });
        mFinal->AddInput({ .Name = "Scene"_Hash,    .Target = Scene                 });
        mFinal->AddInput({ .Name = "Bloom"_Hash,    .Target = mBloom.GetOutput()    });
        mFinal->AddInput({ .Name = "Exposure"_Hash, .Target = mExposure.GetOutput() });

        // The bloom does not run, nor the exposure measure, nor the smoothing, until something asks for it.
        SetAntialias(Antialias::None);
        SetBloom(PostEffect::Bloom());
        SetExposure(PostEffect::Exposure());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::Advance(Real64 Time)
    {
        mFinal->SetTime(Time);
        mExposure.SetTime(Time);

        // Whatever reads the scene after the smoothing reads the image it steadied this frame, or the scene.
        const UInt32 Image = mTemporal.Advance();

        mBloom.SetInput(Image);
        mExposure.SetInput(Image);
        mFinal->SetInput(0, Image);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetBloom(ConstRef<Bloom> Bloom)
    {
        const Bool Glow = (Bloom.Strength > 0.0f);

        mBloom.SetThreshold(Bloom.Threshold);
        mBloom.SetActive(Glow);

        // The halo sums every level on the way up, so it is laid back as their mean.
        mFinal->SetHalo(Glow ? Bloom.Strength / PostEffectBloom::kLevels : 0.0f);
    }
}