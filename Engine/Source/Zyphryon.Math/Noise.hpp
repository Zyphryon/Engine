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

#include "Random.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    /// \brief Represents seeded two-dimensional simplex noise, the same field for the same seed on every run.
    class Noise final
    {
    public:

        /// \brief Constructs the noise field a seed names.
        ///
        /// \param Seed The seed, every value giving its own field.
        explicit Noise(UInt64 Seed = 0);

        /// \brief Samples the field at a point.
        ///
        /// \param Point The point, where one unit is about one feature of the field.
        /// \return The value, from -1 to 1.
        Real32 Sample(Vector2 Point) const;

        /// \brief Samples several octaves of the field, each finer and fainter than the one before.
        ///
        /// \param Point      The point, in units of the coarsest octave.
        /// \param Octaves    The number of octaves, from 1.
        /// \param Lacunarity The factor each octave's frequency grows by.
        /// \param Gain       The factor each octave's amplitude shrinks by.
        /// \return The value, from -1 to 1.
        Real32 Fractal(Vector2 Point, UInt32 Octaves, Real32 Lacunarity = 2.0f, Real32 Gain = 0.5f) const;

        /// \brief Samples several octaves of the field folded into ridges, sharp along the field's zero line.
        ///
        /// \param Point      The point, in units of the coarsest octave.
        /// \param Octaves    The number of octaves, from 1.
        /// \param Lacunarity The factor each octave's frequency grows by.
        /// \param Gain       The factor each octave's amplitude shrinks by.
        /// \return The value, from 0 in the valleys to 1 on the ridges.
        Real32 Ridged(Vector2 Point, UInt32 Octaves, Real32 Lacunarity = 2.0f, Real32 Gain = 0.5f) const;

    private:

        /// \brief The scale that brings the sum of the three corners to about -1 to 1.
        static constexpr Real32 kScale  = 70.0f;

        /// \brief The skew that maps the plane onto the simplex lattice, (√3 − 1) / 2.
        static constexpr Real32 kSkew   = 0.36602540378f;

        /// \brief The unskew that maps the simplex lattice back onto the plane, (3 − √3) / 6.
        static constexpr Real32 kUnskew = 0.21132486540f;

        /// \brief Gets what one corner of a triangle adds to a sample.
        ///
        /// \param Offset The offset of the sample from the corner.
        /// \param Hash   The hash of the corner, which picks its gradient.
        /// \return The corner's share, zero once the sample lies past its reach.
        ZY_INLINE static Real32 Corner(Vector2 Offset, UInt8 Hash)
        {
            static constexpr Array kGradients = Array(
                Vector2( 1.0f,  0.0f),
                Vector2(-1.0f,  0.0f),
                Vector2( 0.0f,  1.0f),
                Vector2( 0.0f, -1.0f),
                Vector2( 0.70710678f,  0.70710678f),
                Vector2(-0.70710678f,  0.70710678f),
                Vector2( 0.70710678f, -0.70710678f),
                Vector2(-0.70710678f, -0.70710678f));

            const Real32 Falloff = 0.5f - Offset.GetLengthSquared();

            if (Falloff > 0.0f)
            {
                const Real32 Squared = Falloff * Falloff;
                return Squared * Squared * Vector2::Dot(kGradients[Hash & 7], Offset);
            }
            return 0.0f;
        }

        /// \brief Gets the hash of a lattice corner, which picks the gradient it holds.
        ///
        /// \param Cell The corner, in lattice coordinates.
        /// \return The hash, from 0 to 255.
        ZY_INLINE UInt8 Hash(IntVector2 Cell) const
        {
            return mPermutation[(mPermutation[Cell.GetX() & 255] + Cell.GetY()) & 255];
        }

    private:

        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
        // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

        Array<UInt8, 256> mPermutation;
    };
}