// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_GRID_INCLUDED
#define ZY_GRID_INCLUDED

/// How much of a pixel the nearest grid line covers, one pixel wide; fragment stage only.
float ZyGridLine(float Repeat)
{
    const float Derivate = max(fwidth(Repeat), 1e-8);
    const float Distance = abs(frac(Repeat - 0.5) - 0.5) / Derivate;
    const float Density  = saturate(1.0 - Derivate * 2.0);

    return saturate(1.0 - Distance) * Density;
}

/// How much of a pixel the nearest line of a 2D grid covers, on either axis; fragment stage only.
float ZyGridLine(float2 Repeat)
{
    const float2 Derivate = max(fwidth(Repeat), 1e-8);
    const float2 Distance = abs(frac(Repeat - 0.5) - 0.5) / Derivate;
    const float  Density  = saturate(1.0 - max(Derivate.x, Derivate.y) * 2.0);

    return saturate(1.0 - min(Distance.x, Distance.y)) * Density;
}

#endif // ZY_GRID_INCLUDED