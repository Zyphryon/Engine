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

        mFinal = AddressOf(Blueprint.AddPass<PostEffectFinal>("Final", Host, Scene, Output == Pass::kNone));
        mFinal->AddColor({ .Target = Output, .Load = ZyGraphic::Action::Discard });
        mFinal->AddInput({ .Name = "Scene"_Hash, .Target = Scene  });
        mFinal->AddInput({ .Name = "Bloom"_Hash, .Target = mUp[0] });

        // The bloom does not run until something asks for it.
        SetBloom(0.0f, 1.0f);
        SetBloomScale(Target::Scale::Quarter);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::Advance(Real64 Time)
    {
        // The grain's frame is counted off the clock, and wrapped while a float still carries it whole.
        const Real64 Count = Floor(Time * kCadence);
        mFinal->SetFrame(static_cast<UInt32>(Count - Floor(Count / kFrames) * kFrames));
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetBloom(Real32 Strength, Real32 Threshold)
    {
        const Bool Glow = (Strength > 0.0f);

        mBloom[0]->SetThreshold(Threshold);

        for (const Ptr<PostEffectBloom> Step : mBloom)
        {
            Step->SetActive(Glow);
        }

        // The halo sums every level on the way up, so it is laid back as their mean.
        mFinal->SetHalo(Glow ? Strength / kLevels : 0.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffect::SetBloomScale(Target::Scale Sizing)
    {
        constexpr UInt32 Smallest = ZyEnum::Cast(Target::Scale::Sixteenth);

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
}