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
    float Clip = Depth * 2.0 - 1.0;

    return (2.0 * Near * Far) / (Far + Near - Clip * (Far - Near));
}

/// Distance from the eye, from 0 at the near plane to 1 at the far plane.
float ZyLinearizeDepth01(float Depth, float Near, float Far)
{
    return (ZyLinearizeDepth(Depth, Near, Far) - Near) / (Far - Near);
}

/// World distance the whole clip depth range covers.
float ZyDepthSpan(mat4 Inverse)
{
    return length((Inverse * vec4(0.0, 0.0, 1.0, 0.0)).xyz);
}

/// Turns a depth-buffer value into clip-space depth.
float ZyClipDepth(float Depth)
{
    return Depth * 2.0 - 1.0;
}

/// Rebuilds a position from a texture coordinate and depth, through an inverse matrix.
vec3 ZyPositionFromDepth(vec2 Uv, float Depth, mat4 Inverse)
{
    vec4 Clip     = vec4(Uv * 2.0 - 1.0, ZyClipDepth(Depth), 1.0);
    vec4 Position = Inverse * Clip;

    return Position.xyz / Position.w;
}

#endif // ZY_DEPTH_INCLUDED