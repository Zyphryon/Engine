// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_SKIN_INCLUDED
#define ZY_SKIN_INCLUDED

#include "Embedded://Shader/Affine.hlsl"

/// The texels one line of a palette texture holds, as a power of two.
#define ZY_SKIN_WIDTH_BITS 10u

/// Reads one row of a palette texture, its texels laid line after line.
float4 ZySkinRow(Texture2D<float4> Palette, uint Texel)
{
    return Palette.Load(int3(Texel & ((1u << ZY_SKIN_WIDTH_BITS) - 1u), Texel >> ZY_SKIN_WIDTH_BITS, 0));
}

/// Blends a vertex's four bones by weight, from a palette texture of three rows per bone starting at a texel.
ZyAffine ZySkin(Texture2D<float4> Palette, uint Base, uint4 Joints, float4 Weights)
{
    float4 Row0 = float4(0.0, 0.0, 0.0, 0.0);
    float4 Row1 = float4(0.0, 0.0, 0.0, 0.0);
    float4 Row2 = float4(0.0, 0.0, 0.0, 0.0);

    [unroll]
    for (int Slot = 0; Slot < 4; ++Slot)
    {
        const uint Row = Base + Joints[Slot] * 3u;

        Row0 += Weights[Slot] * ZySkinRow(Palette, Row);
        Row1 += Weights[Slot] * ZySkinRow(Palette, Row + 1u);
        Row2 += Weights[Slot] * ZySkinRow(Palette, Row + 2u);
    }
    return ZyReadAffine(Row0, Row1, Row2);
}

#endif // ZY_SKIN_INCLUDED