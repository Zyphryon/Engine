// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PBR_INCLUDED
#define ZY_PBR_INCLUDED

#include "Embedded://Shader/Math.glsl"

/// The least roughness a surface is shaded at, since a perfect mirror turns a light into one burning texel.
#define ZY_ROUGHNESS_FLOOR 0.045

/// The share of the light a white metal's single bounce misses, as 1/E - 1, a cubic in the floored roughness (rows)
/// and the root of NoV (columns), fitted to an importance-sampled reference of the BRDF below, within 3% over NoV 0.05.
#define ZY_BOUNCES_0 vec4( 0.11915,  -1.15002,   2.39832,  -1.44896)
#define ZY_BOUNCES_1 vec4( 0.21413,   7.86972, -22.92279,  16.19535)
#define ZY_BOUNCES_2 vec4(-2.42695,  -6.08632,  37.69816, -34.34492)
#define ZY_BOUNCES_3 vec4( 2.22216,  -0.83850, -14.39541,  19.10697)

/// Returns the colour a surface scatters, which a metal has none of.
vec3 ZyDiffuse(vec3 Albedo, float Metalness)
{
    return Albedo * (1.0 - Metalness);
}

/// Returns the colour a surface reflects head on, which is its own colour where it is a metal.
vec3 ZySpecular(vec3 Albedo, float Metalness, float Reflectance)
{
    return mix(vec3(Reflectance), Albedo, Metalness);
}

/// Returns how many of the surface's microfacets face halfway between the light and the eye, after GGX.
float ZyDistribution(float NoH, float Alpha)
{
    float Square = Alpha * Alpha;
    float Lean   = NoH * NoH * (Square - 1.0) + 1.0;

    return Square / (ZY_PI * Lean * Lean);
}

/// Returns how much of the light the microfacets shade and mask from one another, after the height-correlated Smith.
float ZyVisibility(float NoV, float NoL, float Alpha)
{
    float Square = Alpha * Alpha;
    float View   = NoL * sqrt(NoV * NoV * (1.0 - Square) + Square);
    float Light  = NoV * sqrt(NoL * NoL * (1.0 - Square) + Square);

    return 0.5 / max(View + Light, ZY_EPSILON);
}

/// Returns how much more a surface reflects as the light grazes it, after Schlick.
vec3 ZyFresnel(vec3 Specular, float VoH)
{
    return Specular + (1.0 - Specular) * pow(1.0 - VoH, 5.0);
}

/// Returns what a surface reflects of light arriving from every side, after Karis' fit of the split-sum lookup.
vec3 ZyEnvironment(vec3 Specular, float Roughness, float NoV)
{
    vec4 Low  = vec4(-1.0, -0.0275, -0.572,  0.022);
    vec4 High = vec4( 1.0,  0.0425,  1.040, -0.040);

    vec4  Fit   = Roughness * Low + High;
    float Bias  = min(Fit.x * Fit.x, exp2(-9.28 * NoV)) * Fit.x + Fit.y;
    vec2  Scale = vec2(-1.04, 1.04) * Bias + Fit.zw;

    return Specular * Scale.x + Scale.y;
}

/// Returns the share of the light a white metal's single bounce misses, which the bounces GGX leaves out carry.
float ZyBounces(float Roughness, float NoV)
{
    float Rough = max(Roughness, ZY_ROUGHNESS_FLOOR);
    float Root  = sqrt(max(NoV, 0.05));
    vec4  Along = vec4(1.0, Root, Root * Root, Root * Root * Root);
    vec4  Terms = vec4(
        dot(ZY_BOUNCES_0, Along), dot(ZY_BOUNCES_1, Along), dot(ZY_BOUNCES_2, Along), dot(ZY_BOUNCES_3, Along));

    return max(dot(vec4(1.0, Rough, Rough * Rough, Rough * Rough * Rough), Terms), 0.0);
}

/// \brief Holds what the BRDF of a point seen from one way shares between every light that reaches it.
struct ZyLobe
{
    float NoV;         ///< The cosine between the normal and the way to the eye, held above zero.
    float Alpha;       ///< The roughness past its floor, squared.
    vec3  Environment; ///< The share the surface reflects of light arriving from every side.
    vec3  Scatter;     ///< The colour the diffuse sends back per unit of light, less what the specular took.
    vec3  Gain;        ///< The factor the single bounce is scaled by to carry the bounces GGX leaves out.
};

