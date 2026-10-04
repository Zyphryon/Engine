// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_PBR_INCLUDED
#define ZY_PBR_INCLUDED

#include "Embedded://Shader/Math.hlsl"

/// The least roughness a surface is shaded at, since a perfect mirror turns a light into one burning texel.
#define ZY_ROUGHNESS_FLOOR 0.045

/// The share of the light a white metal's single bounce misses, as 1/E - 1, a cubic in the floored roughness (rows)
/// and the root of NoV (columns), fitted to an importance-sampled reference of the BRDF below, within 3% over NoV 0.05.
#define ZY_BOUNCES_0 float4( 0.11915,  -1.15002,   2.39832,  -1.44896)
#define ZY_BOUNCES_1 float4( 0.21413,   7.86972, -22.92279,  16.19535)
#define ZY_BOUNCES_2 float4(-2.42695,  -6.08632,  37.69816, -34.34492)
#define ZY_BOUNCES_3 float4( 2.22216,  -0.83850, -14.39541,  19.10697)

/// Returns the colour a surface scatters, which a metal has none of.
float3 ZyDiffuse(float3 Albedo, float Metalness)
{
    return Albedo * (1.0 - Metalness);
}

/// Returns the colour a surface reflects head on, which is its own colour where it is a metal.
float3 ZySpecular(float3 Albedo, float Metalness, float Reflectance)
{
    return lerp(float3(Reflectance, Reflectance, Reflectance), Albedo, Metalness);
}

/// Returns how many of the surface's microfacets face halfway between the light and the eye, after GGX.
float ZyDistribution(float NoH, float Alpha)
{
    const float Square = Alpha * Alpha;
    const float Lean   = NoH * NoH * (Square - 1.0) + 1.0;

    return Square / (ZY_PI * Lean * Lean);
}

/// Returns how much of the light the microfacets shade and mask from one another, after the height-correlated Smith.
float ZyVisibility(float NoV, float NoL, float Alpha)
{
    const float Square = Alpha * Alpha;
    const float View   = NoL * sqrt(NoV * NoV * (1.0 - Square) + Square);
    const float Light  = NoV * sqrt(NoL * NoL * (1.0 - Square) + Square);

    return 0.5 / max(View + Light, ZY_EPSILON);
}

/// Returns how much more a surface reflects as the light grazes it, after Schlick.
float3 ZyFresnel(float3 Specular, float VoH)
{
    return Specular + (1.0 - Specular) * pow(1.0 - VoH, 5.0);
}

/// Returns what a surface reflects of light arriving from every side, after Karis' fit of the split-sum lookup.
float3 ZyEnvironment(float3 Specular, float Roughness, float NoV)
{
    const float4 Low  = float4(-1.0, -0.0275, -0.572,  0.022);
    const float4 High = float4( 1.0,  0.0425,  1.040, -0.040);

    const float4 Fit   = Roughness * Low + High;
    const float  Bias  = min(Fit.x * Fit.x, exp2(-9.28 * NoV)) * Fit.x + Fit.y;
    const float2 Scale = float2(-1.04, 1.04) * Bias + Fit.zw;

    return Specular * Scale.x + Scale.y;
}

/// Returns the share of the light a white metal's single bounce misses, which the bounces GGX leaves out carry.
float ZyBounces(float Roughness, float NoV)
{
    const float  Rough = max(Roughness, ZY_ROUGHNESS_FLOOR);
    const float  Root  = sqrt(max(NoV, 0.05));
    const float4 Along = float4(1.0, Root, Root * Root, Root * Root * Root);
    const float4 Terms = float4(
        dot(ZY_BOUNCES_0, Along), dot(ZY_BOUNCES_1, Along), dot(ZY_BOUNCES_2, Along), dot(ZY_BOUNCES_3, Along));

    return max(dot(float4(1.0, Rough, Rough * Rough, Rough * Rough * Rough), Terms), 0.0);
}

/// \brief Holds what the BRDF of a point seen from one way shares between every light that reaches it.
struct ZyLobe
{
    float  NoV;         ///< The cosine between the normal and the way to the eye, held above zero.
    float  Alpha;       ///< The roughness past its floor, squared.
    float3 Environment; ///< The share the surface reflects of light arriving from every side.
    float3 Scatter;     ///< The colour the diffuse sends back per unit of light, less what the specular took.
    float3 Gain;        ///< The factor the single bounce is scaled by to carry the bounces GGX leaves out.
};

/// Returns what the BRDF of a point seen from one way shares between every light that reaches it.
ZyLobe ZyPrepareLobe(float3 Normal, float3 Sight, float3 Diffuse, float3 Specular, float Roughness)
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
float3 ZyBrdf(ZyLobe Lobe, float3 Normal, float3 Sight, float3 Incident, float3 Specular, float Spread)
{
    const float3 Halfway = normalize(Sight + Incident);
    const float  NoL     = saturate(dot(Normal, Incident));
    const float  NoH     = saturate(dot(Normal, Halfway));
    const float  VoH     = saturate(dot(Sight, Halfway));

    // A light of some size turns the half vector across half its angle, which widens the lobe it lands in, the two
    // spreads adding in quadrature so the distribution keeps its whole energy.
    const float  Widened = sqrt(Lobe.Alpha * Lobe.Alpha + ZySquare(Spread * 0.5));

    const float3 Fresnel = ZyFresnel(Specular, VoH);
    const float3 Single  = Fresnel * (ZyDistribution(NoH, Widened) * ZyVisibility(Lobe.NoV, NoL, Lobe.Alpha));

    return (Lobe.Scatter + Single * Lobe.Gain) * NoL;
}

/// Returns what a surface sends towards the eye from a light of an angular radius, in radians, per unit of the light
/// arriving along its normal.
float3 ZyBrdf(float3 Normal, float3 Sight, float3 Incident, float3 Diffuse, float3 Specular, float Roughness, float Spread)
{
    const ZyLobe Lobe = ZyPrepareLobe(Normal, Sight, Diffuse, Specular, Roughness);

    return ZyBrdf(Lobe, Normal, Sight, Incident, Specular, Spread);
}

/// Returns how much of the light from every side an occluded surface still reflects, after Lagarde, which the
/// cavity takes less of the more the surface faces the eye and the smoother it is.
float ZyOcclusion(float NoV, float Occlusion, float Roughness)
{
    return saturate(pow(NoV + Occlusion, exp2(-16.0 * Roughness - 1.0)) - 1.0 + Occlusion);
}

/// Returns how much of a light reaches a point, whole within its core, falling with the square past it and windowed
/// down to nothing at its reach.
float ZyFalloff(float Distance, float Radius, float Core)
{
    const float Share  = Distance / max(Radius, ZY_EPSILON);
    const float Window = ZySquare(saturate(1.0 - ZySquare(ZySquare(Share))));

    return Window * (Core * Core) / max(Distance * Distance, Core * Core);
}

/// Returns how much of a cone reaches a point, whole within its inner angle and faded out by its outer one.
float ZyCone(float3 Axis, float3 Toward, float Outer, float Inner)
{
    return smoothstep(Outer, Inner, dot(Axis, Toward));
}

/// Returns what the sky and the ground lay on a direction, the sky above and the ground below.
float3 ZyHemisphere(float3 Direction, float3 Sky, float3 Ground)
{
    return lerp(Ground, Sky, Direction.y * 0.5 + 0.5);
}

#endif // ZY_PBR_INCLUDED