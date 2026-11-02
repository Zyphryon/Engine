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

#include "Emitter.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Box Emitter::Measure() const
    {
        // A filled shape stands on the origin, a unit across and a unit tall, which its frame stretches.
        if (IsArea())
        {
            return Box(Vector3(-0.5f, 0.0f, -0.5f), Vector3(0.5f, 1.0f, 0.5f));
        }

        const Real32 Life = mLife.GetMaximum();

        // Drag bleeds the launch away, so the travel is held at one time constant instead of solving an exponential.
        const Real32 Travel = mSpeed.GetMaximum() * (mDrag > 0.0f ? Min(Life, 1.0f / mDrag) : Life);

        // The pull is bounded by what it would add unopposed, since drag only ever takes from it.
        const Vector3 Drift   = mGravity * (0.5f * Life * Life);
        const Vector3 Forward = mDirection;

        // The throw is a cone, not a ball, so an axis is reached only as far as the widest heading leans onto it.
        const Real32 Lean = Cosine(mSpread.GetRadians());
        const Real32 Open = Sine(mSpread.GetRadians());

        const auto Along = [Lean, Travel, Open](Real32 Axis)
        {
            const Real32 Sideways = Sqrt(Max(0.0f, 1.0f - Axis * Axis));
            const Real32 Most     =  Axis >= Lean ?  1.0f : Axis * Lean + Sideways * Open;
            const Real32 Least    = -Axis >= Lean ? -1.0f : Axis * Lean - Sideways * Open;

            return Vector2(Min(0.0f, Travel * Least), Max(0.0f, Travel * Most));
        };

        const Vector2 SpanX = Along(Forward.GetX());
        const Vector2 SpanY = Along(Forward.GetY());
        const Vector2 SpanZ = Along(Forward.GetZ());

        // The wobble and the strongest wind lie on the ground axes, and the art hangs half its size and streak past.
        const Real32  Half  = Max(mScale.GetX(), mScale.GetY()) * 0.5f * (1.0f + mVariance) + mSpeed.GetMaximum() * mLength;
        const Real32  Blown = mWind * kWindReach * Life;
        const Vector3 Edge(Half + mWander.GetX() + Blown, Half, Half + mWander.GetX() + Blown);

        const Vector3 Minimum(
            SpanX.GetX() + Min(Drift.GetX(), 0.0f) - Edge.GetX(),
            SpanY.GetX() + Min(Drift.GetY(), 0.0f) - Edge.GetY(),
            SpanZ.GetX() + Min(Drift.GetZ(), 0.0f) - Edge.GetZ());

        const Vector3 Maximum(
            SpanX.GetY() + Max(Drift.GetX(), 0.0f) + Edge.GetX(),
            SpanY.GetY() + Max(Drift.GetY(), 0.0f) + Edge.GetY(),
            SpanZ.GetY() + Max(Drift.GetZ(), 0.0f) + Edge.GetZ());

        return Box(Minimum, Maximum);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Emitter::Pack(Ref<GpuEmitterLayout> Output, ConstRef<Matrix4x3> Frame, Vector3 Wind, Real32 Time, UInt32 Count, UInt32 Seed) const
    {
        const Vector3 Origin = Frame.GetTranslation();
        const Vector3 AxisX  = Frame.GetBasisX();
        const Vector3 AxisY  = Frame.GetBasisY();
        const Vector3 AxisZ  = Frame.GetBasisZ();
        const Vector3 Carry  = Wind * mWind;

        Output.Origin = Vector4(Origin, 0.0f);
        Output.AxisX  = Vector4(AxisX,  0.0f);
        Output.AxisY  = Vector4(AxisY,  0.0f);
        Output.AxisZ  = Vector4(AxisZ,  0.0f);
        Output.Cone   = Vector4(mDirection, Cosine(mSpread.GetRadians()));
        Output.Launch = Vector4(mSpeed.GetMinimum(), mSpeed.GetMaximum(), mLife.GetMinimum(), mLife.GetMaximum());
        Output.Pull   = Vector4(mGravity, mDrag);
        Output.Carry  = Vector4(Carry, Time);
        Output.Sway   = Vector4(mWander.GetX(), mWander.GetY(), mSpin.GetX(), mSpin.GetY());
        Output.Size   = Vector4(mScale.GetX(), mScale.GetY(), mGrowth, mVariance);
        Output.Crop   = Vector4(mSource.GetMinimumX(), mSource.GetMinimumY(), mSource.GetWidth(), mSource.GetHeight());
        Output.Card   = Vector4(mLength, static_cast<Real32>(ZyEnum::Cast(mSpan)), 0.0f, 0.0f);
        Output.Count  = Array(Count, Seed,
                              static_cast<UInt32>(ZyEnum::Cast(mShape)),
                              static_cast<UInt32>(ZyEnum::Cast(mContact)));
        Output.Grid   = Array(static_cast<UInt32>(mVariants.GetX()),
                              static_cast<UInt32>(mVariants.GetY()),
                              mAnimated ? 1u : 0u, 0u);

        // The gradient is held flat past its last stop, and white stands in for one never authored.
        Array<Real32, kMaxStops> Times;

        for (UInt32 Index = 0; Index < kMaxStops; ++Index)
        {
            const Bool  Held = mGradient.IsEmpty();
            const UInt  Pick = Held ? 0 : Min<UInt>(Index, mGradient.GetSize() - 1);
            const Color Stop = Color::FromColor8(Held ? IntColor8::White() : mGradient[Pick].Value).ToLinear();

            Output.Stops[Index] = Vector4(Stop.GetRed(), Stop.GetGreen(), Stop.GetBlue(), Stop.GetAlpha());
            Times[Index]        = Held ? 0.0f : mGradient[Pick].Time;
        }
        Output.Times = Vector4(Times[0], Times[1], Times[2], Times[3]);
    }
}