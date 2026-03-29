// Resources

Texture2D    ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

// Parameters

cbuffer cb_Global   : register(b0)
{
    float4x4 u_Camera;
};

cbuffer cb_Material : register(b2)
{
    float2   u_Range;
};

cbuffer cb_Object   : register(b3)
{
    struct PackedFontParameters
    {
        float4 u_OutsetTint;
        float  u_OutsetOffset;
        float  u_OutsetWidthRelative;
        float  u_OutsetWidthAbsolute;
        float  u_OutsetBlur;
        float  u_InsetRoundness;
        float  u_InsetThreshold;
    };
    PackedFontParameters u_Parameters[64];
};

// Attributes

struct vs_Input
{
    uint     VertexID   : SV_VertexID;

    float4   Transform0 : CUSTOM0;
    float4   Transform1 : CUSTOM1;
    float4   Frame      : TEXCOORD0;
    float4   Local      : CUSTOM2;
    float4   Color      : COLOR0;
    uint     Effect     : CUSTOM3;
};

struct ps_Input
{
    float4               Position : SV_POSITION;
    float2               Texture  : TEXCOORD0;
    float4               Color    : COLOR0;
    nointerpolation uint Effect   : CUSTOM3;
};

// VS Main

ps_Input vertex(vs_Input Input)
{
    static const float2 Quad[4] =
    {
        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1)
    };

    const float2 Corner   = Quad[Input.VertexID];
    const float2 LocalPos = Input.Local.xy + Corner * Input.Local.zw;

    // Apply transform
    const float3 WorldPos = float3(
        dot(Input.Transform0.xyz, float3(LocalPos, 1.0)),
        dot(Input.Transform1.xyz, float3(LocalPos, 1.0)),
        0.0);

    ps_Input Result;

    Result.Position = mul(u_Camera, float4(WorldPos, 1.0));
    Result.Texture  = lerp(Input.Frame.xy, Input.Frame.zw, Corner);
    Result.Color    = Input.Color;
    Result.Effect   = Input.Effect;

    return Result;
}

// PS Main

float Median(float3 Color)
{
    return max(min(Color.r, Color.g), min(max(Color.r, Color.g), Color.b));
}

float Spread(float2 Coordinates, float2 Unit)
{
    return max(0.5 * dot(Unit, 1.0 / fwidth(Coordinates)), 1.0);
}

float4 fragment(ps_Input Input) : SV_Target
{
    const PackedFontParameters FontParameters = u_Parameters[Input.Effect];

    const float4 Sample       = ColorTexture.Sample(ColorSampler, Input.Texture);
    const float  DistanceSDF  = Sample.a;
    const float  DistanceMSDF = min(Median(Sample.rgb), DistanceSDF + 0.1);

    // Convert atlas distance into normalized screen-space range.
    const float  Scale = Spread(Input.Texture, u_Range);

    // Interpolated distance field depending on rounded vs sharp style.
    const float InnerStrokeDistance = lerp(DistanceMSDF, DistanceSDF, FontParameters.u_InsetRoundness);
    const float OuterStrokeDistance = lerp(DistanceMSDF, DistanceSDF, FontParameters.u_InsetRoundness);

    // Convert distance to alpha coverage.
    const float InnerStrokeA = Scale * (InnerStrokeDistance - FontParameters.u_InsetThreshold)
            + 0.5
            + FontParameters.u_OutsetOffset;
    const float OuterStrokeA = Scale * (OuterStrokeDistance - FontParameters.u_InsetThreshold
            + FontParameters.u_OutsetWidthRelative)
            + 0.5
            + FontParameters.u_OutsetOffset
            + FontParameters.u_OutsetWidthAbsolute;

    float4 InnerColor  = Input.Color;
    float4 OuterColor  = FontParameters.u_OutsetTint;
    float InnerOpacity = clamp(InnerStrokeA, 0.0, 1.0);
    float OuterOpacity = clamp(OuterStrokeA, 0.0, 1.0);

    // Optional: Apply Blur Effect
    const float BlurStart  = FontParameters.u_OutsetWidthRelative + FontParameters.u_OutsetWidthAbsolute / Scale;
    const float BlurEnd    = BlurStart * (1.0 - FontParameters.u_OutsetBlur);
    const float BlurDist   = FontParameters.u_InsetThreshold - DistanceSDF - FontParameters.u_OutsetOffset / Scale;
    const float BlurFactor = lerp(1.0, smoothstep(BlurStart, BlurEnd, BlurDist), step(0.0001, FontParameters.u_OutsetBlur));
    OuterColor.a *= BlurFactor;

    // Apply Gamma Correction.
    InnerOpacity = pow(InnerOpacity, 1.0 / 2.2);
    OuterOpacity = pow(OuterOpacity, 1.0 / 2.2);

    float4 Result = (InnerColor * InnerOpacity) + (OuterColor * (OuterOpacity - InnerOpacity));
    return Result;
}