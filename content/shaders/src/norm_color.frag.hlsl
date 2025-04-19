struct Input
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
};

float4 main(Input input) : SV_Target0
{
    return float4(input.Normal, 1);
}
