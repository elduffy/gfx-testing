#include "std_types.hlsl"

Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler : register(s0, space2);

float4 main(DefaultInput input) : SV_Target0
{
    return Texture.Sample(Sampler, input.uv);
}
