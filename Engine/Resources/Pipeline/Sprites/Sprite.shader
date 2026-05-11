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

    float4   Transform0 : TEXCOORD0;
    float4   Transform1 : TEXCOORD1;
    float4   Frame      : TEXCOORD2;
    float2   Size       : TEXCOORD3;
    float4   Color      : COLOR0;
};

struct ps_Input
{
    float4 Position : SV_POSITION;
    float2 Texture  : TEXCOORD0;
    float4 Color    : COLOR0;
};

// VS Main

float2 TessellateRect(uint VertexID)
{
    static const float2 kUnitRectCorners[4] = {
        float2(0, 0),
        float2(1, 0),
        float2(0, 1),
        float2(1, 1)
    };
    return kUnitRectCorners[VertexID];
}

ps_Input vertex(vs_Input Input)
{
    ps_Input Result;

    const float2 Corner   = TessellateRect(Input.VertexID);
    const float2 Local    = Corner * Input.Size;
    const float2 Position = float2(
        dot(Local, Input.Transform0.xy) + Input.Transform0.z,
        dot(Local, Input.Transform1.xy) + Input.Transform1.z
    );

    Result.Position = mul(u_Camera, float4(Position, Input.Transform0.w, 1.0));
    Result.Texture  = lerp(Input.Frame.xy, Input.Frame.zw, float2(Corner.x, 1.0 - Corner.y));
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