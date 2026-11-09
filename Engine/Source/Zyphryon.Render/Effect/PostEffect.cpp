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
        : mBlueprint { Blueprint }
    {
        const ZyGraphic::TextureFormat Format = Blueprint.GetTargets()[Scene].Format;

        // Every level on the way down has one beside it on the way up, but for the last, which turns the chain round.
        const Target Bloom(Format);

        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            mDown[Level] = Blueprint.AddTarget(Bloom);
        }

        for (UInt32 Level = 0; Level < kLevels - 1; ++Level)
        {
            mUp[Level] = Blueprint.AddTarget(Bloom);
        }

        // The sift keeps what clears the threshold as it halves the image, and every step after halves it again.
        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            const UInt32                 Source = (Level == 0 ? Scene : mDown[Level - 1]);
            const PostEffectBloom::Sweep Axis   = (Level == 0
                ? PostEffectBloom::Sweep::Sift
                : PostEffectBloom::Sweep::Down);

            Ref<PostEffectBloom> Down = Blueprint.AddPass<PostEffectBloom>(
                Level == 0 ? "Sift" : "Down", Host, Source, Axis);
            Down.AddColor({ .Target = mDown[Level], .Load = ZyGraphic::Action::Discard });
            Down.AddInput({ .Name = "Scene"_Hash, .Target = Source });
            mBloom.Append(AddressOf(Down));
        }

        // The way back up spreads the level below over each one, adding what that level held on the way down.
        for (UInt32 Level = kLevels - 1; Level-- > 0;)
        {
            const UInt32 Source = (Level + 2 < kLevels ? mUp[Level + 1] : mDown[Level + 1]);

            Ref<PostEffectBloom> Up = Blueprint.AddPass<PostEffectBloom>("Up", Host, Source, PostEffectBloom::Sweep::Up);
            Up.AddColor({ .Target = mUp[Level], .Load = ZyGraphic::Action::Discard });
            Up.AddInput({ .Name = "Scene"_Hash, .Target = Source       });
            Up.AddInput({ .Name = "Level"_Hash, .Target = mDown[Level] });
            mBloom.Append(AddressOf(Up));
        }

        // The meter reads the scene into a small grid of its own, which the adaptation averages whole.
        const UInt32 Meter = Blueprint.AddTarget({
            .Format = ZyGraphic::TextureFormat::RG32Float,
            .Sizing = Target::Scale::Fixed,
            .Width  = PostEffectExposure::kMeter,
            .Height = PostEffectExposure::kMeter });

        Ref<PostEffectExposure> Measure = Blueprint.AddPass<PostEffectExposure>(
            "Measure", Host, PostEffectExposure::Sweep::Measure);
        Measure.AddColor({ .Target = Meter, .Load = ZyGraphic::Action::Discard });
        Measure.AddInput({ .Name = "Scene"_Hash, .Target = Scene });
        mMeter.Append(AddressOf(Measure));

        // The exposure eased this frame is kept aside, since a pass never reads the target it writes.
        constexpr Target Single(ZyGraphic::TextureFormat::R32Float, Target::Scale::Fixed, 1, 1);

        const UInt32 Eased = Blueprint.AddTarget(Single);
        const UInt32 Kept  = Blueprint.AddTarget(Single);

        mAdapt = AddressOf(Blueprint.AddPass<PostEffectExposure>(
            "Adapt", Host, PostEffectExposure::Sweep::Adapt));
        mAdapt->AddColor({ .Target = Eased, .Load = ZyGraphic::Action::Discard });
        mAdapt->AddInput({ .Name = "Scene"_Hash, .Target = Meter });
        mAdapt->AddInput({ .Name = "Level"_Hash, .Target = Kept  });

        Ref<PostEffectExposure> Keep = Blueprint.AddPass<PostEffectExposure>(
            "Keep", Host, PostEffectExposure::Sweep::Keep);
        Keep.AddColor({ .Target = Kept, .Load = ZyGraphic::Action::Discard });
        Keep.AddInput({ .Name = "Scene"_Hash, .Target = Eased });
        mMeter.Append(AddressOf(Keep));

        mFinal = AddressOf(Blueprint.AddPass<PostEffectFinal>("Final", Host, Scene));
        mFinal->AddColor({ .Target = Output, .Load = ZyGraphic::Action::Discard });
        mFinal->AddInput({ .Name = "Scene"_Hash,    .Target = Scene  });
        mFinal->AddInput({ .Name = "Bloom"_Hash,    .Target = mUp[0] });
        mFinal->AddInput({ .Name = "Exposure"_Hash, .Target = Eased  });

        // The bloom does not run, nor the exposure measure, until something asks for it.
        SetBloom(PostEffect::Bloom());
        SetBloomScale(Target::Scale::Quarter);
        SetExposure(PostEffect::Exposure());
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::Advance(Real64 Time)
    {
        mFinal->SetTime(Time);
        mAdapt->SetTime(Time);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetBloom(ConstRef<Bloom> Bloom)
    {
        const Bool Glow = (Bloom.Strength > 0.0f);

        mBloom[0]->SetThreshold(Bloom.Threshold);

        for (const Ptr<PostEffectBloom> Step : mBloom)
        {
            Step->SetActive(Glow);
        }

        // The halo sums every level on the way up, so it is laid back as their mean.
        mFinal->SetHalo(Glow ? Bloom.Strength / kLevels : 0.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetBloomScale(Target::Scale Sizing)
    {
        constexpr UInt32 Smallest = ZyEnum::Cast(Target::Scale::ThirtySecond);

        // Each level halves the one above, down to the smallest fraction a target tracks the output by.
        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            const Target::Scale Step = static_cast<Target::Scale>(Min<UInt32>(ZyEnum::Cast(Sizing) + Level, Smallest));

            mBlueprint.ResizeTarget(mDown[Level], Step);

            if (Level < kLevels - 1)
            {
                mBlueprint.ResizeTarget(mUp[Level], Step);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetExposure(ConstRef<Exposure> Exposure)
    {
        mFinal->SetExposure(Exposure);
        mAdapt->SetExposure(Exposure);

        for (const Ptr<PostEffectExposure> Step : mMeter)
        {
            Step->SetActive(Exposure.Auto);
        }
    }
}