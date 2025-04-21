cbuffer MvpTransform : register(b0, space1)
{
    float4x4 modelView;
    float4x4 projection;
};

struct Input
{
    float3 Position : SV_Position;
    float4 Color : COLOR0;
};

struct Output
{
    float4 Color : COLOR0;
    float4 Position : SV_Position;
};

Output main(Input input)
{
    Output output;
    output.Color = input.Color;
    output.Position = mul(mul(projection, modelView), float4(input.Position, 1.0f));
    return output;
}
