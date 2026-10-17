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

#include "Noise.hpp"

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// [   CODE   ]
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

inline namespace ZyMath
{
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Noise::Noise(UInt64 Seed)
        : mPermutation { }
    {
        for (UInt32 Index = 0; Index < 256; ++Index)
        {
            mPermutation[Index] = static_cast<UInt8>(Index);
        }

        // The order of the lattice hashes is the seed's alone, so one seed always lays out the same field.
        Random Shuffle(Seed);

        for (UInt32 Index = 255; Index > 0; --Index)
        {
            Swap(mPermutation[Index], mPermutation[Shuffle.NextInteger<UInt32>(0, Index)]);
        }
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Noise::Sample(Vector2 Point) const
    {
        // The point is skewed onto the lattice to find the triangle it falls in.
        const IntVector2 Cell = IntVector2(Vector2::Floor(Point + (Point.GetX() + Point.GetY()) * kSkew));
        const Vector2    Near = Point - (Vector2(Cell) - static_cast<Real32>(Cell.GetX() + Cell.GetY()) * kUnskew);

        // The middle corner is the one along the larger offset, which picks the upper or the lower triangle.
        const IntVector2 Step = (Near.GetX() > Near.GetY() ? IntVector2(1, 0) : IntVector2(0, 1));

        const Real32 Sum = Corner(Near, Hash(Cell))
                         + Corner(Near - Vector2(Step) + kUnskew, Hash(Cell + Step))
                         + Corner(Near - 1.0f + 2.0f * kUnskew, Hash(Cell + IntVector2(1)));
        return Clamp(kScale * Sum, -1.0f, 1.0f);
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Noise::Fractal(Vector2 Point, UInt32 Octaves, Real32 Lacunarity, Real32 Gain) const
    {
        Real32 Sum       = 0.0f;
        Real32 Total     = 0.0f;
        Real32 Amplitude = 1.0f;
        Real32 Frequency = 1.0f;

        for (UInt32 Octave = 0; Octave < Max(Octaves, 1u); ++Octave)
        {
            Sum       += Amplitude * Sample(Point * Frequency);
            Total     += Amplitude;
            Amplitude *= Gain;
            Frequency *= Lacunarity;
        }
        return Sum / Total;
    }

    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
    // -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

    Real32 Noise::Ridged(Vector2 Point, UInt32 Octaves, Real32 Lacunarity, Real32 Gain) const
    {
        Real32 Sum       = 0.0f;
        Real32 Total     = 0.0f;
        Real32 Amplitude = 1.0f;
        Real32 Frequency = 1.0f;

        for (UInt32 Octave = 0; Octave < Max(Octaves, 1u); ++Octave)
        {
            const Real32 Fold = 1.0f - Abs(Sample(Point * Frequency));

            Sum       += Amplitude * Fold * Fold;
            Total     += Amplitude;
            Amplitude *= Gain;
            Frequency *= Lacunarity;
        }
        return Sum / Total;
    }
}