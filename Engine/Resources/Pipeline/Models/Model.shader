// Resources

Texture2D    ColorTexture : register(t0);
SamplerState ColorSampler : register(s0);

// Parameters

cbuffer cb_Global : register(b0)
{
    float4x4 u_Camera;
};

// Attributes

struct vs_Input
{
    float3   Position  : POSITION;
    float2   Texture   : TEXCOORD0;
    float4x4 Transform : TEXCOORD4; // (TEXCOORD4, TEXCOORD5, TEXCOORD6, TEXCOORD7)
};

struct ps_Input
{
    float4 Position : SV_POSITION;
    float2 Texture  : TEXCOORD0;
};

// VS Main

ps_Input vertex(vs_Input Input)
{
    ps_Input Result;

    Result.Position = mul(u_Camera, mul(Input.Transform, float4(Input.Position, 1.0)));
    Result.Texture  = Input.Texture;

    return Result;
}

// PS Main

float4 fragment(ps_Input Input) : SV_Target
{
    return ColorTexture.Sample(ColorSampler, Input.Texture);
}

