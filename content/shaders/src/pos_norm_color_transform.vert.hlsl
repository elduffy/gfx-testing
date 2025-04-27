cbuffer MvpTransform : register(b0, space1)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

struct Input
{
    float3 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

struct Output
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    float3 positionMS: POSITION;
};

Output main(Input input)
{
    float4x4 mv = mul(view, model);
    float4x4 mvp = mul(projection, mv);

    Output output;
    output.position = mul(mvp, float4(input.position, 1.0f));
    output.uv = input.uv;
    output.normal = input.normal;
    output.color = input.color;
    output.positionMS = input.position.xyz;
    return output;
}
