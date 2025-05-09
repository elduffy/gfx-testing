#include "buffer_macros.hlsl"
#include "std_types.hlsl"

SAMPLED_TEXTURE_FRAG(Texture, 0);
SAMPLER_FRAG(Sampler, 0);

float4 main(DefaultOutput input) : SV_Target0
{
    return Texture.Sample(Sampler, input.uv);
}
