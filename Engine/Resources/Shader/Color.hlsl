// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_COLOR_INCLUDED
#define ZY_COLOR_INCLUDED

/// Rec. 709 luminance weights.
#define ZY_LUMINANCE_709 float3(0.2126, 0.7152, 0.0722)

/// Converts an sRGB channel to linear light.
float ZyToLinear(float Channel)
{
    return (Channel <= 0.04045) ? Channel / 12.92 : pow(abs((Channel + 0.055) / 1.055), 2.4);
}

/// Converts an sRGB color to linear light.
float3 ZyToLinear(float3 Color)
{
    return float3(ZyToLinear(Color.r), ZyToLinear(Color.g), ZyToLinear(Color.b));
}

/// Converts a linear channel to sRGB.
float ZyToGamma(float Channel)
{
    return (Channel <= 0.0031308) ? Channel * 12.92 : 1.055 * pow(abs(Channel), 1.0 / 2.4) - 0.055;
}

/// Converts a linear color to sRGB.
float3 ZyToGamma(float3 Color)
{
    return float3(ZyToGamma(Color.r), ZyToGamma(Color.g), ZyToGamma(Color.b));
}

/// Applies a plain power curve, for displays that want one instead of sRGB.
float3 ZyToGamma(float3 Color, float Gamma)
{
    return pow(abs(Color), 1.0 / Gamma);
}

/// How bright a linear color reads to the eye.
float ZyLuminance(float3 Color)
{
    return dot(Color, ZY_LUMINANCE_709);
}

#endif // ZY_COLOR_INCLUDED