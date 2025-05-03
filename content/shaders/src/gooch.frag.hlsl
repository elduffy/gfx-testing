#include "std_types.hlsl"

tbuffer GoochParams : register(t0, space2)
{
    float3 coolColor;
    float3 warmColor;
};
cbuffer ObjectLighting : register(b0, space3)
{
    float3 lightPosMS;
    float3 cameraPosMS;
};

float4 main(DefaultOutput input) : SV_Target0
{
    float3 norm = normalize(input.normal);
    float3 cameraDir = normalize(cameraPosMS - input.positionMS);
    float3 lightDir = normalize(lightPosMS - input.positionMS);
    // See Real Time Rendering 5.1
    float3 cool = coolColor + 0.25 * input.color.xyz;
    float3 warm = warmColor + 0.25 * input.color.xyz;
    float t = (dot(norm, lightDir) + 1.0) / 2.0;
    float3 r = reflect(-lightDir, norm);
    float s = saturate(100.0 * dot(r, cameraDir) - 97.0);

    float3 highlight = float3(1.0, 1.0, 1.0);
    float3 result = lerp(lerp(cool, warm, t), highlight, s);
    return float4(result, 1.0);
}
