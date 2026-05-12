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
    float3   Position    : POSITION;
    float2   Texture     : TEXCOORD0;
    float4x4 Transform   : TEXCOORD4; // (TEXCOORD4, TEXCOORD5, TEXCOORD6, TEXCOORD7)
    float4   DepthParams : COLOR3; // x=Order, y=BoundsMinZ, z=BoundsRangeZInv, w=unused
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

    float4 world = mul(Input.Transform, float4(Input.Position, 1.0));
    float4 clip  = mul(u_Camera, world);

    // Compress the model's Z range into a tiny depth-buffer slice centred on Order.
    // This makes the model sort correctly among 2D sprites while preserving internal
    // triangle depth-ordering.  kSliceSize is 1/65536 (~15 bits of internal precision).
    static const float kSliceSize = 1.0 / 65536.0;
    float t  = saturate((world.z - Input.DepthParams.y) * Input.DepthParams.z);
    clip.z   = Input.DepthParams.x + t * kSliceSize;

    Result.Position = clip;
    Result.Texture  = Input.Texture;

    return Result;
}

// PS Main

float4 fragment(ps_Input Input) : SV_Target
{
    return ColorTexture.Sample(ColorSampler, Input.Texture);
}

