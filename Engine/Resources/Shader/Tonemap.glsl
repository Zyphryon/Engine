// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_TONEMAP_INCLUDED
#define ZY_TONEMAP_INCLUDED

/// Carries linear sRGB into the space the ACES fit works in.
const mat3 ZY_ACES_INPUT = mat3(
    0.59719, 0.35458, 0.04823,
    0.07600, 0.90834, 0.01566,
    0.02840, 0.13383, 0.83777);

/// Carries the ACES fit's result back to linear sRGB.
const mat3 ZY_ACES_OUTPUT = mat3(
     1.60475, -0.53108, -0.07367,
    -0.10208,  1.10813, -0.00605,
    -0.00327, -0.07276,  1.07602);

/// Carries linear sRGB into the Rec.2020 inset space AgX compresses in.
const mat3 ZY_AGX_INPUT = mat3(
    0.544812080, 0.373797444, 0.081380964,
    0.140419345, 0.754107783, 0.105396747,
    0.088817138, 0.178859755, 0.732315427);

/// Carries AgX's compressed result back out of the inset space, still in Rec.2020.
const mat3 ZY_AGX_OUTPUT = mat3(
     1.127100582, -0.110606643, -0.016493939,
    -0.141329763,  1.157823702, -0.016493939,
    -0.141329763, -0.110606643,  1.251936407);

/// Carries linear Rec.2020 back to linear sRGB.
const mat3 ZY_AGX_DISPLAY = mat3(
     1.6605, -0.5876, -0.0728,
    -0.1246,  1.1329, -0.0083,
    -0.0182, -0.1006,  1.1187);

/// ACES filmic tonemap, to 0..1.
vec3 ZyTonemapAces(vec3 Color)
{
    Color = Color * ZY_ACES_INPUT;

    vec3 Numerator   = Color * (Color + 0.0245786) - 0.000090537;
    vec3 Denominator = Color * (0.983729 * Color + 0.4329510) + 0.238081;

    return clamp((Numerator / Denominator) * ZY_ACES_OUTPUT, 0.0, 1.0);
}

/// Cheap approximation of the ACES filmic tonemap, to 0..1.
vec3 ZyTonemapAcesFast(vec3 Color)
{
    return clamp((Color * (2.51 * Color + 0.03)) / (Color * (2.43 * Color + 0.59) + 0.14), 0.0, 1.0);
}

/// Khronos PBR Neutral tonemap, to 0..1, which keeps colours as authored until they near white.
vec3 ZyTonemapNeutral(vec3 Color)
{
    const float Start        = 0.8 - 0.04;
    const float Desaturation = 0.15;

    float Least = min(Color.r, min(Color.g, Color.b));
    Color -= (Least < 0.08 ? Least - 6.25 * Least * Least : 0.04);

    float Peak = max(Color.r, max(Color.g, Color.b));

    if (Peak < Start)
    {
        return clamp(Color, 0.0, 1.0);
    }

    float Knee    = 1.0 - Start;
    float Shelved = 1.0 - Knee * Knee / (Peak + Knee - Start);
    float Fade    = 1.0 - 1.0 / (Desaturation * (Peak - Shelved) + 1.0);

    return clamp(mix(Color * (Shelved / Peak), vec3(Shelved), Fade), 0.0, 1.0);
}

/// AgX tonemap over Rec.2020 primaries, to 0..1, which keeps hues as bright colours wash toward white.
vec3 ZyTonemapAgx(vec3 Color)
{
    const float Lowest  = -12.47393;
    const float Highest = 4.026069;

    Color = Color * ZY_AGX_INPUT;
    Color = (clamp(log2(max(Color, vec3(1e-10))), Lowest, Highest) - Lowest) / (Highest - Lowest);

    vec3 Upper = 98.056696 + Color * (-98.640212 + Color * 34.730566);
    vec3 Lower = 7.37235 + Color * (-40.213864 + Color * Upper);

    Color = clamp(Color * (-0.301287 + Color * Lower), 0.0, 1.0);

    return clamp(pow(max(Color * ZY_AGX_OUTPUT, vec3(0.0)), vec3(2.2)) * ZY_AGX_DISPLAY, 0.0, 1.0);
}

#endif // ZY_TONEMAP_INCLUDED