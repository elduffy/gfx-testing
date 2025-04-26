struct Input
{
    float4 Position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
};

float4 main(Input input) : SV_Target0
{
    return input.Color;
}
