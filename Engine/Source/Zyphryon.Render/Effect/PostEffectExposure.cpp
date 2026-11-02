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

#include "PostEffectExposure.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    PostEffectExposure::PostEffectExposure(Ref<ZyEngine::Subsystem::Host> Host, Sweep Axis)
        : mTechnique { Host.GetService<ZyContent::Service>()->Load<ZyGraphic::Technique>(kTechnique) },
          mSweep     { Axis },
          mAuto      { false },
          mFresh     { false },
          mRange     { Vector2::Zero() },
          mShares    { Vector2(1.0f) },
          mTime      { 0.0 }
    {
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectExposure::Run(Ref<Encoder> Encoder, ConstRef<Graph> Graph)
    {
        ZY_PROFILE_SCOPE("Render::PostEffectExposure::Run");

        Vector4 Filter = Vector4(1.0f / kMeter, 1.0f / kMeter, 0.0f, 0.0f);

        // A manual exposure, or one just turned automatic, settles in the frame rather than easing toward it.
        if (mSweep == Sweep::Adapt)
        {
            const Vector2 Shares = (!mAuto || mFresh ? Vector2(1.0f) : mShares);

            Filter = Vector4(Shares.GetX(), Shares.GetY(), mRange.GetX(), mRange.GetY());
            mFresh = false;
        }

        Encoder.SetPass(GpuExposureLayout { .Filter = Filter });

        Encoder::Binder Binder = Encoder.Begin(* mTechnique);

        if (mSweep != Sweep::Measure)
        {
            Binder.SetVariant(ZyEnum::GetName(mSweep));
        }

        Binder.DrawFullscreen();
    }
    
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectExposure::SetExposure(ConstRef<PostEffectFinal::Exposure> Exposure)
    {
        // An exposure turning automatic takes the meter's reading at once, rather than easing from what was kept.
        mFresh = mFresh || (Exposure.Auto && !mAuto);
        mAuto  = Exposure.Auto;

        // A manual exposure is held at no stops at all, which leaves the level as the whole of it.
        mRange = Vector2::Zero();

        if (mAuto)
        {
            mRange = Vector2(
                Exposure.Minimum > 0.0f ? Log(Exposure.Minimum) : -kStops,
                Exposure.Maximum > 0.0f ? Log(Exposure.Maximum) : +kStops);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void PostEffectExposure::SetTime(Real64 Time)
    {
        const Real32 Delta = static_cast<Real32>(Clamp(Time - mTime, 0.0, kStep));

        mTime   = Time;
        mShares = Vector2(Settle(Delta, kBrighten), Settle(Delta, kDarken));
    }
}