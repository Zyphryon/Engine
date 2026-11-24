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

#include "PostEffectFinal.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectFinal::PostEffectFinal(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source)
        : mTechnique { Host.GetService<ZyContent::Service>()->Load<ZyGraphic::Technique>(kTechnique) },
          mSource    { Source },
          mAntialias { Antialias::None },
          mHalo      { 0.0f },
          mFrame     { 0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectFinal::Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph)
    {
        ZY_PROFILE_SCOPE("Render::PostEffectFinal::Run");

        const Bool Graded = (mGrade.Table && mGrade.Blend > 0.0f);

        // A grade that looks nothing up, grains nothing and steps nothing hands back the curve's image as it stands.
        const Bool Plain = (!Graded && mLens.Grain <= 0.0f && mGrade.Posterize <= 1.0f);

        const Real32 TexelX = 1.0f / Graph.GetWidth(mSource);
        const Real32 TexelY = 1.0f / Graph.GetHeight(mSource);
        const Real32 Steps  = Max(mGrade.Posterize - 1.0f, 0.0f);

        Encoder.SetPass(GpuFinalLayout {
            .Grading = Vector4(mExposure.Level, mGrade.Saturation, 2.0f * mLens.Vignette, mLens.Grain),
            .Finish  = Vector4(Plain ? -1.0f : static_cast<Real32>(mFrame), mHalo, Steps, 0.0f),
            .Lens    = Vector4(TexelX, TexelY, 2.0f * mLens.Aberration * TexelX, 2.0f * mLens.Aberration * TexelY),
            .Output  = Vector4(GetColor().Target == kNone ? 1.0f : 0.0f, mGrade.Blend, 0.0f, 0.0f)
        });

        Encoder::Binder Binder = Encoder.Begin(* mTechnique);

        if (mExposure.Curve != Tonemap::Neutral)
        {
            Binder.SetVariant(ZyEnum::GetName(mExposure.Curve));
        }

        // The grade turns on with the table bound, so a missing table reads as none.
        if (Graded)
        {
            Binder.SetImage("Grading"_Hash, mGrade.Table);
        }

        if (mAntialias != Antialias::None)
        {
            Binder.SetVariant(ZyEnum::GetName(mAntialias));
        }

        Binder.DrawFullscreen();
    }
}