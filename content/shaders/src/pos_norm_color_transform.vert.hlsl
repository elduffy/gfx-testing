cbuffer MvpTransform : register(b0, space1)
{
    float4x4 model;
    float4x4 view;
    float4x4 projection;
};

cbuffer CameraLight : register(b1, space1)
{
    float3 cameraPosWs;
    float3 lightPosWs;
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
    float3 normalCamS : NORMAL0;
    float4 color : COLOR0;
    // Pointing toward the camera
    float3 cameraDirCamS: POSITION0;
    // Pointing toward the camera
    float3 lightDirCamS: POSITION1;
};

Output main(Input input)
{
    float4x4 mv = mul(view, model);
    float4x4 mvp = mul(projection, mv);

    float3 vertexPosCamS = mul(mv, float4(input.position, 1)).xyz;
    float3 lightPosCamS = mul(view, float4(lightPosWs, 1)).xyz;

    Output output;
    output.position = mul(mvp, float4(input.position, 1.0f));
    output.uv = input.uv;
    output.normalCamS = mul(mv, float4(input.normal, 0.0f)).xyz;
    output.color = input.color;
    output.cameraDirCamS = normalize(float3(0,0,0) - vertexPosCamS);
    output.lightDirCamS = normalize(lightPosCamS - vertexPosCamS);
    return output;
}
