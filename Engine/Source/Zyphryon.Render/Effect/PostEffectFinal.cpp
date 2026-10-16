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

    PostEffectFinal::PostEffectFinal(Ref<ZyEngine::Subsystem::Host> Host, UInt32 Source, Bool Surface)
        : mTechnique { Host.GetService<ZyContent::Service>()->Load<ZyGraphic::Technique>(kTechnique) },
          mSource    { Source },
          mSurface   { Surface },
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

        const Vector3 Lift  = mGrade.Lift * kLiftRange;
        const Vector3 Gamma = mGrade.Gamma;
        const Vector3 Gain  = mGrade.Gain;

        // A grade that pushes no band, grains nothing and steps nothing hands back the curve's image as it stands.
        const Bool Plain = Lift.IsAlmostZero()
                        && Gamma.IsAlmostZero()
                        && Gain.IsAlmostZero()
                        && mLens.Grain <= 0.0f
                        && mGrade.Posterize <= 1.0f;

        const UInt16 Width  = Graph.GetWidth(mSource);
        const UInt16 Height = Graph.GetHeight(mSource);

        // The tone map's knee sits at a fixed place in the range, so exposure is what moves the image into it.
        Encoder.SetPass(GpuFinalLayout {
            .Grading = Vector4(mExposure.Level, mGrade.Saturation, mLens.Vignette, mLens.Grain),
            .Lift    = Vector4(Lift, Plain ? -1.0f : static_cast<Real32>(mFrame)),
            .Gamma   = Vector4(Pow(2.0f, -Gamma.GetX()), Pow(2.0f, -Gamma.GetY()), Pow(2.0f, -Gamma.GetZ()), mHalo),
            .Gain    = Vector4(1.0f + Gain.GetX(), 1.0f + Gain.GetY(), 1.0f + Gain.GetZ(), mGrade.Posterize),
            .Lens    = Vector4(Width  > 0 ? 1.0f / Width  : 0.0f,
                               Height > 0 ? 1.0f / Height : 0.0f,
                               mLens.Aberration,
                               0.0f)
        });

        Encoder::Binder Binder = Encoder.Begin(* mTechnique);

        if (mExposure.Curve != Tonemap::GT7)
        {
            Binder.SetVariant(ZyEnum::GetName(mExposure.Curve));
        }

        if (mAntialias != Antialias::None)
        {
            Binder.SetVariant(ZyEnum::GetName(mAntialias));
        }

#ifdef    ZY_PLATFORM_WEB
        // The web's display surface stores what it is handed, so the image is encoded before it is written.
        if (mSurface)
        {
            Binder.SetVariant("Gamma");
        }
#endif // ZY_PLATFORM_WEB

        Binder.DrawFullscreen();
    }
}