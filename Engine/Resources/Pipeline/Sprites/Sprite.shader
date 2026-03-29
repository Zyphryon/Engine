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
    uint     VertexID   : SV_VertexID;

    float4   Transform0 : CUSTOM0;
    float4   Transform1 : CUSTOM1;
    float4   Frame      : TEXCOORD0;
    float2   Size       : CUSTOM2;
    float4   Color      : COLOR0;
};

struct ps_Input
{
    float4 Position : SV_POSITION;
    float2 Texture  : TEXCOORD0;
    float4 Color    : COLOR0;
};

// VS Main

ps_Input vertex(vs_Input Input)
{
    ps_Input Result;

    const float2 Corner = float2(
        (Input.VertexID & 1) ? 0.5 : -0.5,
        (Input.VertexID & 2) ? 0.5 : -0.5);
    const float2 UV     = float2(
        (Input.VertexID & 1) ? Input.Frame.z : Input.Frame.x,
        (Input.VertexID & 2) ? Input.Frame.y : Input.Frame.w);

    const float2   Local    = Corner * Input.Size;
    const float2   Position = float2(
        dot(Local, Input.Transform0.xy) + Input.Transform0.z,
        dot(Local, Input.Transform1.xy) + Input.Transform1.z
    );

    Result.Position = mul(u_Camera, float4(Position, Input.Transform0.w, 1.0));
    Result.Texture  = UV;
    Result.Color    = Input.Color;

    return Result;
}

// PS Main

float4 fragment(ps_Input Input) : SV_Target
{
    float4 Texel = ColorTexture.Sample(ColorSampler, Input.Texture);

#ifdef    ENABLE_ALPHA_TEST
    clip(Texel.a - 0.001);
#endif // ENABLE_ALPHA_TEST

    return Input.Color * Texel;
}