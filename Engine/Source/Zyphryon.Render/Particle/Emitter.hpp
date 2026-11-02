// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#pragma once

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [  HEADER  ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Zyphryon.Math/Color.hpp"
#include "Zyphryon.Math/Curve/Ramp.hpp"
#include "Zyphryon.Math/Geometry/Box.hpp"
#include "Zyphryon.Math/Range.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents a source of particles, and everything the ones it throws are born with.
    class ZY_API Emitter final
    {
    public:

        /// \brief The greatest number of particles one emitter may hold at once.
        static constexpr UInt16 kMaxParticles = 1024;

        /// \brief The greatest number of colours a particle may pass through over its life.
        static constexpr UInt   kMaxStops     = 4;

        /// \brief The strongest wind a swarm's reach allows for, in world units per second.
        static constexpr Real32 kWindReach    = 8.0f;

        /// \brief Enumerates the pair of directions a particle spans.
        enum class Span : UInt8
        {
            Billboard,  ///< Faces the camera, so the art lands the way it was drawn.
            Vertical,   ///< Stands upright and turns about the vertical axis to face the camera.
            Horizontal, ///< Lies flat against the ground.
        };

        /// \brief Enumerates what a particle does where it comes back down to the height it was thrown from.
        enum class Contact : UInt8
        {
            None,    ///< Falls straight through, which is what art that never comes down wants.
            Kill,    ///< Dies where it lands, taking the rest of its life with it.
            Settle,  ///< Stops where it lands and lives out its life there.
        };

        /// \brief Enumerates what particles are let go from, which also decides how they are made.
        enum class Shape : UInt8
        {
            Point,  ///< Thrown from the origin, each one carried forward on its own by a swarm.
            Box,    ///< Fills a unit box standing on the origin, endlessly, drawn by a filler rather than a swarm.
            Column, ///< Fills a unit upright cylinder standing on the origin, endlessly, drawn by a filler as well.
        };

        /// \brief The colours a particle passes through, held inline since an emitter is read every frame.
        using Gradient = Ramp<IntColor8, kMaxStops>;

        /// \brief Represents an emitter as the particle shader library reads it, the `ZyEmitter` of `Particle.hlsl`.
        struct GpuEmitterLayout final
        {
            /// The point the frame stands on, which a filled shape stands on and a point throws from.
            Vector4                   Origin;

            /// The frame's x axis, which stretches the shape and whose way alone turns the throw.
            Vector4                   AxisX;

            /// The frame's y axis, which stretches the shape and whose way alone turns the throw.
            Vector4                   AxisY;

            /// The frame's z axis, which stretches the shape and whose way alone turns the throw.
            Vector4                   AxisZ;

            /// The way the cone points in the emitter's own frame, and the cosine of its half-angle.
            Vector4                   Cone;

            /// The slowest and fastest throw in world units per second, and the shortest and longest life in seconds.
            Vector4                   Launch;

            /// The pull in the emitter's own frame in world units per second squared, and the share of speed shed.
            Vector4                   Pull;

            /// The wind's carry in world units per second, and the moment the particles stand at in seconds.
            Vector4                   Carry;

            /// The wander's push and turn rate, and the widest start turn and fastest turn rate in degrees.
            Vector4                   Sway;

            /// The size at birth and at death in world units, the share of life spent growing, and the variance.
            Vector4                   Size;

            /// The crop every variant is cut from, its corner and its size in normalized texture coordinates.
            Vector4                   Crop;

            /// The seconds of travel a streak covers, the span as its index, and two spares.
            Vector4                   Card;

            /// The colours a particle passes through, in linear light with its cover in alpha.
            Array<Vector4, kMaxStops> Colors;

            /// The share of life each colour is worn at, the unused ones held at the last.
            Vector4                   Times;

            /// The particles drawn, the seed they roll from, the shape as its index and the contact as its index.
            Array<UInt32, 4>          Count;

            /// The variants across and down, one when they play as a flipbook, and a spare.
            Array<UInt32, 4>          Grid;
        };

    public:

        /// \brief Constructs an emitter throwing nothing.
        ZY_INLINE Emitter()
            : mDirection { Vector3(0.0f, 1.0f, 0.0f) },
              mGravity   { Vector3::Zero() },
              mSource    { Rect::One() },
              mLife      { 1.0f, 1.0f },
              mSpeed     { 1.0f, 1.0f },
              mScale     { 1.0f, 1.0f },
              mWander    { 0.0f, 1.0f },
              mSpin      { 0.0f, 0.0f },
              mRate      { 0.0f },
              mSpread    { Angle() },
              mDrag      { 0.0f },
              mVariance  { 0.0f },
              mWind      { 0.0f },
              mLength    { 0.0f },
              mGrowth    { 0.0f },
              mBurst     { 0 },
              mLimit     { kMaxParticles },
              mVariants  { 1, 1 },
              mSpan      { Span::Billboard },
              mContact   { Contact::None },
              mShape     { Shape::Point },
              mAnimated  { false }
        {
        }

        /// \brief Sets the crop of the art every particle shows.
        ///
        /// \param Source The source rectangle to set, which the variants divide between them.
        ZY_INLINE void SetSource(Rect Source)
        {
            mSource = Source;
        }

        /// \brief Gets the crop of the art every particle shows.
        ///
        /// \return The source rectangle.
        ZY_INLINE Rect GetSource() const
        {
            return mSource;
        }

        /// \brief Sets how many particles are thrown each second.
        ///
        /// \param Rate The rate to throw at, each second over a unit of ground for a filled shape, where zero is none.
        ZY_INLINE void SetRate(Real32 Rate)
        {
            mRate = Max(Rate, 0.0f);
        }

        /// \brief Gets how many particles are thrown each second.
        ///
        /// \return The rate particles are thrown at.
        ZY_INLINE Real32 GetRate() const
        {
            return mRate;
        }

        /// \brief Sets how many particles are thrown the moment the emitter is armed.
        ///
        /// \param Burst The number to throw at once, which the emitter clamps to its own limit.
        ZY_INLINE void SetBurst(UInt16 Burst)
        {
            mBurst = Min(Burst, kMaxParticles);
        }

        /// \brief Gets how many particles are thrown the moment the emitter is armed.
        ///
        /// \return The number thrown at once, or zero for an emitter that only trickles.
        ZY_INLINE UInt16 GetBurst() const
        {
            return mBurst;
        }

        /// \brief Sets how long a particle lives for.
        ///
        /// \param Life The shortest and longest a particle may live, in seconds, where below zero counts as zero.
        ZY_INLINE void SetLife(Range Life)
        {
            mLife = Range(Max(Life.GetMinimum(), 0.0f), Max(Life.GetMaximum(), 0.0f));
        }

        /// \brief Gets how long a particle lives for.
        ///
        /// \return The shortest and longest a particle may live, in seconds.
        ZY_INLINE Range GetLife() const
        {
            return mLife;
        }

        /// \brief Sets the way the cone of thrown particles points.
        ///
        /// \param Direction The direction the cone points, which is normalized.
        ZY_INLINE void SetDirection(Vector3 Direction)
        {
            mDirection = Direction.IsAlmostZero() ? Vector3::UnitY() : Vector3::Normalize(Direction);
        }

        /// \brief Gets the way the cone of thrown particles points.
        ///
        /// \return The direction the cone points, of unit length.
        ZY_INLINE Vector3 GetDirection() const
        {
            return mDirection;
        }

        /// \brief Sets how wide the cone of thrown particles opens.
        ///
        /// \param Spread The half-angle of the cone, from 0 throwing every particle one way to 180 degrees every way.
        ZY_INLINE void SetSpread(Angle Spread)
        {
            mSpread = Clamp(Spread, Angle(), Angle::FromDegrees(180.0f));
        }

        /// \brief Gets how wide the cone of thrown particles opens.
        ///
        /// \return The half-angle of the cone.
        ZY_INLINE Angle GetSpread() const
        {
            return mSpread;
        }

        /// \brief Sets how fast a particle is thrown.
        ///
        /// \param Speed The slowest and fastest a particle may be thrown, in world units per second.
        ZY_INLINE void SetSpeed(Range Speed)
        {
            mSpeed = Speed;
        }

        /// \brief Gets how fast a particle is thrown.
        ///
        /// \return The slowest and fastest a particle may be thrown.
        ZY_INLINE Range GetSpeed() const
        {
            return mSpeed;
        }

        /// \brief Sets the pull every particle falls under.
        ///
        /// \param Gravity The acceleration to apply, in world units per second squared.
        ZY_INLINE void SetGravity(Vector3 Gravity)
        {
            mGravity = Gravity;
        }

        /// \brief Gets the pull every particle falls under.
        ///
        /// \return The acceleration applied to every particle.
        ZY_INLINE Vector3 GetGravity() const
        {
            return mGravity;
        }

        /// \brief Sets how much of its speed a particle sheds each second.
        ///
        /// \param Drag The fraction of speed shed each second, where zero or less lets a particle keep it all.
        ZY_INLINE void SetDrag(Real32 Drag)
        {
            mDrag = Max(Drag, 0.0f);
        }

        /// \brief Gets how much of its speed a particle sheds each second.
        ///
        /// \return The fraction of speed shed each second.
        ZY_INLINE Real32 GetDrag() const
        {
            return mDrag;
        }

        /// \brief Sets how the air pushes a particle off its course, and how fast that push turns.
        ///
        /// \param Wander The sideways push and the rate it turns at, or a zero push to travel straight.
        ZY_INLINE void SetWander(Vector2 Wander)
        {
            mWander = Wander;
        }

        /// \brief Gets how the air pushes a particle off its course, and how fast that push turns.
        ///
        /// \return The sideways push and the rate it turns at.
        ZY_INLINE Vector2 GetWander() const
        {
            return mWander;
        }

        /// \brief Gets how far the air has pushed a particle off its course, in the emitter's own frame.
        ///
        /// \param Age     The time the particle has been alive, in seconds.
        /// \param Elapsed The share of its life the particle has lived, from zero to one, which grows the push to full.
        /// \param Jitter  The particle's own roll of the dice, from zero to one, which sets the phase it wanders on.
        /// \return The offset to lay over where the particle stands, along the ground axes alone so it never lifts it.
        ZY_INLINE Vector3 Wander(Real32 Age, Real32 Elapsed, Real32 Jitter) const
        {
            // Still air pushes nothing, so the phase is never turned.
            if (mWander.GetX() == 0.0f)
            {
                return Vector3::Zero();
            }

            const Real32 Phase = Age * mWander.GetY() + Jitter * kTwoPI<Real32>;
            return Vector3(Sine(Phase), 0.0f, Sine(Phase * 0.73f + Jitter * 5.0f)) * (mWander.GetX() * Elapsed);
        }

        /// \brief Sets how far a particle is turned at birth, and how fast it turns from there.
        ///
        /// \param Spin The widest start in `X` and fastest turn in `Y`, in degrees and degrees a second, either way.
        ZY_INLINE void SetSpin(Vector2 Spin)
        {
            mSpin = Vector2::Max(Spin, Vector2::Zero());
        }

        /// \brief Gets how far a particle is turned at birth, and how fast it turns from there.
        ///
        /// \return The widest start in `X` and fastest turn in `Y`, in degrees and degrees a second, either way.
        ZY_INLINE Vector2 GetSpin() const
        {
            return mSpin;
        }

        /// \brief Gets how far a particle has turned on its card.
        ///
        /// \param Age    The time the particle has been alive, in seconds.
        /// \param Jitter The particle's own roll of the dice, from zero to one, which picks its start and its turn.
        /// \return The turn, which is none for an emitter that never spins.
        ZY_INLINE Angle Turn(Real32 Age, Real32 Jitter) const
        {
            const Real32 Pick  = Jitter * 61.7f;
            const Real32 Start = mSpin.GetX() * (Jitter * 2.0f - 1.0f);
            const Real32 Rate  = mSpin.GetY() * ((Pick - Floor(Pick)) * 2.0f - 1.0f);

            return Angle::FromDegrees(Start + Rate * Age);
        }

        /// \brief Sets how big a particle is drawn over its life.
        ///
        /// \param Scale The size at birth in `X` and at death in `Y`, in world units, below zero counting as zero.
        ZY_INLINE void SetScale(Vector2 Scale)
        {
            mScale = Vector2::Max(Scale, Vector2::Zero());
        }

        /// \brief Gets how big a particle is drawn over its life.
        ///
        /// \return The size at birth along `X` and at death along `Y`, in world units.
        ZY_INLINE Vector2 GetScale() const
        {
            return mScale;
        }

        /// \brief Sets how much of its life a particle takes to grow to its size at birth, from nothing.
        ///
        /// \param Growth The share of its life, from zero to be born full size to `0.9`, where it reaches it late.
        ZY_INLINE void SetGrowth(Real32 Growth)
        {
            mGrowth = Clamp(Growth, 0.0f, 0.9f);
        }

        /// \brief Gets how much of its life a particle takes to grow to its size at birth, from nothing.
        ///
        /// \return The share of its life, where zero is born full size.
        ZY_INLINE Real32 GetGrowth() const
        {
            return mGrowth;
        }

        /// \brief Gets how big a particle is drawn at a point in its life.
        ///
        /// \param Elapsed The point in the particle's life to measure at, from zero to one.
        /// \return The size, in world units, grown from nothing and then eased from its size at birth to its death.
        ZY_INLINE Real32 Size(Real32 Elapsed) const
        {
            if (Elapsed < mGrowth)
            {
                return mScale.GetX() * (Elapsed / mGrowth);
            }
            return Lerp(mScale.GetX(), mScale.GetY(), (Elapsed - mGrowth) / (1.0f - mGrowth));
        }

        /// \brief Sets how much of the wind carries a particle.
        ///
        /// \param Wind The share of the wind that carries it, from never at zero to with the air at one.
        ZY_INLINE void SetWind(Real32 Wind)
        {
            mWind = Max(Wind, 0.0f);
        }

        /// \brief Gets how much of the wind carries a particle.
        ///
        /// \return The share of the wind that carries it, from never at zero to with the air at one.
        ZY_INLINE Real32 GetWind() const
        {
            return mWind;
        }

        /// \brief Sets how far a particle's card is stretched along the way it travels.
        ///
        /// \param Length The seconds of travel the card covers behind the particle, where zero keeps it square.
        ZY_INLINE void SetLength(Real32 Length)
        {
            mLength = Max(Length, 0.0f);
        }

        /// \brief Gets how far a particle's card is stretched along the way it travels.
        ///
        /// \return The seconds of travel the card covers behind the particle, where zero keeps it square.
        ZY_INLINE Real32 GetLength() const
        {
            return mLength;
        }

        /// \brief Sets what particles are let go from, which also decides how they are made.
        ///
        /// \param Shape The shape, where a point throws them and any other fills itself with them.
        ZY_INLINE void SetShape(Shape Shape)
        {
            mShape = Shape;
        }

        /// \brief Gets what particles are let go from.
        ///
        /// \return The shape, where a point throws them and any other fills itself with them.
        ZY_INLINE Shape GetShape() const
        {
            return mShape;
        }

        /// \brief Checks whether the emitter fills a shape rather than throwing from a point.
        ///
        /// \return `true` when it fills a shape, `false` when it throws from the origin.
        ZY_INLINE Bool IsArea() const
        {
            return mShape != Shape::Point;
        }

        /// \brief Sets how much particles differ from one another in size.
        ///
        /// \param Variance The fraction a particle may be larger or smaller by, where zero makes them alike.
        ZY_INLINE void SetVariance(Real32 Variance)
        {
            mVariance = Clamp(Variance, 0.0f, 1.0f);
        }

        /// \brief Gets how much particles differ from one another in size.
        ///
        /// \return The fraction a particle may be larger or smaller by.
        ZY_INLINE Real32 GetVariance() const
        {
            return mVariance;
        }

        /// \brief Sets the colours a particle passes through over its life.
        ///
        /// \param Gradient The stops to wear, in ascending order of time.
        ZY_INLINE void SetGradient(AnyRef<Gradient> Gradient)
        {
            mGradient = Move(Gradient);
        }

        /// \brief Gets the colours a particle passes through over its life.
        ///
        /// \return The stops worn, in ascending order of time.
        ZY_INLINE ConstRef<Gradient> GetGradient() const
        {
            return mGradient;
        }

        /// \brief Gets the colour a particle wears at a point in its life.
        ///
        /// \param Elapsed The point in the particle's life to sample at, from zero to one.
        /// \return The colour blended between the stops either side of that point, white where none are authored.
        ZY_INLINE IntColor8 Sample(Real32 Elapsed) const
        {
            return mGradient.Sample(Elapsed, IntColor8::White());
        }

        /// \brief Sets how the emitter's crop is divided into the variants a particle picks between.
        ///
        /// \param Variants The number of variants across the crop and down it.
        ZY_INLINE void SetVariants(IntVector2 Variants)
        {
            mVariants = IntVector2(Max(Variants.GetX(), 1), Max(Variants.GetY(), 1));
        }

        /// \brief Gets how the emitter's crop is divided into variants.
        ///
        /// \return The number of variants across the crop and down it.
        ZY_INLINE IntVector2 GetVariants() const
        {
            return mVariants;
        }

        /// \brief Sets whether a particle plays every variant in turn over its life, as the frames of a flipbook.
        ///
        /// \param Animated `true` to play them across and down the grid, `false` to keep the one it was born with.
        ZY_INLINE void SetAnimated(Bool Animated)
        {
            mAnimated = Animated;
        }

        /// \brief Checks whether a particle plays every variant in turn over its life.
        ///
        /// \return `true` when it plays them as a flipbook, `false` when it keeps the one it was born with.
        ZY_INLINE Bool IsAnimated() const
        {
            return mAnimated;
        }

        /// \brief Gets the size one variant covers, an equal cut of the crop the whole emitter was given.
        ///
        /// \param Source The crop the art is cut from, which may differ from the emitter's own once it is measured.
        /// \return The width and height one variant covers, in normalized texture coordinates.
        ZY_INLINE Vector2 Divide(Rect Source) const
        {
            return Vector2(Source.GetWidth() / mVariants.GetX(), Source.GetHeight() / mVariants.GetY());
        }

        /// \brief Gets the crop one variant covers, within the crop the whole emitter was given.
        ///
        /// \param Source  The crop the art is cut from, as \ref Divide was given it.
        /// \param Cell    The size one variant covers, as \ref Divide measures it from the same crop.
        /// \param Variant The variant to read, which wraps around the grid the emitter was authored with.
        /// \param Elapsed The share of its life lived, from zero to one, which an animated particle plays by.
        /// \return The crop that one variant covers.
        ZY_INLINE Rect Crop(Rect Source, Vector2 Cell, UInt8 Variant, Real32 Elapsed) const
        {
            const SInt32 Columns = mVariants.GetX();
            const SInt32 Count   = Columns * mVariants.GetY();
            const SInt32 Index   = mAnimated ? Min(static_cast<SInt32>(Elapsed * Count), Count - 1) : Variant % Count;

            const Real32 MinX    = Source.GetMinimumX() + (Index % Columns) * Cell.GetX();
            const Real32 MinY    = Source.GetMinimumY() + (Index / Columns) * Cell.GetY();

            return Rect(MinX, MinY, MinX + Cell.GetX(), MinY + Cell.GetY());
        }

        /// \brief Sets how many particles the emitter may hold at once.
        ///
        /// \param Limit The cap, which the emitter clamps to \ref kMaxParticles.
        ZY_INLINE void SetLimit(UInt16 Limit)
        {
            mLimit = Min(Limit, kMaxParticles);
        }

        /// \brief Gets how many particles the emitter may hold at once.
        ///
        /// \return The cap on live particles.
        ZY_INLINE UInt16 GetLimit() const
        {
            return mLimit;
        }

        /// \brief Sets the pair of directions a particle spans.
        ///
        /// \param Span The directions the art is laid along.
        ZY_INLINE void SetSpan(Span Span)
        {
            mSpan = Span;
        }

        /// \brief Gets the pair of directions a particle spans.
        ///
        /// \return The directions the art is laid along.
        ZY_INLINE Span GetSpan() const
        {
            return mSpan;
        }

        /// \brief Sets what a particle does where it comes back down to the height it was thrown from.
        ///
        /// \param Contact The way to meet that height.
        ZY_INLINE void SetContact(Contact Contact)
        {
            mContact = Contact;
        }

        /// \brief Gets what a particle does where it comes back down to the height it was thrown from.
        ///
        /// \return The way that height is met.
        ZY_INLINE Contact GetContact() const
        {
            return mContact;
        }

        /// \brief Measures how far the swarm reaches from the emitter it is thrown by.
        ///
        /// \return The volume the swarm reaches into, about the emitter that throws it.
        Box Measure() const;

        /// \brief Gets how many particles a steady stream holds alive at once, each one thrown again as it dies.
        ///
        /// \param Ground The ground a filled shape covers in square world units, or one for a point.
        /// \return The particles alive at once, the rate over the ground times the mean life.
        ZY_INLINE Real32 GetPopulation(Real32 Ground) const
        {
            return mRate * Ground * (mLife.GetMinimum() + mLife.GetMaximum()) * 0.5f;
        }

        /// \brief Writes the emitter as the particle shader library reads it.
        ///
        /// \param Output The layout that receives the emitter.
        /// \param Frame  The frame the particles are thrown from, which places, stretches and turns them.
        /// \param Wind   The wind in world units per second, which carries the emitter's share of it.
        /// \param Time   The moment the particles stand at in seconds, kept small enough to stay exact as a float.
        /// \param Count  The particles drawn, each one reborn as it dies.
        /// \param Seed   The number every particle rolls from, which keeps two emitters apart.
        void Pack(Ref<GpuEmitterLayout> Output, ConstRef<Matrix4x3> Frame, Vector3 Wind, Real32 Time, UInt32 Count, UInt32 Seed) const;

        /// \brief Serializes the state of the object to or from the specified archive.
        ///
        /// \param Archive The archive to serialize the object with.
        template<typename Serializer>
        ZY_INLINE void Serialize(Serializer Archive)
        {
            Archive.Serialize(mDirection);
            Archive.Serialize(mGravity);
            Archive.Serialize(mGradient);
            Archive.Serialize(mSource);
            Archive.Serialize(mLife);
            Archive.Serialize(mSpeed);
            Archive.Serialize(mScale);
            Archive.Serialize(mWander);
            Archive.Serialize(mRate);
            Archive.Serialize(mSpread);
            Archive.Serialize(mDrag);
            Archive.Serialize(mVariance);
            Archive.Serialize(mBurst);
            Archive.Serialize(mLimit);
            Archive.Serialize(mVariants);
            Archive.Serialize(mSpan);
            Archive.Serialize(mContact);
            Archive.Serialize(mShape);
            Archive.Serialize(mWind);
            Archive.Serialize(mLength);
            Archive.Serialize(mSpin);
            Archive.Serialize(mGrowth);
            Archive.Serialize(mAnimated);
        }

    public:

        /// \brief Provides the reflected members of this type.
        ///
        /// \return The fields of the type, in the order they are shown.
        ZY_INLINE static constexpr auto OnDescribe()
        {
            return Array(
                ZyReflection::Field::Custom<&Emitter::mSource>("Source", "Render.Frame"),
                ZyReflection::Field::Property<&Emitter::GetSpan, &Emitter::SetSpan>("Span"),
                ZyReflection::Field::Property<&Emitter::GetShape, &Emitter::SetShape>("Shape"),
                ZyReflection::Field::Property<&Emitter::GetRate, &Emitter::SetRate>("Rate")
                    .Between(0.0f, 512.0f)
                    .Stepped(0.1f),
                ZyReflection::Field::Property<&Emitter::GetBurst, &Emitter::SetBurst>("Burst")
                    .Between(0.0f, kMaxParticles),
                ZyReflection::Field::Property<&Emitter::GetLimit, &Emitter::SetLimit>("Limit")
                    .Between(0.0f, kMaxParticles),
                ZyReflection::Field::Property<&Emitter::GetLife, &Emitter::SetLife>("Life"),
                ZyReflection::Field::Property<&Emitter::GetSpeed, &Emitter::SetSpeed>("Speed"),
                ZyReflection::Field::Property<&Emitter::GetScale, &Emitter::SetScale>("Scale"),
                ZyReflection::Field::Property<&Emitter::GetGrowth, &Emitter::SetGrowth>("Growth")
                    .Between(0.0f, 0.9f)
                    .Stepped(0.01f),
                ZyReflection::Field::Property<&Emitter::GetDirection, &Emitter::SetDirection>("Direction"),
                ZyReflection::Field::Property<&Emitter::GetContact, &Emitter::SetContact>("Contact"),
                ZyReflection::Field::Property<&Emitter::GetSpread, &Emitter::SetSpread>("Spread"),
                ZyReflection::Field::Property<&Emitter::GetGravity, &Emitter::SetGravity>("Gravity"),
                ZyReflection::Field::Property<&Emitter::GetWander, &Emitter::SetWander>("Wander"),
                ZyReflection::Field::Property<&Emitter::GetSpin, &Emitter::SetSpin>("Spin"),
                ZyReflection::Field::Property<&Emitter::GetVariance, &Emitter::SetVariance>("Variance")
                    .Between(0.0f, 1.0f)
                    .Stepped(0.01f),
                ZyReflection::Field::Property<&Emitter::GetVariants, &Emitter::SetVariants>("Variants"),
                ZyReflection::Field::Property<&Emitter::IsAnimated, &Emitter::SetAnimated>("Animated"),
                ZyReflection::Field::Property<&Emitter::GetDrag, &Emitter::SetDrag>("Drag")
                    .Between(0.0f, 16.0f)
                    .Stepped(0.01f),
                ZyReflection::Field::Property<&Emitter::GetWind, &Emitter::SetWind>("Wind")
                    .Between(0.0f, 4.0f)
                    .Stepped(0.01f),
                ZyReflection::Field::Property<&Emitter::GetLength, &Emitter::SetLength>("Length")
                    .Between(0.0f, 1.0f)
                    .Stepped(0.001f),
                ZyReflection::Field::Nested<&Emitter::GetGradient, &Emitter::SetGradient>("Gradient"));
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Vector3    mDirection;
        Vector3    mGravity;
        Gradient   mGradient;
        Rect       mSource;
        Range      mLife;
        Range      mSpeed;
        Vector2    mScale;
        Vector2    mWander;
        Vector2    mSpin;
        Real32     mRate;
        Angle      mSpread;
        Real32     mDrag;
        Real32     mVariance;
        Real32     mWind;
        Real32     mLength;
        Real32     mGrowth;
        UInt16     mBurst;
        UInt16     mLimit;
        IntVector2 mVariants;
        Span       mSpan;
        Contact    mContact;
        Shape      mShape;
        Bool       mAnimated;
    };
}