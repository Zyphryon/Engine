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

/// The lowest roughness shaded, so a mirror never turns a light into one burning texel.
#define ZY_ROUGHNESS_FLOOR 0.045

/// Fit of what a white metal's single bounce misses (1/E - 1), by roughness and sqrt(NoV).
#define ZY_BOUNCES_0 float4( 0.11915,  -1.15002,   2.39832,  -1.44896)
#define ZY_BOUNCES_1 float4( 0.21413,   7.86972, -22.92279,  16.19535)
#define ZY_BOUNCES_2 float4(-2.42695,  -6.08632,  37.69816, -34.34492)
#define ZY_BOUNCES_3 float4( 2.22216,  -0.83850, -14.39541,  19.10697)

/// The diffuse colour, which a metal has none of.
float3 ZyDiffuse(float3 Albedo, float Metalness)
{
    return Albedo * (1.0 - Metalness);
}

/// The head-on specular colour, which is the albedo for a metal.
float3 ZySpecular(float3 Albedo, float Metalness, float Reflectance)
{
    return lerp(float3(Reflectance, Reflectance, Reflectance), Albedo, Metalness);
}

/// GGX normal distribution, from the lobe's width already squared.
float ZyDistributionSquare(float NoH, float Square)
{
    const float Lean = NoH * NoH * (Square - 1.0) + 1.0;

    return Square / (ZY_PI * Lean * Lean);
}

/// GGX normal distribution.
float ZyDistribution(float NoH, float Alpha)
{
    return ZyDistributionSquare(NoH, Alpha * Alpha);
}

/// One side's term of the height-correlated Smith visibility, by its cosine to the normal.
float ZySmith(float Cosine, float Alpha)
{
    const float Square = Alpha * Alpha;

    return sqrt(Cosine * Cosine * (1.0 - Square) + Square);
}

/// Height-correlated Smith visibility, with the view's term already worked out.
float ZyVisibility(float NoV, float NoL, float Alpha, float Facing)
{
    return 0.5 / max(NoL * Facing + NoV * ZySmith(NoL, Alpha), ZY_EPSILON);
}

/// Height-correlated Smith visibility.
float ZyVisibility(float NoV, float NoL, float Alpha)
{
    return ZyVisibility(NoV, NoL, Alpha, ZySmith(NoV, Alpha));
}

/// Schlick Fresnel.
float3 ZyFresnel(float3 Specular, float VoH)
{
    return Specular + (1.0 - Specular) * pow(1.0 - VoH, 5.0);
}

/// Reflectance under light from every side, after Karis' split-sum fit.
float3 ZyEnvironment(float3 Specular, float Roughness, float NoV)
{
    const float4 Low  = float4(-1.0, -0.0275, -0.572,  0.022);
    const float4 High = float4( 1.0,  0.0425,  1.040, -0.040);

    const float4 Fit   = Roughness * Low + High;
    const float  Bias  = min(Fit.x * Fit.x, exp2(-9.28 * NoV)) * Fit.x + Fit.y;
    const float2 Scale = float2(-1.04, 1.04) * Bias + Fit.zw;

    return Specular * Scale.x + Scale.y;
}

/// The energy GGX's single bounce misses on a white metal.
float ZyBounces(float Roughness, float NoV)
{
    const float  Rough = max(Roughness, ZY_ROUGHNESS_FLOOR);
    const float  Root  = sqrt(max(NoV, 0.05));
    const float4 Along = float4(1.0, Root, Root * Root, Root * Root * Root);
    const float4 Terms = float4(
        dot(ZY_BOUNCES_0, Along),
        dot(ZY_BOUNCES_1, Along),
        dot(ZY_BOUNCES_2, Along),
        dot(ZY_BOUNCES_3, Along));

    return max(dot(float4(1.0, Rough, Rough * Rough, Rough * Rough * Rough), Terms), 0.0);
}

/// Holds the BRDF terms every light on a point shares, for one view.
struct ZyLobe
{
    /// The cosine of normal and view, kept above zero.
    float  NoV;

    /// The floored roughness, squared.
    float  Alpha;

    /// The view's term of the height-correlated visibility, the same for every light.
    float  Facing;

    /// The reflectance under light from every side.
    float3 Environment;

    /// The diffuse per unit of light, less what the specular took.
    float3 Scatter;

    /// The boost to the single bounce for the bounces GGX leaves out.
    float3 Gain;
};

