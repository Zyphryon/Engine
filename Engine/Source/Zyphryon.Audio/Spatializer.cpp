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

#include "Spatializer.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace Audio
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Real32 Attenuate(Attenuation Model, Real32 Distance, Real32 Inner, Real32 Outer)
    {
        if (Distance <= Inner)
        {
            return 1.0f;
        }

        const Real32 Clamped = Min(Distance, Outer);

        switch (Model)
        {
        case Attenuation::Linear:
            return (Outer - Clamped) / Max(Outer - Inner, kEpsilon<Real32>);
        case Attenuation::Inverse:
            return Inner / Clamped;
        case Attenuation::Exponential:
            return (Inner / Clamped) * (Inner / Clamped);
        }
        return 1.0f;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    static Real32 Cone(Real32 Angle, Real32 HalfInner, Real32 HalfOuter, Real32 OuterGain)
    {
        if (Angle <= HalfInner)
        {
            return 1.0f;
        }
        if (Angle >= HalfOuter)
        {
            return OuterGain;
        }

        const Real32 Factor = (Angle - HalfInner) / Max(HalfOuter - HalfInner, kEpsilon<Real32>);
        return 1.0f + (OuterGain - 1.0f) * Factor;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Spatializer::Gains Spatializer::Compute(Vector3 Position, Vector3 Forward, ConstRef<Emitter> Emitter) const
    {
        Gains Result;

        const Vector3 Delta    = Position - mPosition;
        const Real32  Distance = Delta.GetLength();

        // Sources at the listener's position collapse to an equal-power centered image.
        if (Distance < kEpsilon<Real32>)
        {
            Result.Left = Result.Right = 0.70710678f;
            return Result;
        }

        const Vector3 Normal = Delta / Distance;

        // Distance attenuation.
        Real32 Attenuation = Attenuate(
            Emitter.GetAttenuation(), Distance, Emitter.GetInnerRadius(), Emitter.GetOuterRadius());

        // Emitter cone: angle between the source's forward axis and the direction toward the listener.
        const Real32 EmitterCos   = Clamp(Vector3::Dot(Forward, -Normal), -1.0f, 1.0f);
        const Real32 EmitterAngle = Angle::FromCosine(EmitterCos).GetRadians();
        Attenuation *= Cone(
            EmitterAngle,
            Emitter.GetInnerAngle().GetRadians() * 0.5f,
            Emitter.GetOuterAngle().GetRadians() * 0.5f,
            Emitter.GetOuterGain());

        // Listener cone: angle between the listener's forward axis and the direction toward the source.
        const Real32 ListenerCos   = Clamp(Vector3::Dot(mForward, Normal), -1.0f, 1.0f);
        const Real32 ListenerAngle = Angle::FromCosine(ListenerCos).GetRadians();
        Attenuation *= Cone(ListenerAngle, mListenerHalfInner, mListenerHalfOuter, mListenerGain);

        // Constant-power stereo pan from the source's position along the listener's right axis.
        const Real32 Pan = Clamp((Vector3::Dot(Normal, mRight) + 1.0f) * 0.5f, 0.0f, 1.0f);
        Result.Left  = Sqrt(1.0f - Pan) * Attenuation;
        Result.Right = Sqrt(Pan) * Attenuation;

        return Result;
    }
}