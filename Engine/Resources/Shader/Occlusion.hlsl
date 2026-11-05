// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_OCCLUSION_INCLUDED
#define ZY_OCCLUSION_INCLUDED

#include "Embedded://Shader/Math.hlsl"

/// Directions about a pixel the horizons are searched along, each one slice of the hemisphere.
#ifndef ZY_OCCLUSION_SLICES
#define ZY_OCCLUSION_SLICES 2
#endif

/// Steps each side of a slice is searched in, out to the radius.
#ifndef ZY_OCCLUSION_STEPS
#define ZY_OCCLUSION_STEPS  6
#endif

/// Share of the radius a horizon fades out over, toward its far end.
#define ZY_OCCLUSION_FADE   0.615

/// Nearest a sample counts from the point, which a sample landing on the point itself never clears.
#define ZY_OCCLUSION_NEAR   1e-4

/// Two noises, each laying all sixteen of its values once in every 4x4 block of pixels, so a blur that wide settles both.
float2 ZyOcclusionNoise(float2 Pixel)
{
    static const float kBayer[16] = { 0, 8, 2, 10, 12, 4, 14, 6, 3, 11, 1, 9, 15, 7, 13, 5 };

    const uint2 Cell = uint2(Pixel) & 3u;
    const float Turn = kBayer[Cell.y * 4u + Cell.x];
    const float Step = kBayer[((Cell.x + 1u) & 3u) * 4u + ((Cell.y + 2u) & 3u)];

    return (float2(Turn, Step) + 0.5) / 16.0;
}

/// The visibility occlusion leaves once light bouncing between surfaces is counted (Jimenez 2016), for an albedo.
///
/// \param Visible The share of the hemisphere a point sees, from 0 to 1.
/// \param Albedo  The share of light the surfaces about it reflect, from 0 to 1.
/// \return The visibility, never darker than it was.
float ZyOcclusionBounce(float Visible, float Albedo)
{
    const float A = 2.0404 * Albedo - 0.3324;
    const float B = 4.7951 * Albedo - 0.6417;
    const float C = 2.7552 * Albedo + 0.6903;

    return max(Visible, ((Visible * A - B) * Visible + C) * Visible);
}

#ifdef ZY_OCCLUSION_READ

/// Ground-truth horizon occlusion (Jimenez 2016) under parallel lines of sight, read through ZY_OCCLUSION_READ(Pixel).
///
/// \param Center The position of the point, in the space ZY_OCCLUSION_READ returns.
/// \param Normal The way the point's surface faces, of unit length.
/// \param Viewer The way toward the camera, of unit length.
/// \param Pixel  The point's coordinate on the screen, in pixels.
/// \param Across The position step one pixel takes across the screen.
/// \param Down   The position step one pixel takes down the screen.
/// \param Radius The farthest a horizon is searched, in the units of the positions.
/// \param Noise  The pixel's two noises, from 0 to 1, which turn its slices and offset its steps.
/// \param Bent   Receives the way the open sky above the point lies on the whole, of unit length.
/// \return The share of the hemisphere the point sees, 1 for wholly open.
float ZyHorizonOcclusion(
    float3 Center, float3 Normal, float3 Viewer, float2 Pixel, float3 Across, float3 Down, float Radius, float2 Noise,
    out float3 Bent)
{
    float  Visible = 0.0;
    float3 Open    = 0.0;

    [unroll]
    for (int Slice = 0; Slice < ZY_OCCLUSION_SLICES; ++Slice)
    {
        const float  Turn   = (float(Slice) + Noise.x) * (ZY_PI / float(ZY_OCCLUSION_SLICES));
        const float2 Way    = float2(cos(Turn), sin(Turn));
        const float3 Ground = Across * Way.x + Down * Way.y;
        const float2 Reach  = Way * (Radius / max(length(Ground), 1e-6));

        const float3 Side      = normalize(Ground - Viewer * dot(Ground, Viewer));
        const float3 Axis      = normalize(cross(Side, Viewer));
        const float3 Projected = Normal - Axis * dot(Normal, Axis);
        const float  Length    = length(Projected);
        const float  Facing    = saturate(dot(Projected, Viewer) / max(Length, 1e-6));
        const float  Lean      = sign(dot(Side, Projected)) * acos(Facing);

        float Highest0 = cos(Lean + ZY_HALF_PI);
        float Highest1 = cos(Lean - ZY_HALF_PI);

        [unroll]
        for (int Step = 0; Step < ZY_OCCLUSION_STEPS; ++Step)
        {
            const float  Along = ZySquare((float(Step) + Noise.y) / float(ZY_OCCLUSION_STEPS));
            const float3 Near  = ZY_OCCLUSION_READ(Pixel + Reach * Along) - Center;
            const float3 Far   = ZY_OCCLUSION_READ(Pixel - Reach * Along) - Center;
            const float  Gap0  = length(Near);
            const float  Gap1  = length(Far);

            const float Weight0 = saturate((Radius - Gap0) / (Radius * ZY_OCCLUSION_FADE)) * step(ZY_OCCLUSION_NEAR, Gap0);
            const float Weight1 = saturate((Radius - Gap1) / (Radius * ZY_OCCLUSION_FADE)) * step(ZY_OCCLUSION_NEAR, Gap1);

            Highest0 = max(Highest0, lerp(cos(Lean + ZY_HALF_PI), dot(Near, Viewer) / max(Gap0, 1e-6), Weight0));
            Highest1 = max(Highest1, lerp(cos(Lean - ZY_HALF_PI), dot(Far,  Viewer) / max(Gap1, 1e-6), Weight1));
        }

        const float Horizon0 = Lean + max(-acos(clamp(Highest1, -1.0, 1.0)) - Lean, -ZY_HALF_PI);
        const float Horizon1 = Lean + min( acos(clamp(Highest0, -1.0, 1.0)) - Lean,  ZY_HALF_PI);
        const float Arc0     = (Facing + 2.0 * Horizon0 * sin(Lean) - cos(2.0 * Horizon0 - Lean)) * 0.25;
        const float Arc1     = (Facing + 2.0 * Horizon1 * sin(Lean) - cos(2.0 * Horizon1 - Lean)) * 0.25;

        const float Across0 = 6.0 * sin(Horizon0 - Lean) - sin(3.0 * Horizon0 - Lean);
        const float Across1 = 6.0 * sin(Horizon1 - Lean) - sin(3.0 * Horizon1 - Lean);
        const float Sideway = Across0 + Across1 + 16.0 * sin(Lean) - 3.0 * (sin(Horizon0 + Lean) + sin(Horizon1 + Lean));
        const float Upward  = 8.0 * cos(Lean) - cos(3.0 * Horizon0 - Lean) - cos(3.0 * Horizon1 - Lean)
                            - 3.0 * (cos(Horizon0 + Lean) + cos(Horizon1 + Lean));

        Visible += Length * (Arc0 + Arc1);
        Open    += Length * (Side * Sideway + Viewer * Upward) / 12.0;
    }

    Bent = normalize(Open + Normal * 1e-6);
    return saturate(Visible / float(ZY_OCCLUSION_SLICES));
}

#endif // ZY_OCCLUSION_READ

#endif // ZY_OCCLUSION_INCLUDED