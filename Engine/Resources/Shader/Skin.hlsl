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

/// Blends a vertex's four bones by weight, from the shader's `u_Palette` of three rows per bone.
ZyAffine ZySkin(uint4 Joints, float4 Weights)
{
    float4 Row0 = float4(0.0, 0.0, 0.0, 0.0);
    float4 Row1 = float4(0.0, 0.0, 0.0, 0.0);
    float4 Row2 = float4(0.0, 0.0, 0.0, 0.0);

    [unroll]
    for (int Slot = 0; Slot < 4; ++Slot)
    {
        const uint Row = Joints[Slot] * 3u;

        Row0 += Weights[Slot] * u_Palette[Row];
        Row1 += Weights[Slot] * u_Palette[Row + 1u];
        Row2 += Weights[Slot] * u_Palette[Row + 2u];
    }
    return ZyReadAffine(Row0, Row1, Row2);
}

#endif // ZY_SKIN_INCLUDED