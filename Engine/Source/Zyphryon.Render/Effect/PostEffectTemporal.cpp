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

#include "PostEffectTemporal.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectTemporal::PostEffectTemporal(Ref<ZyEngine::Subsystem::Host> Host, Ref<Blueprint> Blueprint, UInt32 Scene)
        : mScene  { Scene },
          mRound  { 0 },
          mActive { false }
    {
        const Target Image(Blueprint.GetTargets()[Scene].Format);

        for (UInt32 Turn = 0; Turn < mSteady.GetSize(); ++Turn)
        {
            mSteady[Turn] = Blueprint.AddTarget(Image);
        }

        // Each pass blends the scene into the image the other steadied last, so neither reads what it writes.
        for (UInt32 Turn = 0; Turn < mBlend.GetSize(); ++Turn)
        {
            mBlend[Turn] = AddressOf(Blueprint.AddPass<Step>("Steady", Host, Scene));
            mBlend[Turn]->AddColor({ .Target = mSteady[Turn], .Load = ZyGraphic::Action::Discard });
            mBlend[Turn]->AddInput({ .Name = "Scene"_Hash, .Target = Scene             });
            mBlend[Turn]->AddInput({ .Name = "Level"_Hash, .Target = mSteady[Turn ^ 1] });
            mBlend[Turn]->SetActive(false);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    UInt32 PostEffectTemporal::Advance()
    {
        const UInt32 Turn = (mRound & 1);

        mBlend[Turn]->SetActive(mActive);
        mBlend[Turn ^ 1]->SetActive(false);

        ++mRound;

        return mActive ? mSteady[Turn] : mScene;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Vector2 PostEffectTemporal::GetJitter() const
    {
        // A round of offsets spread evenly over the pixel, which eight images in a row cover between them.
        static constexpr Array kRound = {
            Vector2( 0.000f, -0.167f),
            Vector2(-0.250f,  0.167f),
            Vector2( 0.250f, -0.389f),
            Vector2(-0.375f, -0.056f),
            Vector2( 0.125f,  0.278f),
            Vector2(-0.125f, -0.278f),
            Vector2( 0.375f,  0.056f),
            Vector2(-0.438f,  0.389f),
        };
        return mActive ? kRound[mRound % kRound.GetSize()] : Vector2::Zero();
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectTemporal::Step::Step(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source)
        : mTechnique { Host.GetService<ZyContent::Service>()->Load<ZyGraphic::Technique>(kTechnique) },
          mSource    { Source },
          mShift     { Vector2::Zero() },
          mScale     { Vector2(1.0f) }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectTemporal::Step::Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph)
    {
        ZY_PROFILE_SCOPE("Render::PostEffectTemporal::Run");

        const Real32 Wide = Graph.GetWidth(mSource);
        const Real32 High = Graph.GetHeight(mSource);

        // A view on the move takes more of each new image, since what was kept was drawn from somewhere else.
        const Real32 Drift = 0.5f * Max(
            (Abs(mShift.GetX()) + Abs(mScale.GetX() - 1.0f)) * Wide,
            (Abs(mShift.GetY()) + Abs(mScale.GetY() - 1.0f)) * High);
        const Real32 Share = Lerp(kStill, kMoving, Min(Drift, 1.0f));

        Encoder.SetPass(GpuTemporalLayout {
            .Shift = Vector4(mShift.GetX(), mShift.GetY(), Share, 0.0f),
            .Texel = Vector4(1.0f / Wide, 1.0f / High, mScale.GetX(), mScale.GetY())
        });

        Encoder.Begin(* mTechnique).DrawFullscreen();
    }
}