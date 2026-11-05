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

/// The lowest roughness shaded, so a mirror never turns a light into one burning texel.
#define ZY_ROUGHNESS_FLOOR 0.045

/// Fit of the scale GGX's reflectance from every side puts on the head-on one, by roughness and sqrt(NoV).
#define ZY_SCALE_0 vec4(-0.93389,   5.76245,  -5.57847,   1.69716)
#define ZY_SCALE_1 vec4( 6.46865, -31.86981,  48.79980, -22.87373)
#define ZY_SCALE_2 vec4(-6.81281,  47.51846, -89.36417,  47.28808)
#define ZY_SCALE_3 vec4( 2.25848, -22.39072,  46.57426, -26.26733)

/// Fit of the bias GGX's reflectance from every side adds, by roughness and sqrt(NoV).
#define ZY_BIAS_0  vec4( 1.84821,  -4.95538,   3.99187,  -0.86727)
#define ZY_BIAS_1  vec4(-6.75487,  26.66359, -33.98736,  14.09534)
#define ZY_BIAS_2  vec4( 8.80842, -44.27804,  66.25260, -30.91070)
#define ZY_BIAS_3  vec4(-3.86136,  22.66879, -36.64702,  17.93608)

/// The diffuse colour, which a metal has none of.
vec3 ZyDiffuse(vec3 Albedo, float Metalness)
{
    return Albedo * (1.0 - Metalness);
}

/// The head-on specular colour, which is the albedo for a metal.
vec3 ZySpecular(vec3 Albedo, float Metalness, float Reflectance)
{
    return mix(vec3(Reflectance), Albedo, Metalness);
}

