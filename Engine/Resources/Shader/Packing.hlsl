// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PACKING_INCLUDED
#define ZY_PACKING_INCLUDED

/// Unpacks an RGBA8 color, red in the lowest byte.
float4 ZyUnpackTint(uint Packed)
{
    const uint4 Bytes = uint4(Packed, Packed >> 8u, Packed >> 16u, Packed >> 24u) & 0xFFu;

    return float4(Bytes) * (1.0 / 255.0);
}

/// Packs a color into RGBA8, the way ZyUnpackTint reads it.
uint ZyPackTint(float4 Color)
{
    const uint4 Bytes = uint4(saturate(Color) * 255.0 + 0.5);

    return Bytes.x | (Bytes.y << 8u) | (Bytes.z << 16u) | (Bytes.w << 24u);
}

/// Folds a unit direction onto an octahedron, as a point in -1..1.
float2 ZyEncodeOctahedral(float3 Normal)
{
    const float3 Projected = Normal / (abs(Normal.x) + abs(Normal.y) + abs(Normal.z));
    const float2 Wrapped   = (1.0 - abs(Projected.yx)) * (2.0 * step(0.0, Projected.xy) - 1.0);

    return (Projected.z >= 0.0) ? Projected.xy : Wrapped;
}

/// Unfolds an octahedral point back into a unit direction.
float3 ZyDecodeOctahedral(float2 Encoded)
{
    float3 Normal = float3(Encoded, 1.0 - abs(Encoded.x) - abs(Encoded.y));

    Normal.xy -= max(-Normal.z, 0.0) * (2.0 * step(0.0, Normal.xy) - 1.0);

    return normalize(Normal);
}

/// Encodes a tangent-space normal as a 0..1 normal map texel.
float3 ZyEncodeNormalMap(float3 Normal)
{
    return Normal * 0.5 + 0.5;
}

/// Decodes a 0..1 normal map texel into a tangent-space normal.
float3 ZyDecodeNormalMap(float3 Texel)
{
    return Texel * 2.0 - 1.0;
}

/// Decodes a two-channel normal map texel, rebuilding the third axis.
float3 ZyDecodeNormalMap(float2 Texel)
{
    const float2 Tangent = Texel * 2.0 - 1.0;

    return float3(Tangent, sqrt(max(1.0 - dot(Tangent, Tangent), 0.0)));
}

#endif // ZY_PACKING_INCLUDED