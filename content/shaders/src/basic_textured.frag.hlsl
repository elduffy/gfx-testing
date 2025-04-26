Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

struct Input
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normalCamS : NORMAL0;
    float4 color : COLOR0;
};

float4 main(Input input) : SV_Target0
{
    return Texture.Sample(Sampler, input.uv);
}
