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

#include "Emitter.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

namespace ZyRender
{
    /// \brief Represents the particles one emitter has thrown, carried forward on the CPU.
    class ZY_API Swarm final
    {
    public:

        /// \brief Represents one particle, holding only what carrying it forward needs.
        struct Particle final
        {
            /// The place the particle stands, in world units measured from the swarm's own origin.
            Vector3 Position;

            /// The velocity the particle travels at, in world units per second.
            Vector3 Velocity;

            /// The time the particle has been alive, in seconds.
            Real32  Age     = 0.0f;

            /// The time the particle lives for, in seconds.
            Real32  Life    = 0.0f;

            /// The height the emitter stood at when it was thrown, which is the ground it comes back down to.
            Real32  Floor   = 0.0f;

            /// The particle's own roll of the dice, which sizes and wanders it apart from its neighbours.
            UInt8   Seed    = 0;

            /// The variant of the emitter's crop the particle shows.
            UInt8   Variant = 0;

            /// `true` once the particle settled on its floor, `false` while it still flies.
            Bool    Landed  = false;

            /// \brief Gets the roll of the dice as the share it stands for.
            ///
            /// \return The roll, from nothing at zero to all of it at one.
            ZY_INLINE Real32 Jitter() const
            {
                return Seed * (1.0f / 255.0f);
            }
        };

    public:

        /// \brief Constructs a swarm holding no particles, armed to throw its emitter's burst.
        ZY_INLINE Swarm()
            : mRandom  { Random::Seeded() },
              mCarry   { Vector3::Zero() },
              mBacklog { 0.0f },
              mIgnited { false }
        {
        }

        /// \brief Gets how fast the wind carried every particle on the last step.
        ///
        /// \return The carry, in world units per second, which a streak leans with on top of each particle's own way.
        ZY_INLINE Vector3 GetCarry() const
        {
            return mCarry;
        }

        /// \brief Gets the particles the swarm is holding.
        ///
        /// \return The live particles, in no particular order.
        ZY_INLINE ConstSpan<Particle> GetParticles() const
        {
            return mParticles;
        }

        /// \brief Moves every particle by the same offset, which is how the origin they are measured from is moved.
        ///
        /// \param Offset The offset to move them by, in world units.
        void Shift(Vector3 Offset);

        /// \brief Carries every particle forward, retiring whatever has outlived itself.
        ///
        /// \param Emitter    The emitter the particles were thrown by.
        /// \param Worldspace The frame of the emitter, which turns and stretches everything it throws.
        /// \param Wind       The wind, in world units per second, which carries the emitter's share of it.
        /// \param Delta      The time to carry them forward by, in seconds.
        void Advance(ConstRef<Emitter> Emitter, ConstRef<Matrix4x3> Worldspace, Vector3 Wind, Real32 Delta);

        /// \brief Throws however many particles the emitter is owed for the time that has passed.
        ///
        /// \param Emitter    The emitter throwing them.
        /// \param Worldspace The frame of the emitter, which the throw is turned and stretched by.
        /// \param Presence   The share of its rate the emitter throws at, from none at zero to all of it at one.
        /// \param Delta      The time to throw for, in seconds.
        void Spawn(ConstRef<Emitter> Emitter, ConstRef<Matrix4x3> Worldspace, Real32 Presence, Real32 Delta);

        /// \brief Arms the swarm so its emitter's burst is thrown again on the next spawn, from a seed of its own.
        ZY_INLINE void Rearm()
        {
            mRandom  = Random::Seeded();
            mIgnited = false;
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Sequence<Particle> mParticles;
        Random             mRandom;
        Vector3            mCarry;
        Real32             mBacklog;
        Bool               mIgnited;
    };
}