// Vertex data input used in the default vertex shader
struct DefaultInput
{
    float3 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
};

// Output from default vertex shader.
// Contains the data from DefaultInput extended for per-fragment lighting.
struct DefaultOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 color : COLOR0;
    // Model space position
    float3 positionMS: POSITION;
};