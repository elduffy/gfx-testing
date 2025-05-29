#include "std_types.hlsl"

StructuredBuffer<DefaultVertexData> VertexBufferIn : register(t0, space0);
RWStructuredBuffer<DefaultVertexData> VertexBufferOut : register(u0, space1);

[numthreads(64, 1, 1)]
void main(uint3 GlobalInvocationID : SV_DispatchThreadID) {
    uint n = GlobalInvocationID.x;
    DefaultVertexData vertIn = VertexBufferIn[n];
    VertexBufferOut[n].position = vertIn.position + vertIn.normal;
}