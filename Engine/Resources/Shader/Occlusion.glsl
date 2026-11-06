// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_OCCLUSION_INCLUDED
#define ZY_OCCLUSION_INCLUDED

#include "Embedded://Shader/Math.glsl"

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

/// Reach a texel holds where no surface stands, at or past which it is left open to the sky.
#define ZY_OCCLUSION_OPEN   1e6

/// Two noises, each laying all sixteen of its values once in every 4x4 block of pixels, so a blur that wide settles both.
vec2 ZyOcclusionNoise(vec2 Pixel)
{
    const float kBayer[16] = float[16](0.0, 8.0, 2.0, 10.0, 12.0, 4.0, 14.0, 6.0, 3.0, 11.0, 1.0, 9.0, 15.0, 7.0, 13.0, 5.0);

    uvec2 Cell = uvec2(Pixel) & 3u;
    float Turn = kBayer[Cell.y * 4u + Cell.x];
    float Step = kBayer[((Cell.x + 1u) & 3u) * 4u + ((Cell.y + 2u) & 3u)];

    return (vec2(Turn, Step) + 0.5) / 16.0;
}

/// The visibility occlusion leaves once light bouncing between surfaces is counted (Jimenez 2016), for an albedo.
///
/// \param Visible The share of the hemisphere a point sees, from 0 to 1.
/// \param Albedo  The share of light the surfaces about it reflect, from 0 to 1.
/// \return The visibility, never darker than it was.
float ZyOcclusionBounce(float Visible, float Albedo)
{
    float A = 2.0404 * Albedo - 0.3324;
    float B = 4.7951 * Albedo - 0.6417;
    float C = 2.7552 * Albedo + 0.6903;

    return max(Visible, ((Visible * A - B) * Visible + C) * Visible);
}

#ifdef ZY_OCCLUSION_READ

/// Ground-truth horizon occlusion (Jimenez 2016) about each point's own sight, read through ZY_OCCLUSION_READ(Pixel).
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
    vec3 Center, vec3 Normal, vec3 Viewer, vec2 Pixel, vec3 Across, vec3 Down, float Radius, vec2 Noise,
    out vec3 Bent)
{
    float Visible = 0.0;
    vec3  Open    = vec3(0.0);

    for (int Slice = 0; Slice < ZY_OCCLUSION_SLICES; ++Slice)
    {
        float  Turn   = (float(Slice) + Noise.x) * (ZY_PI / float(ZY_OCCLUSION_SLICES));
        vec2 Way    = vec2(cos(Turn), sin(Turn));
        vec3 Ground = Across * Way.x + Down * Way.y;
        vec2 Reach  = Way * (Radius / max(length(Ground), 1e-6));

        vec3 Side      = normalize(Ground - Viewer * dot(Ground, Viewer));
        vec3 Axis      = normalize(cross(Side, Viewer));
        vec3 Projected = Normal - Axis * dot(Normal, Axis);
        float  Length    = length(Projected);
        float  Facing    = ZySaturate(dot(Projected, Viewer) / max(Length, 1e-6));
        float  Lean      = sign(dot(Side, Projected)) * acos(Facing);

        float Highest0 = cos(Lean + ZY_HALF_PI);
        float Highest1 = cos(Lean - ZY_HALF_PI);

            for (int Step = 0; Step < ZY_OCCLUSION_STEPS; ++Step)
        {
            float  Along = ZySquare((float(Step) + Noise.y) / float(ZY_OCCLUSION_STEPS));
            vec3 Near  = ZY_OCCLUSION_READ(Pixel + Reach * Along) - Center;
            vec3 Far   = ZY_OCCLUSION_READ(Pixel - Reach * Along) - Center;
            float  Gap0  = length(Near);
            float  Gap1  = length(Far);

            float Weight0 = ZySaturate((Radius - Gap0) / (Radius * ZY_OCCLUSION_FADE)) * step(ZY_OCCLUSION_NEAR, Gap0);
            float Weight1 = ZySaturate((Radius - Gap1) / (Radius * ZY_OCCLUSION_FADE)) * step(ZY_OCCLUSION_NEAR, Gap1);

            Highest0 = max(Highest0, mix(cos(Lean + ZY_HALF_PI), dot(Near, Viewer) / max(Gap0, 1e-6), Weight0));
            Highest1 = max(Highest1, mix(cos(Lean - ZY_HALF_PI), dot(Far,  Viewer) / max(Gap1, 1e-6), Weight1));
        }

        float Horizon0 = Lean + max(-acos(clamp(Highest1, -1.0, 1.0)) - Lean, -ZY_HALF_PI);
        float Horizon1 = Lean + min( acos(clamp(Highest0, -1.0, 1.0)) - Lean,  ZY_HALF_PI);
        float Arc0     = (Facing + 2.0 * Horizon0 * sin(Lean) - cos(2.0 * Horizon0 - Lean)) * 0.25;
        float Arc1     = (Facing + 2.0 * Horizon1 * sin(Lean) - cos(2.0 * Horizon1 - Lean)) * 0.25;

        float Across0 = 6.0 * sin(Horizon0 - Lean) - sin(3.0 * Horizon0 - Lean);
        float Across1 = 6.0 * sin(Horizon1 - Lean) - sin(3.0 * Horizon1 - Lean);
        float Sideway = Across0 + Across1 + 16.0 * sin(Lean) - 3.0 * (sin(Horizon0 + Lean) + sin(Horizon1 + Lean));
        float Upward  = 8.0 * cos(Lean) - cos(3.0 * Horizon0 - Lean) - cos(3.0 * Horizon1 - Lean)
                            - 3.0 * (cos(Horizon0 + Lean) + cos(Horizon1 + Lean));

        Visible += Length * (Arc0 + Arc1);
        Open    += Length * (Side * Sideway + Viewer * Upward) / 12.0;
    }

    Bent = normalize(Open + Normal * 1e-6);
    return ZySaturate(Visible / float(ZY_OCCLUSION_SLICES));
}

#endif // ZY_OCCLUSION_READ

#endif // ZY_OCCLUSION_INCLUDED