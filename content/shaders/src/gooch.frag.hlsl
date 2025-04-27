cbuffer Params : register(b0, space3)
{
    float3 coolColor;
    float3 warmColor;
    float3 lightPosMS;
    float3 cameraPosMS;
};

struct Input
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float3 positionMS: POSITION;
};

float4 main(Input input) : SV_Target0
{
    float3 norm = normalize(input.normal);
    float3 cameraDir = normalize(cameraPosMS - input.positionMS);
    float3 lightDir = normalize(lightPosMS - input.positionMS);
    // See Real Time Rendering 5.1
    float3 cool = coolColor + 0.25 * input.color.xyz;
    float3 warm = warmColor + 0.25 * input.color.xyz;
    float diff = dot(norm, lightDir);
    float t = (diff + 1.0) / 2.0;
    float3 r = 2.0 * diff * norm - lightDir;
    float s = saturate(100.0 * dot(r, cameraDir) - 97.0);

    float3 highlight = float3(1.0, 1.0, 1.0);
    float3 result = s * highlight + (1.0-s)*(t*warm + (1.0-t)*cool);
    return float4(result, 1.0);
}
