// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_GRID_INCLUDED
#define ZY_GRID_INCLUDED

/// \brief Measures how much of a pixel the nearest line of a lattice covers, one pixel wide and softened at its edge.
///
/// \note The line is measured through screen derivatives, so this belongs to the fragment stage alone.
///
/// \param Repeat The place along the lattice, counted in periods, so that a line falls on every whole number.
///
/// \return The coverage, over zero between lines through one on a line.
float ZyGridLine(float Repeat)
{
    float Derivate = max(fwidth(Repeat), 1e-8);
    float Distance = abs(fract(Repeat - 0.5) - 0.5) / Derivate;
    float Density  = clamp(1.0 - Derivate * 2.0, 0.0, 1.0);

    return clamp(1.0 - Distance, 0.0, 1.0) * Density;
}

/// \brief Measures how much of a pixel the nearest line of a lattice on a plane covers, along either axis.
///
/// \note The line is measured through screen derivatives, so this belongs to the fragment stage alone.
///
/// \param Repeat The place on the lattice, counted in periods, so that a line falls on every whole number of either axis.
///
/// \return The coverage, over zero between lines through one on a line.
float ZyGridLine(vec2 Repeat)
{
    vec2  Derivate = max(fwidth(Repeat), vec2(1e-8));
    vec2  Distance = abs(fract(Repeat - 0.5) - 0.5) / Derivate;
    float Density  = clamp(1.0 - max(Derivate.x, Derivate.y) * 2.0, 0.0, 1.0);

    return clamp(1.0 - min(Distance.x, Distance.y), 0.0, 1.0) * Density;
}

#endif // ZY_GRID_INCLUDED