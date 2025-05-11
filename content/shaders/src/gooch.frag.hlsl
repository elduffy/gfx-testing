#include "buffer_macros.hlsl"
#include "std_types.hlsl"

STORAGE_BUFFER_FRAG(GoochParams, 0)
{
    float3 coolColor;
    float3 warmColor;
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
    // See Real Time Rendering 5.1
    float3 norm = normalize(input.normal);
    float3 cameraDir = normalize(cameraPosMS - input.positionMS);
    float3 cool = coolColor + 0.25 * input.color.xyz;
    float3 warm = warmColor + 0.25 * input.color.xyz;

    float t = 0;
    float s = 0;
    for (int i = 0; i < min(numLights, MAX_NUM_LIGHTS); i++) {
        float3 lightDir = normalize(lightPosMS[i] - input.positionMS);
        t = max(t, (dot(norm, lightDir) + 1.0) / 2.0);
        float3 r = reflect(-lightDir, norm);
        s = max(s, saturate(100.0 * dot(r, cameraDir) - 97.0));
    }

    float3 highlight = float3(1.0, 1.0, 1.0);
    float3 result = lerp(lerp(cool, warm, t), highlight, s);
    return float4(result, 1.0);
}
