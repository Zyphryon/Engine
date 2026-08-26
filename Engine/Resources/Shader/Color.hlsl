// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifndef ZY_COLOR_INCLUDED
#define ZY_COLOR_INCLUDED

/// The share of luminance each primary carries, as Rec. 709 weighs them.
#define ZY_LUMINANCE_709 float3(0.2126, 0.7152, 0.0722)

/// \brief Reads one channel the way sRGB stores it and gives back the light it stands for.
///
/// \param Channel The channel to read, as sRGB stores it.
///
/// \return The light the channel stands for.
float ZyToLinear(float Channel)
{
    return (Channel <= 0.04045) ? Channel / 12.92 : pow(abs((Channel + 0.055) / 1.055), 2.4);
}

/// \brief Reads a color the way sRGB stores it and gives back the light it stands for.
///
/// \param Color The color to read, as sRGB stores it.
///
/// \return The light the color stands for.
float3 ZyToLinear(float3 Color)
{
    return float3(ZyToLinear(Color.r), ZyToLinear(Color.g), ZyToLinear(Color.b));
}

/// \brief Writes one channel of light back the way sRGB stores it.
///
/// \param Channel The channel of light to write.
///
/// \return The channel as sRGB stores it.
float ZyToGamma(float Channel)
{
    return (Channel <= 0.0031308) ? Channel * 12.92 : 1.055 * pow(abs(Channel), 1.0 / 2.4) - 0.055;
}

/// \brief Writes a color of light back the way sRGB stores it.
///
/// \param Color The color of light to write.
///
/// \return The color as sRGB stores it.
float3 ZyToGamma(float3 Color)
{
    return float3(ZyToGamma(Color.r), ZyToGamma(Color.g), ZyToGamma(Color.b));
}

/// \brief Applies a plain power curve, for the places a display wants one instead of the sRGB kink.
///
/// \param Color The color of light to write.
/// \param Gamma The exponent the display expects the color to be raised against.
///
/// \return The color as that curve stores it.
float3 ZyToGamma(float3 Color, float Gamma)
{
    return pow(abs(Color), 1.0 / Gamma);
}

/// \brief Measures how bright a color reads, weighing each primary as the eye does.
///
/// \note The weights hold only over linear light, so a color sRGB still encodes goes through \ref ZyToLinear first.
///
/// \param Color The color of light to measure.
///
/// \return The luminance the color carries.
float ZyLuminance(float3 Color)
{
    return dot(Color, ZY_LUMINANCE_709);
}

#endif // ZY_COLOR_INCLUDED