/// Builds the BRDF terms every light on a point shares.
ZyLobe ZyPrepareLobe(float3 Normal, float3 Sight, float3 Diffuse, float3 Specular, float Roughness)
{
    ZyLobe Result;
    Result.NoV         = max(dot(Normal, Sight), ZY_EPSILON);
    Result.Alpha       = ZySquare(max(Roughness, ZY_ROUGHNESS_FLOOR));
    Result.Facing      = ZySmith(Result.NoV, Result.Alpha);
    Result.Environment = ZyEnvironment(Specular, Roughness, Result.NoV);

    // Light the specular reflects is light the diffuse never gets.
    Result.Scatter     = Diffuse * ZY_INV_PI * (1.0 - Result.Environment);

    // Put back the energy GGX's single bounce loses on rough surfaces.
    Result.Gain        = 1.0 + Specular * ZyBounces(Roughness, Result.NoV);
    return Result;
}

/// Light sent to the eye from a light of given angular radius, using a prepared lobe.
float3 ZyBrdf(ZyLobe Lobe, float3 Normal, float3 Sight, float3 Incident, float3 Specular, float Spread)
{
    const float3 Halfway = normalize(Sight + Incident);
    const float  NoL     = saturate(dot(Normal, Incident));
    const float  NoH     = saturate(dot(Normal, Halfway));
    const float  VoH     = saturate(dot(Sight, Halfway));

    // A sized light widens the lobe; the two spreads add in quadrature to keep energy.
    const float  Widened = Lobe.Alpha * Lobe.Alpha + ZySquare(Spread * 0.5);
    const float  Visible = ZyVisibility(Lobe.NoV, NoL, Lobe.Alpha, Lobe.Facing);

    const float3 Fresnel = ZyFresnel(Specular, VoH);
    const float3 Single  = Fresnel * (ZyDistributionSquare(NoH, Widened) * Visible);

    return (Lobe.Scatter + Single * Lobe.Gain) * NoL;
}

/// Light sent to the eye from a light of given angular radius.
float3 ZyBrdf(float3 Normal, float3 Sight, float3 Incident, float3 Diffuse, float3 Specular, float Roughness, float Spread)
{
    const ZyLobe Lobe = ZyPrepareLobe(Normal, Sight, Diffuse, Specular, Roughness);

    return ZyBrdf(Lobe, Normal, Sight, Incident, Specular, Spread);
}

/// Specular occlusion from ambient occlusion, after Lagarde.
float ZyOcclusion(float NoV, float Occlusion, float Roughness)
{
    return saturate(pow(NoV + Occlusion, exp2(-16.0 * Roughness - 1.0)) - 1.0 + Occlusion);
}

/// The unit way from one point to another, writing their distance and one over it; straight up where they meet.
float3 ZyIncident(float3 From, float3 To, out float Distance, out float Inverse)
{
    const float3 Relative = To - From;
    const float  Square   = dot(Relative, Relative);

    Inverse  = rsqrt(max(Square, ZY_EPSILON * ZY_EPSILON));
    Distance = Square * Inverse;

    return (Distance > ZY_EPSILON) ? Relative * Inverse : float3(0.0, 1.0, 0.0);
}

/// The arc tangent of a source's half width over its distance, matched to its third order at a root's cost.
float ZySourceSpread(float Source, float Inverse)
{
    const float Half = Source * 0.5 * Inverse;

    return Half * rsqrt(1.0 + Half * Half * (2.0 / 3.0));
}

/// Light falloff, full inside the core, inverse square past it and zero at the radius.
float ZyFalloff(float Distance, float Radius, float Core)
{
    const float Share  = Distance / max(Radius, ZY_EPSILON);
    const float Window = ZySquare(saturate(1.0 - ZySquare(ZySquare(Share))));

    return Window * (Core * Core) / max(Distance * Distance, Core * Core);
}

/// Spot cone falloff, full inside the inner angle and gone past the outer.
float ZyCone(float3 Axis, float3 Toward, float Outer, float Inner)
{
    return smoothstep(Outer, Inner, dot(Axis, Toward));
}

/// Hemisphere ambient, sky above and ground below.
float3 ZyHemisphere(float3 Direction, float3 Sky, float3 Ground)
{
    return lerp(Ground, Sky, Direction.y * 0.5 + 0.5);
}

#endif // ZY_PBR_INCLUDED