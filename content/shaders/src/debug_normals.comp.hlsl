#include "buffer_macros.hlsl"
#include "std_types.hlsl"

// Vertex buffer for the mesh on which to draw normals
// Note: can't use StructuredBuffer<DefaultVertexData> due to different packing
ByteAddressBuffer VertexBufferIn : register(t0, space0);

// Output vertex buffer. Must be at least 2x the size of VertexBufferIn
RWByteAddressBuffer VertexBufferOut : register(u0, space1);

UNIFORM_BUFFER_COMPUTE(Options, 0) {
    float3 lineColor;
    float lineLength;
};

// Someday we might have an offsetof macro to use instead of this: https://github.com/microsoft/hlsl-specs/issues/257
UNIFORM_BUFFER_COMPUTE(VertexBufferOffsets, 1) {
    uint normalOffset;
    uint colorOffset;
};

[numthreads(64, 1, 1)]
void main(uint3 GlobalInvocationID : SV_DispatchThreadID) {
    uint vertexInIdx = GlobalInvocationID.x;

    uint stride = sizeof(DefaultVertexData);
    uint numBytes;
    VertexBufferIn.GetDimensions(numBytes);
    uint numVertices = numBytes / stride;

    if (vertexInIdx < numVertices) {
        float3 posIn = VertexBufferIn.Load<float3>(vertexInIdx * stride);
        float3 normIn = VertexBufferIn.Load<float3>(vertexInIdx * stride + normalOffset);
        // First vertex out
        VertexBufferOut.Store<float3>(2 * vertexInIdx * stride, posIn);
        VertexBufferOut.Store<float4>(2 * vertexInIdx * stride + colorOffset, float4(lineColor, 1));
        // Second vertex out
        VertexBufferOut.Store<float3>((2 * vertexInIdx + 1) * stride, posIn + lineLength * normIn);
        VertexBufferOut.Store<float4>((2 * vertexInIdx + 1) * stride + colorOffset, float4(lineColor, 1));
    }
}