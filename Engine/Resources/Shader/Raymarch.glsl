// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_RAYMARCH_INCLUDED
#define ZY_RAYMARCH_INCLUDED

/// Henyey-Greenstein phase, scaled so an even medium reads one.
float ZyRaymarchPhase(float Cosine, float Forward)
{
    float Facing = 1.0 + Forward * Forward - 2.0 * Forward * Cosine;

    return (1.0 - Forward * Forward) / (Facing * sqrt(Facing));
}

/// One over a ray direction, each axis kept off zero for box clipping.
vec3 ZyRaymarchInverse(vec3 Sight)
{
    return 1.0 / mix(vec3(1e-6), Sight, greaterThan(abs(Sight), vec3(1e-6)));
}

/// Clips a ray to a box, giving where it enters and leaves; false if it misses before Reach.
bool ZyRaymarchClip(vec3 Start, vec3 Inverse, vec3 Least, vec3 Most, float Reach, out float Enter, out float Leave)
{
    vec3 Low     = (Least - Start) * Inverse;
    vec3 High    = (Most  - Start) * Inverse;
    vec3 Nearer  = min(Low, High);
    vec3 Further = max(Low, High);

    Enter = max(max(max(Nearer.x, Nearer.y), Nearer.z), 0.0);
    Leave = min(min(min(Further.x, Further.y), Further.z), Reach);

    return Leave > Enter;
}

/// Clips a ray to an ellipsoid, giving where it enters and leaves; false if it misses before Reach.
bool ZyRaymarchEllipsoid(vec3 Start, vec3 Sight, vec3 Middle, vec3 Radii, float Reach, out float Enter, out float Leave)
{
    vec3  Origin = (Start - Middle) / Radii;
    vec3  Way    = Sight / Radii;
    float Square = dot(Way, Way);
    float Along  = dot(Origin, Way);
    float Spread = Along * Along - Square * (dot(Origin, Origin) - 1.0);
    float Root   = sqrt(max(Spread, 0.0));

    Enter = max((-Along - Root) / Square, 0.0);
    Leave = min((-Along + Root) / Square, Reach);

    return Spread > 0.0 && Leave > Enter;
}

/// Clips a ray to an upright elliptic cylinder of any height, giving where it enters and leaves.
bool ZyRaymarchCylinder(vec3 Start, vec3 Sight, vec2 Middle, vec2 Radii, float Reach, out float Enter, out float Leave)
{
    vec2  Origin = (Start.xz - Middle) / Radii;
    vec2  Way    = Sight.xz / Radii;
    float Square = dot(Way, Way);
    float Along  = dot(Origin, Way);
    float Inside = dot(Origin, Origin) - 1.0;

    // A ray running up the axis is inside for its whole length or never.
    if (Square < 1e-8)
    {
        Enter = 0.0;
        Leave = Reach;
        return Inside < 0.0;
    }

    float Spread = Along * Along - Square * Inside;
    float Root   = sqrt(max(Spread, 0.0));

    Enter = max((-Along - Root) / Square, 0.0);
    Leave = min((-Along + Root) / Square, Reach);

    return Spread > 0.0 && Leave > Enter;
}

/// Billow noise from a stack of lattice slices, eased across a slice and between slices.
float ZyRaymarchLattice(sampler2DArray Lattice, vec2 Plane, float Height, float Texel, float Slices)
{
    vec2  Cell   = floor(Plane);
    vec2  Offset = Plane - Cell;
    vec2  Uv     = (Cell + Offset * Offset * (3.0 - 2.0 * Offset) + 0.5) * Texel;
    float Layer  = floor(Height);
    float Rise   = Height - Layer;
    float Lower  = Layer - Slices * floor(Layer / Slices);
    float Upper  = (Lower + 1.0 < Slices) ? Lower + 1.0 : 0.0;

    return mix(textureLod(Lattice, vec3(Uv, Lower), 0.0).r,
               textureLod(Lattice, vec3(Uv, Upper), 0.0).r, Rise * Rise * (3.0 - 2.0 * Rise));
}

/// The first full-size texel of the 2x2 block a half-size texel covers.
ivec2 ZyRaymarchBlock(vec2 Pixel, vec2 Full)
{
    return ivec2(Pixel * (Full / max(floor(Full * 0.5), 1.0)) - 0.5);
}

/// The depth a half-size texel keeps, farthest on a checkerboard and nearest elsewhere.
float ZyRaymarchMeasure(vec2 Pixel, vec4 Reaches)
{
    bool Far = ((uint(Pixel.x) + uint(Pixel.y)) & 1u) == 0u;

    return Far ? max(max(Reaches.x, Reaches.y), max(Reaches.z, Reaches.w))
               : min(min(Reaches.x, Reaches.y), min(Reaches.z, Reaches.w));
}

/// Depth-aware upsample of a half-size medium onto a full-size pixel.
vec4 ZyRaymarchSettle(sampler2D Medium, sampler2D Reaches, vec2 Pixel, vec2 Full, float Reach, float Reject)
{
    ivec2 Half   = textureSize(Medium, 0);
    vec2  At     = Pixel * (vec2(Half) / Full) - 0.5;
    vec2  Corner = floor(At);
    vec2  Blend  = At - Corner;
    ivec2 Last   = Half - 1;

#ifdef GL_ARB_texture_gather
    vec4 Gaps = abs(textureGather(Reaches, (Corner + 1.0) / vec2(Half)).wzxy - Reach);
#endif

    vec4  Sum     = vec4(0.0);
    float Total   = 0.0;
    float Nearest = 1e30;
    vec4  Closest = vec4(0.0);

    for (int Index = 0; Index < 4; ++Index)
    {
        ivec2 Offset = ivec2(Index & 1, Index >> 1);
        ivec2 Tap    = clamp(ivec2(Corner) + Offset, ivec2(0), Last);
        vec4  Light  = texelFetch(Medium, Tap, 0);
#ifdef GL_ARB_texture_gather
        float Gap    = Gaps[Index];
#else
        float Gap    = abs(texelFetch(Reaches, Tap, 0).r - Reach);
#endif
        vec2  Share  = mix(1.0 - Blend, Blend, greaterThan(Offset, ivec2(0)));
        float Weight = Share.x * Share.y * clamp(1.0 - Gap * Reject, 0.0, 1.0);

        Sum   += Light * Weight;
        Total += Weight;

        if (Gap < Nearest)
        {
            Nearest = Gap;
            Closest = Light;
        }
    }
    return (Total > 1e-4) ? Sum / Total : Closest;
}

#endif // ZY_RAYMARCH_INCLUDED