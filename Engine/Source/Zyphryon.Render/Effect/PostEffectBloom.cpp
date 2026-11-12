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

#include "PostEffectBloom.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectBloom::PostEffectBloom(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Image)
        : mBlueprint { Blueprint }
    {
        // Every level on the way down has one beside it on the way up, but for the last, which turns the chain round.
        const Target Glow(Blueprint.GetTargets()[Image].Format);

        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            mDown[Level] = Blueprint.AddTarget(Glow);
        }

        for (UInt32 Level = 0; Level < kLevels - 1; ++Level)
        {
            mUp[Level] = Blueprint.AddTarget(Glow);
        }

        // The sift keeps what clears the threshold as it halves the image, and every step after halves it again.
        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            const UInt32 Source = (Level == 0 ? Image : mDown[Level - 1]);
            const Sweep  Axis   = (Level == 0 ? Sweep::Sift : Sweep::Down);

            Ref<Step> Down = Blueprint.AddPass<Step>(Level == 0 ? "Sift" : "Down", Host, Source, Axis);
            Down.AddColor({ .Target = mDown[Level], .Load = ZyGraphic::Action::Discard });
            Down.AddInput({ .Name = "Scene"_Hash, .Target = Source });
            mSteps.Append(AddressOf(Down));
        }

        // The way back up spreads the level below over each one, adding what that level held on the way down.
        for (UInt32 Level = kLevels - 1; Level-- > 0;)
        {
            const UInt32 Source = (Level + 2 < kLevels ? mUp[Level + 1] : mDown[Level + 1]);

            Ref<Step> Up = Blueprint.AddPass<Step>("Up", Host, Source, Sweep::Up);
            Up.AddColor({ .Target = mUp[Level], .Load = ZyGraphic::Action::Discard });
            Up.AddInput({ .Name = "Scene"_Hash, .Target = Source       });
            Up.AddInput({ .Name = "Level"_Hash, .Target = mDown[Level] });
            mSteps.Append(AddressOf(Up));
        }

        SetScale(Target::Scale::Quarter);
        SetActive(false);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectBloom::SetActive(Bool Active)
    {
        for (const Ptr<Step> Each : mSteps)
        {
            Each->SetActive(Active);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectBloom::SetScale(Target::Scale Sizing)
    {
        constexpr UInt32 Smallest = ZyEnum::Cast(Target::Scale::ThirtySecond);

        // Each level halves the one above, down to the smallest fraction a target tracks the output by.
        for (UInt32 Level = 0; Level < kLevels; ++Level)
        {
            const Target::Scale Size = static_cast<Target::Scale>(Min<UInt32>(ZyEnum::Cast(Sizing) + Level, Smallest));

            mBlueprint.ResizeTarget(mDown[Level], Size);

            if (Level < kLevels - 1)
            {
                mBlueprint.ResizeTarget(mUp[Level], Size);
            }
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectBloom::Step::Step(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source, Sweep Axis)
        : mTechnique { Host.GetService<ZyContent::Service>()->Load<ZyGraphic::Technique>(kTechnique) },
          mSource    { Source },
          mSweep     { Axis },
          mThreshold { 1.0f }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectBloom::Step::Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph)
    {
        ZY_PROFILE_SCOPE("Render::PostEffectBloom::Run");

        // Every step taps by a texel of the level it reads, whichever way the chain is being walked.
        const Real32 Across = 1.0f / Graph.GetWidth(mSource);
        const Real32 Up     = 1.0f / Graph.GetHeight(mSource);

        Encoder.SetPass(GpuBloomLayout { .Filter = Vector4(mThreshold, kKnee, Across, Up) });

        Encoder::Binder Binder = Encoder.Begin(* mTechnique);

        if (mSweep != Sweep::Down)
        {
            Binder.SetVariant(ZyEnum::GetName(mSweep));
        }

        Binder.DrawFullscreen();
    }
}