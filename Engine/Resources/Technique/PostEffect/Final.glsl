// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Copyright (C) 2021-2026 by Agustin L. Alvarez. All rights reserved.
//
// This work is licensed under the terms of the MIT license.
//
// For a copy, see <https://opensource.org/licenses/MIT>.
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#include "Embedded://Shader/Vertex.glsl"
#include "Embedded://Shader/Color.glsl"
#include "Embedded://Shader/Noise.glsl"
#include "Embedded://Shader/Tonemap.glsl"
#include "Embedded://Shader/Antialias.glsl"

layout(std140, binding = 1) uniform cb_Pass
{
    vec4 u_Grading;   // X = Exposure, Y = Saturation, Z = Vignette, W = Grain
    vec4 u_Lift;      // RGB = Lift,   W = The grain's frame, or below zero when nothing past the curve is graded
    vec4 u_Gamma;     // RGB = Gamma,  W = The strength of the halo
    vec4 u_Gain;      // RGB = Gain,   W = The levels each channel is stepped to
    vec4 u_Lens;      // XY = A texel of the scene, Z = Aberration in pixels
};

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Vertex Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef VERTEX_SHADER

out vec2 v_Texture;

void main()
{
    vec4 Screen = ZyEmitScreen(gl_VertexID);

    gl_Position = vec4(Screen.xy, 0.0, 1.0);
    v_Texture   = Screen.zw;
}

#endif // VERTEX_SHADER

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
// Fragment Shader
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

#ifdef FRAGMENT_SHADER

layout(binding = 0) uniform sampler2D t_Scene;

in vec2 v_Texture;

layout(location = 0) out vec4 out_Color;

layout(binding = 1) uniform sampler2D t_Bloom;

// GT7 is the curve the image is authored against; ACES is the variant a scene can ask for instead.
#if defined(ENABLE_TONEMAP_ACES)
    #define Tonemap ZyTonemapAces
#else
    #define Tonemap ZyTonemapGt
#endif // ENABLE_TONEMAP_ACES

float Grain(vec2 Pixel, uint Frame)
{
    ivec2 Cell = ivec2(Pixel);
    uint  Seed = ZyScatter(uint(Cell.x) * 73856093u ^ uint(Cell.y) * 19349663u ^ Frame * 83492791u);

    return float(Seed & 0xFFFFu) * (1.0 / 65535.0) - 0.5;
}

// A 4x4 ordered matrix that dithers a stepped gradient, each cell half a step in so the nudges centre on nought.
const float kBayer[16] = float[16](
     0.5 / 16.0,  8.5 / 16.0,  2.5 / 16.0, 10.5 / 16.0,
    12.5 / 16.0,  4.5 / 16.0, 14.5 / 16.0,  6.5 / 16.0,
     3.5 / 16.0, 11.5 / 16.0,  1.5 / 16.0,  9.5 / 16.0,
    15.5 / 16.0,  7.5 / 16.0, 13.5 / 16.0,  5.5 / 16.0
);

vec3 Palette(vec3 Color, float Levels, vec2 Pixel)
{
    ivec2 Cell  = ivec2(Pixel) & 3;
    float Steps = Levels - 1.0;

    return floor(Color * Steps + 0.5 + (kBayer[Cell.y * 4 + Cell.x] - 0.5)) / Steps;
}

void main()
{
#if defined(ENABLE_ANTIALIAS_FXAA)
    vec3 Scene = ZyAntialiasFXAA(t_Scene, v_Texture, u_Lens.xy, 0.25);
#else
    vec3 Scene = textureLod(t_Scene, v_Texture, 0.0).rgb;
#endif

    // Red and blue part from green toward the corners, as a lens that bends each colour by its own amount.
    if (u_Lens.z > 0.0)
    {
        vec2 Shift = (v_Texture - 0.5) * 2.0 * u_Lens.z * u_Lens.xy;

        Scene.r = textureLod(t_Scene, v_Texture + Shift, 0.0).r;
        Scene.b = textureLod(t_Scene, v_Texture - Shift, 0.0).b;
    }

    // The halo is scene referred, so it joins before exposure carries the pair into the curve.
    vec3 Lit = Scene;

    if (u_Gamma.w > 0.0)
    {
        Lit += textureLod(t_Bloom, v_Texture, 0.0).rgb * u_Gamma.w;
    }

    vec3 Color = Tonemap(Lit * u_Grading.x);

    // Saturation and vignette hold only over light, so both are taken before the image is encoded.
    float Luma = ZyLuminance(Color);
    Color = mix(vec3(Luma), Color, u_Grading.y);

    vec2  Offset   = v_Texture - 0.5;
    float Vignette = 1.0 - u_Grading.z * clamp(dot(Offset, Offset) * 2.0, 0.0, 1.0);

    Color *= Vignette;

    // A grade that changes nothing past the curve hands the curve's image back as it stands.
    if (u_Lift.w < 0.0)
    {
#if defined(ENABLE_ENCODED_OUTPUT)
        out_Color = vec4(ZyToGamma(clamp(Color, 0.0, 1.0)), 1.0);
#else
        out_Color = vec4(clamp(Color, 0.0, 1.0), 1.0);
#endif // ENABLE_ENCODED_OUTPUT
        return;
    }

    // The rest is judged by eye, so it works on the encoded image, where even steps look even.
    vec3 Shown = ZyToGamma(clamp(Color, 0.0, 1.0));

    // The bands are graded on the image the curve handed back, which is the range they are authored against.
    Shown = pow(max(Shown * u_Gain.rgb + u_Lift.rgb, vec3(0.0)), u_Gamma.rgb);

    // Grain shows most at a halfway exposure of the encoded luma, so the corners the vignette darkened grain less.
    float Exposed = clamp(ZyLuminance(Shown), 0.0, 1.0);
    float Density = sqrt(clamp(4.0 * Exposed * (1.0 - Exposed), 0.0, 1.0));

    // The grain is stirred by the scene's clock, at a film's cadence, which keeps it from reading as dirt on the lens.
    Shown += Grain(gl_FragCoord.xy, uint(u_Lift.w)) * (u_Grading.w * Density);

    Shown = clamp(Shown, 0.0, 1.0);

    // The palette is what the display carried, so the steps land last, over the grain that dithers them.
    if (u_Gain.w > 1.0)
    {
        Shown = Palette(Shown, u_Gain.w, gl_FragCoord.xy);
    }

    // The target encodes as it stores, so the image is handed back as light, unless it stores what it is handed.
#if defined(ENABLE_ENCODED_OUTPUT)
    out_Color = vec4(Shown, 1.0);
#else
    out_Color = vec4(ZyToLinear(Shown), 1.0);
#endif // ENABLE_ENCODED_OUTPUT
}

#endif // FRAGMENT_SHADER