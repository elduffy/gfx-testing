cbuffer MvpTransform : register(b0, space1)
{
    float4x4 modelView;
    float4x4 projection;
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
    output.Normal = mul(modelView, float4(input.Normal, 0)).xyz;
    output.Position = mul(mul(projection, modelView), float4(input.Position, 1.0f));
    return output;
}
