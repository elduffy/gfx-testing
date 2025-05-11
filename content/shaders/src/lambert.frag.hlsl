#include "buffer_macros.hlsl"
#include "std_types.hlsl"

STORAGE_BUFFER_FRAG(Params, 0)
{
    float3 unlitColor;
    float3 litColor;
};

#define MAX_NUM_LIGHTS 8

UNIFORM_BUFFER_FRAG(ObjectLighting, 0)
{
    float3 cameraPosMS;
    float pad;
    uint numLights;
    float3 lightPosMS[MAX_NUM_LIGHTS];
};

float4 main(DefaultOutput input) : SV_Target0
{
    // See Real Time Rendering 5.2
    float3 norm = normalize(input.normal);
    float3 cameraDir = normalize(cameraPosMS - input.positionMS);
    float intensity = 0;
    for (int i = 0; i < min(numLights, MAX_NUM_LIGHTS); i++) {
        float3 lightDir = normalize(lightPosMS[i] - input.positionMS);
        intensity = max(intensity, saturate(dot(lightDir, norm)));
    }
    return float4(unlitColor + intensity * litColor * input.color.xyz, 1);
}
