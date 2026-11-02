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

#include "Swarm.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Swarm::Shift(Vector3 Offset)
    {
        for (Ref<Particle> Each : mParticles)
        {
            Each.Position += Offset;
            Each.Floor    += Offset.GetY();
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Swarm::Advance(ConstRef<Emitter> Emitter, ConstRef<Matrix4x3> Worldspace, Vector3 Wind, Real32 Delta)
    {
        // The pull is authored in the emitter's own frame, so it turns and stretches with whatever throws it.
        const Vector3          Gravity = Matrix4x3::ProjectDirection(Worldspace, Emitter.GetGravity()) * Delta;
        const Real32           Drag    = Clamp(1.0f - Emitter.GetDrag() * Delta, 0.0f, 1.0f);
        const Emitter::Contact Meeting = Emitter.GetContact();

        // The wind carries a particle along with it rather than speeding it up, so it is laid on each step whole.
        mCarry = Wind * Emitter.GetWind();

        const Vector3 Carried = mCarry * Delta;

        // The wander only shifts where a particle is drawn and never lifts it, so the ground is met as if straight.
        mParticles.RemoveFastSomeIf([&](Ref<Particle> Each) -> Bool
        {
            Each.Age += Delta;

            if (Each.Age >= Each.Life)
            {
                return true;
            }

            // One that settled stays where it lay, so neither the pull nor the wind moves it again.
            if (Each.Landed)
            {
                return false;
            }

            Each.Velocity = (Each.Velocity + Gravity) * Drag;
            Each.Position = Each.Position + Each.Velocity * Delta + Carried;

            if (Meeting != Emitter::Contact::None && Each.Position.GetY() < Each.Floor)
            {
                if (Meeting == Emitter::Contact::Kill)
                {
                    return true;
                }

                Each.Position = Vector3::FromXZ(Each.Position.GetXZ(), Each.Floor);
                Each.Velocity = Vector3::Zero();
                Each.Landed   = true;
            }
            return false;
        });
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    void Swarm::Spawn(ConstRef<Emitter> Emitter, ConstRef<Matrix4x3> Worldspace, Real32 Presence, Real32 Delta)
    {
        const Vector3 Position = Worldspace.GetTranslation();
        const UInt16  Limit    = Emitter.GetLimit();

        // A burst is owed the moment the emitter is armed, and is thrown whole rather than over time.
        if (!mIgnited)
        {
            mIgnited  = true;
            mBacklog += Min(Emitter.GetBurst(), Limit);

            mParticles.Reserve(Limit);
        }

        // What a frame owes carries to the next so a slow rate still throws, thinned by the emitter's presence.
        mBacklog += Emitter.GetRate() * Presence * Delta;

        if (mBacklog < 1.0f)
        {
            return;
        }

        // The cone is built once in the emitter's own frame, so every particle of this frame reuses it.
        const Vector3 Forward = Emitter.GetDirection();
        const Vector3 Aside   = Vector3::Normalize(
            Abs(Forward.GetY()) > 0.99f
                ? Vector3::Cross(Forward, Vector3::UnitX())
                : Vector3::Cross(Forward, Vector3::UnitY()));

        const Vector3 Above  = Vector3::Cross(Forward, Aside);
        const Real32  Widest = Cosine(Emitter.GetSpread().GetRadians());

        while (mBacklog >= 1.0f)
        {
            mBacklog -= 1.0f;

            if (mParticles.GetSize() >= Limit)
            {
                mBacklog = 0.0f;
                break;
            }

            const Angle  Bearing = Angle::FromTurns(mRandom.NextReal<Real32>());
            const Real32 Lean    = mRandom.NextReal<Real32>(Widest, 1.0f);
            const Real32 Open    = Sqrt(Max(0.0f, 1.0f - Lean * Lean));

            const Vector3 Heading
                = Forward * Lean + (Aside * Angle::Cosine(Bearing) + Above * Angle::Sine(Bearing)) * Open;

            Ref<Particle> Each = mParticles.Append();

            Each.Position = Position;
            Each.Velocity = Matrix4x3::ProjectDirection(Worldspace, Heading * Emitter.GetSpeed().Pick(mRandom));
            Each.Age      = 0.0f;
            Each.Life     = Emitter.GetLife().Pick(mRandom);
            Each.Seed     = mRandom.NextInteger<UInt8>(0, 255);
            Each.Floor    = Position.GetY();
            Each.Variant  = mRandom.NextInteger<UInt8>(0, 255);
            Each.Landed   = false;
        }
    }
}