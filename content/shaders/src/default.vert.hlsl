#include "std_types.hlsl"

cbuffer MvpTransform : register(b0, space1)
{
    float4x4 mvp;
};

DefaultOutput main(DefaultInput input) {
    DefaultOutput output;
    output.position = mul(mvp, float4(input.position, 1.0f));
    output.uv = input.uv;
    output.normal = input.normal;
    output.color = input.color;
    output.positionMS = input.position.xyz;
    return output;
}
