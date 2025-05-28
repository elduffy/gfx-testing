#include "buffer_macros.hlsl"
#include "std_types.hlsl"

SAMPLED_TEXTURECUBE_FRAG(Texture, 0);
SAMPLER_FRAG(Sampler, 0);

float4 main(DefaultFragmentData input) : SV_Target0
{
    // Cubemaps expect a left-handed system (Y+ up, Z+ forward), so swizzle X/Y.
    // See https://community.khronos.org/t/image-orientation-for-cubemaps-actually-a-very-old-topic/105338/4
    return Texture.Sample(Sampler, input.positionMS.xzy);
}
