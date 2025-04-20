cbuffer Params : register(b0, space3)
{
    float3 ViewDir : POSITION0;
    float3 LightDir : POSITION0;
    float3 CoolColor : COLOR1;
    float3 WarmColor : COLOR2;
};

struct Input
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
};

float4 main(Input input) : SV_Target0
{
    // See Real Time Rendering 5.1
    float3 cool = CoolColor + 0.25 * input.Color.xyz;
    float3 warm = WarmColor + 0.25 * input.Color.xyz;
    float t = (dot(input.Normal, LightDir) + 1) / 2;
    float3 r = 2 * dot(input.Normal, LightDir) * input.Normal - LightDir;
    float s = 0;//saturate(100*dot(r, ViewDir) - 97);

    float3 highlight = float3(1.0, 1.0, 1.0);
    float3 result = s * highlight + (1-s)*(t*warm + (1-t)*cool);
    return float4(result, 1.0);
}
