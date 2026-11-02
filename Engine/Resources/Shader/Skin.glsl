// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_SKIN_INCLUDED
#define ZY_SKIN_INCLUDED

#include "Embedded://Shader/Affine.glsl"

/// The texels one line of a palette texture holds, as a power of two.
#define ZY_SKIN_WIDTH_BITS 10u

/// Reads one row of a palette texture, its texels laid line after line.
vec4 ZySkinRow(highp sampler2D Palette, uint Texel)
{
    return texelFetch(Palette, ivec2(Texel & ((1u << ZY_SKIN_WIDTH_BITS) - 1u), Texel >> ZY_SKIN_WIDTH_BITS), 0);
}

/// Blends a vertex's four bones by weight, from a palette texture of three rows per bone starting at a texel.
ZyAffine ZySkin(highp sampler2D Palette, uint Base, uvec4 Joints, vec4 Weights)
{
    vec4 Row0 = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 Row1 = vec4(0.0, 0.0, 0.0, 0.0);
    vec4 Row2 = vec4(0.0, 0.0, 0.0, 0.0);

    for (int Slot = 0; Slot < 4; ++Slot)
    {
        uint Row = Base + Joints[Slot] * 3u;

        Row0 += Weights[Slot] * ZySkinRow(Palette, Row);
        Row1 += Weights[Slot] * ZySkinRow(Palette, Row + 1u);
        Row2 += Weights[Slot] * ZySkinRow(Palette, Row + 2u);
    }
    return ZyReadAffine(Row0, Row1, Row2);
}

#endif // ZY_SKIN_INCLUDED