cbuffer Params : register(b0, space3)
{
    float3 CameraPos;
    float3 LightPos;
    float3 CoolColor;
    float3 WarmColor;
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
    float3 lightDir = normalize(LightPos - input.Position.xyz);
    float3 cool = CoolColor + 0.25 * input.Color.xyz;
    float3 warm = WarmColor + 0.25 * input.Color.xyz;
    float diff = max(0.0, dot(input.Normal, lightDir));
    float t = (diff + 1.0) / 2.0;
    float3 r = 2.0 * diff * input.Normal - lightDir;

    float3 viewDir = normalize(input.Position.xyz - CameraPos);
    // TOOD: re-enable highlights
    float s = 0;//saturate(100.0 * dot(r, viewDir) - 97.0);

    float3 highlight = float3(1.0, 1.0, 1.0);
    float3 result = s * highlight + (1.0-s)*(t*warm + (1.0-t)*cool);
    return float4(result, 1.0);
}
