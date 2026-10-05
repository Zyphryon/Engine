// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_DEPTH_INCLUDED
#define ZY_DEPTH_INCLUDED

/// Turns a perspective depth-buffer value into distance from the eye.
float ZyLinearizeDepth(float Depth, float Near, float Far)
{
    return (Near * Far) / (Far - Depth * (Far - Near));
}

/// Distance from the eye, from 0 at the near plane to 1 at the far plane.
float ZyLinearizeDepth01(float Depth, float Near, float Far)
{
    return (ZyLinearizeDepth(Depth, Near, Far) - Near) / (Far - Near);
}

/// World distance the whole clip depth range covers.
float ZyDepthSpan(float4x4 Inverse)
{
    return length(mul(Inverse, float4(0.0, 0.0, 1.0, 0.0)).xyz);
}

/// Turns a depth-buffer value into clip-space depth.
float ZyClipDepth(float Depth)
{
    return Depth;
}

/// Rebuilds a position from a texture coordinate and depth, through an inverse matrix.
float3 ZyPositionFromDepth(float2 Uv, float Depth, float4x4 Inverse)
{
    const float4 Clip     = float4(Uv.x * 2.0 - 1.0, 1.0 - Uv.y * 2.0, ZyClipDepth(Depth), 1.0);
    const float4 Position = mul(Inverse, Clip);

    return Position.xyz / Position.w;
}

#endif // ZY_DEPTH_INCLUDED