/// Returns what the BRDF of a point seen from one way shares between every light that reaches it.
ZyLobe ZyPrepareLobe(vec3 Normal, vec3 Sight, vec3 Diffuse, vec3 Specular, float Roughness)
{
    ZyLobe Result;
    Result.NoV         = max(dot(Normal, Sight), ZY_EPSILON);
    Result.Alpha       = ZySquare(max(Roughness, ZY_ROUGHNESS_FLOOR));
    Result.Environment = ZyEnvironment(Specular, Roughness, Result.NoV);

    // What the specular sends towards the eye is light the diffuse never gets to scatter.
    Result.Scatter     = Diffuse * ZY_INV_PI * (1.0 - Result.Environment);

    // GGX follows one bounce among the microfacets, so what a rough surface loses to the others is put back.
    Result.Gain        = 1.0 + Specular * ZyBounces(Roughness, Result.NoV);
    return Result;
}

/// Returns what a surface sends towards the eye from a light of an angular radius, in radians, per unit of the light
/// arriving along its normal, the part every light shares already in its lobe.
vec3 ZyBrdf(ZyLobe Lobe, vec3 Normal, vec3 Sight, vec3 Incident, vec3 Specular, float Spread)
{
    vec3  Halfway = normalize(Sight + Incident);
    float NoL     = clamp(dot(Normal, Incident), 0.0, 1.0);
    float NoH     = clamp(dot(Normal, Halfway), 0.0, 1.0);
    float VoH     = clamp(dot(Sight, Halfway), 0.0, 1.0);

    // A light of some size turns the half vector across half its angle, which widens the lobe it lands in, the two
    // spreads adding in quadrature so the distribution keeps its whole energy.
    float Widened = sqrt(Lobe.Alpha * Lobe.Alpha + ZySquare(Spread * 0.5));

    vec3 Fresnel = ZyFresnel(Specular, VoH);
    vec3 Single  = Fresnel * (ZyDistribution(NoH, Widened) * ZyVisibility(Lobe.NoV, NoL, Lobe.Alpha));

    return (Lobe.Scatter + Single * Lobe.Gain) * NoL;
}

/// Returns what a surface sends towards the eye from a light of an angular radius, in radians, per unit of the light
/// arriving along its normal.
vec3 ZyBrdf(vec3 Normal, vec3 Sight, vec3 Incident, vec3 Diffuse, vec3 Specular, float Roughness, float Spread)
{
    ZyLobe Lobe = ZyPrepareLobe(Normal, Sight, Diffuse, Specular, Roughness);

    return ZyBrdf(Lobe, Normal, Sight, Incident, Specular, Spread);
}

/// Returns how much of the light from every side an occluded surface still reflects, after Lagarde, which the
/// cavity takes less of the more the surface faces the eye and the smoother it is.
float ZyOcclusion(float NoV, float Occlusion, float Roughness)
{
    return ZySaturate(pow(NoV + Occlusion, exp2(-16.0 * Roughness - 1.0)) - 1.0 + Occlusion);
}

/// Returns how much of a light reaches a point, whole within its core, falling with the square past it and windowed
/// down to nothing at its reach.
float ZyFalloff(float Distance, float Radius, float Core)
{
    float Share  = Distance / max(Radius, ZY_EPSILON);
    float Window = ZySquare(clamp(1.0 - ZySquare(ZySquare(Share)), 0.0, 1.0));

    return Window * (Core * Core) / max(Distance * Distance, Core * Core);
}

/// Returns how much of a cone reaches a point, whole within its inner angle and faded out by its outer one.
float ZyCone(vec3 Axis, vec3 Toward, float Outer, float Inner)
{
    return smoothstep(Outer, Inner, dot(Axis, Toward));
}

/// Returns what the sky and the ground lay on a direction, the sky above and the ground below.
vec3 ZyHemisphere(vec3 Direction, vec3 Sky, vec3 Ground)
{
    return mix(Ground, Sky, Direction.y * 0.5 + 0.5);
}

#endif // ZY_PBR_INCLUDED