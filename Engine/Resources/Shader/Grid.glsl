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
#ifdef FRAGMENT_SHADER
float ZyGridLine(float Repeat)
{
    float Derivate = max(fwidth(Repeat), 1e-8);
    float Distance = abs(fract(Repeat - 0.5) - 0.5) / Derivate;
    float Density  = clamp(1.0 - Derivate * 2.0, 0.0, 1.0);

    return clamp(1.0 - Distance, 0.0, 1.0) * Density;
}

/// How much of a pixel the nearest line of a 2D grid covers, on either axis; fragment stage only.
float ZyGridLine(vec2 Repeat)
{
    vec2  Derivate = max(fwidth(Repeat), vec2(1e-8));
    vec2  Distance = abs(fract(Repeat - 0.5) - 0.5) / Derivate;
    float Density  = clamp(1.0 - max(Derivate.x, Derivate.y) * 2.0, 0.0, 1.0);

    return clamp(1.0 - min(Distance.x, Distance.y), 0.0, 1.0) * Density;
}
#endif // FRAGMENT_SHADER

#endif // ZY_GRID_INCLUDED