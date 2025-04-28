#include "std_types.hlsl"

cbuffer MvpTransform : register(b0, space1)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

DefaultOutput main(DefaultInput input)
{
    float4x4 mv = mul(view, model);
    float4x4 mvp = mul(projection, mv);

    DefaultOutput output;
    output.position = mul(mvp, float4(input.position, 1.0f));
    output.uv = input.uv;
    output.normal = input.normal;
    output.color = input.color;
    output.positionMS = input.position.xyz;
    return output;
}
