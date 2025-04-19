cbuffer UBO : register(b0, space1)
{
    float4x4 transform : packoffset(c0);
};

struct Input
{
    float3 Position : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
};

struct Output
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL0;
    float4 Color : COLOR0;
};

Output main(Input input)
{
    Output output;
    output.Color = input.Color;
    output.Normal = input.Normal;
    output.Position = mul(transform, float4(input.Position, 1.0f));
    return output;
}
