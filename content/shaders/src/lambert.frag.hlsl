#include "buffer_macros.hlsl"
#include "std_types.hlsl"

STORAGE_BUFFER_FRAG(Params, 0)
{
    float3 unlitColor;
    float3 litColor;
};

UNIFORM_BUFFER_FRAG(ObjectLighting, 0)
{
    float3 lightPosMS;
    float3 cameraPosMS;
};

float4 main(DefaultOutput input) : SV_Target0
{
    float3 norm = normalize(input.normal);
    float3 cameraDir = normalize(cameraPosMS - input.positionMS);
    float3 lightDir = normalize(lightPosMS - input.positionMS);
    // See Real Time Rendering 5.2
    return float4(unlitColor + saturate(dot(lightDir, norm)) * litColor * input.color.xyz, 1);
}