/// GGX normal distribution, from the lobe's width already squared.
float ZyDistributionSquare(float NoH, float Square)
{
    float Lean = NoH * NoH * (Square - 1.0) + 1.0;

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
    float Square = Alpha * Alpha;

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
vec3 ZyFresnel(vec3 Specular, float VoH)
{
    return Specular + (1.0 - Specular) * pow(1.0 - VoH, 5.0);
}

/// Returns GGX's reflectance under light from every side, as the scale on the head-on reflectance and the bias.
vec2 ZySplit(float Roughness, float NoV)
{
    float Rough = max(Roughness, ZY_ROUGHNESS_FLOOR);
    float Root  = sqrt(max(NoV, 0.05));
    vec4  Along = vec4(1.0, Root, Root * Root, Root * Root * Root);
    vec4  Up    = vec4(1.0, Rough, Rough * Rough, Rough * Rough * Rough);
    vec4  Scale = vec4(
        dot(ZY_SCALE_0, Along),
        dot(ZY_SCALE_1, Along),
        dot(ZY_SCALE_2, Along),
        dot(ZY_SCALE_3, Along));
    vec4  Bias  = vec4(
        dot(ZY_BIAS_0, Along),
        dot(ZY_BIAS_1, Along),
        dot(ZY_BIAS_2, Along),
        dot(ZY_BIAS_3, Along));

    return max(vec2(dot(Up, Scale), dot(Up, Bias)), 0.0);
}

/// Holds the BRDF terms every light on a point shares, for one view.
struct ZyLobe
{
    /// The cosine of normal and view, kept above zero.
    float NoV;

    /// The floored roughness, squared.
    float Alpha;

    /// The view's term of the height-correlated visibility, the same for every light.
    float Facing;

    /// The reflectance under light from every side.
    vec3  Environment;

    /// The diffuse per unit of light, less what the specular took.
    vec3  Scatter;

    /// The boost to the single bounce for the bounces GGX leaves out.
    vec3  Gain;
};

/// Builds the BRDF terms every light on a point shares.
ZyLobe ZyPrepareLobe(vec3 Normal, vec3 Sight, vec3 Diffuse, vec3 Specular, float Roughness)
{
    ZyLobe Result;
    Result.NoV         = max(dot(Normal, Sight), ZY_EPSILON);

    vec2 Split = ZySplit(Roughness, Result.NoV);
    Result.Alpha       = ZySquare(max(Roughness, ZY_ROUGHNESS_FLOOR));
    Result.Facing      = ZySmith(Result.NoV, Result.Alpha);
    Result.Environment = Specular * Split.x + Split.y;

    // Light the specular reflects is light the diffuse never gets.
    Result.Scatter     = Diffuse * ZY_INV_PI * (1.0 - Result.Environment);

    // Put back the energy GGX's single bounce loses on rough surfaces, what a white metal's misses (1/E - 1).
    Result.Gain        = 1.0 + Specular * (1.0 / (Split.x + Split.y) - 1.0);
    return Result;
}

/// Light sent to the eye from a light of given angular radius, using a prepared lobe.
vec3 ZyBrdf(ZyLobe Lobe, vec3 Normal, vec3 Sight, vec3 Incident, vec3 Specular, float Spread)
{
    vec3  Halfway = ZySafeNormalize(Sight + Incident);
    float NoL     = clamp(dot(Normal, Incident), 0.0, 1.0);
    float NoH     = clamp(dot(Normal, Halfway), 0.0, 1.0);
    float VoH     = clamp(dot(Sight, Halfway), 0.0, 1.0);

    // A sized light widens the lobe; the two spreads add in quadrature to keep energy.
    float Widened = Lobe.Alpha * Lobe.Alpha + ZySquare(Spread * 0.5);
    float Visible = ZyVisibility(Lobe.NoV, NoL, Lobe.Alpha, Lobe.Facing);

    vec3 Fresnel = ZyFresnel(Specular, VoH);
    vec3 Single  = Fresnel * (ZyDistributionSquare(NoH, Widened) * Visible);

    return (Lobe.Scatter + Single * Lobe.Gain) * NoL;
}

/// Light sent to the eye from a light of given angular radius.
vec3 ZyBrdf(vec3 Normal, vec3 Sight, vec3 Incident, vec3 Diffuse, vec3 Specular, float Roughness, float Spread)
{
    ZyLobe Lobe = ZyPrepareLobe(Normal, Sight, Diffuse, Specular, Roughness);

    return ZyBrdf(Lobe, Normal, Sight, Incident, Specular, Spread);
}

/// Specular occlusion from ambient occlusion, after Lagarde.
float ZyOcclusion(float NoV, float Occlusion, float Roughness)
{
    return ZySaturate(pow(NoV + Occlusion, exp2(-16.0 * Roughness - 1.0)) - 1.0 + Occlusion);
}

/// The unit way from one point to another, writing their distance and one over it; straight up where they meet.
vec3 ZyIncident(vec3 From, vec3 To, out float Distance, out float Inverse)
{
    vec3  Relative = To - From;
    float Square   = dot(Relative, Relative);

    Inverse  = inversesqrt(max(Square, ZY_EPSILON * ZY_EPSILON));
    Distance = Square * Inverse;

    return (Distance > ZY_EPSILON) ? Relative * Inverse : vec3(0.0, 1.0, 0.0);
}

/// The arc tangent of a source's half width over its distance, matched to its third order at a root's cost.
float ZySourceSpread(float Source, float Inverse)
{
    float Half = Source * 0.5 * Inverse;

    return Half * inversesqrt(1.0 + Half * Half * (2.0 / 3.0));
}

/// Light falloff, full inside the core, inverse square past it and zero at the radius.
float ZyFalloff(float Distance, float Radius, float Core)
{
    float Share  = Distance / max(Radius, ZY_EPSILON);
    float Window = ZySquare(clamp(1.0 - ZySquare(ZySquare(Share)), 0.0, 1.0));

    return Window * (Core * Core) / max(Distance * Distance, Core * Core);
}

/// Spot cone falloff, full inside the inner angle and gone past the outer.
float ZyCone(vec3 Axis, vec3 Toward, float Outer, float Inner)
{
    return smoothstep(Outer, Inner, dot(Axis, Toward));
}

/// Hemisphere ambient, sky above and ground below.
vec3 ZyHemisphere(vec3 Direction, vec3 Sky, vec3 Ground)
{
    return mix(Ground, Sky, Direction.y * 0.5 + 0.5);
}

#endif // ZY_PBR_INCLUDED