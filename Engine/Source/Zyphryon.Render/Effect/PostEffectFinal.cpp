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

        const Vector3 Lift  = mGrade.Lift;
        const Vector3 Gamma = mGrade.Gamma;
        const Vector3 Gain  = mGrade.Gain;

        // A grade that pushes no band, grains nothing and steps nothing hands back the curve's image as it stands.
        const Bool Plain = Lift.IsAlmostZero()
                        && Gamma.IsAlmostZero()
                        && Gain.IsAlmostZero()
                        && mLens.Grain <= 0.0f
                        && mGrade.Posterize <= 1.0f;

        const Real32 TexelX = 1.0f / Graph.GetWidth(mSource);
        const Real32 TexelY = 1.0f / Graph.GetHeight(mSource);
        const Real32 Steps  = Max(mGrade.Posterize - 1.0f, 0.0f);

        Encoder.SetPass(GpuFinalLayout {
            .Grading = Vector4(mExposure.Level, mGrade.Saturation, 2.0f * mLens.Vignette, mLens.Grain),
            .Lift    = Vector4(Lift, Plain ? -1.0f : static_cast<Real32>(mFrame)),
            .Gamma   = Vector4(Pow(2.0f, -Gamma.GetX()), Pow(2.0f, -Gamma.GetY()), Pow(2.0f, -Gamma.GetZ()), mHalo),
            .Gain    = Vector4(1.0f + Gain.GetX(), 1.0f + Gain.GetY(), 1.0f + Gain.GetZ(), Steps),
            .Lens    = Vector4(TexelX, TexelY, 2.0f * mLens.Aberration * TexelX, 2.0f * mLens.Aberration * TexelY)
        });

        Encoder::Binder Binder = Encoder.Begin(* mTechnique);

        if (mExposure.Curve != Tonemap::Neutral)
        {
            Binder.SetVariant(ZyEnum::GetName(mExposure.Curve));
        }

        if (mAntialias != Antialias::None)
        {
            Binder.SetVariant(ZyEnum::GetName(mAntialias));
        }

#ifdef    ZY_PLATFORM_WEB
        // The web's display surface stores what it is handed, so the image is encoded before it is written.
        if (GetColor().Target == kNone)
        {
            Binder.SetVariant("Gamma");
        }
#endif // ZY_PLATFORM_WEB

        Binder.DrawFullscreen();
    }
}