#include "buffer_macros.hlsl"
#include "std_types.hlsl"
#include "object_lighting.hlsl"

STORAGE_BUFFER_FRAG(Params, 0)
{
    float3 unlitColor;
    float3 litColor;
};

OBJECT_LIGHTING(ObjectLighting, 0);

float4 main(DefaultFragmentData input) : SV_Target0
{
    // See Real Time Rendering 5.2
    float3 norm = normalize(input.normal);
    float intensity = 0;
    FOR_EACH_LIGHT(i) {
        float3 lightDir = normalize(lightPosMS[i] - input.positionMS);
        intensity = max(intensity, saturate(dot(lightDir, norm)));
    }
    return float4((unlitColor + intensity * litColor) * input.color.xyz, 1);
}
