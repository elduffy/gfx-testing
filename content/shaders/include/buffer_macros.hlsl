/**
 * Defines used to enforce the SDL layout requirements.
 * See https://wiki.libsdl.org/SDL3/SDL_CreateGPUShader#remarks
 */

// Vertex
#define UNIFORM_BUFFER_VERT(Name, n) cbuffer Name : register(b ## n, space1)

// Fragment
#define STORAGE_BUFFER_FRAG(Name, n) tbuffer Name : register(t ## n, space2)
#define SAMPLED_TEXTURE_FRAG(Name, n) Texture2D<float4> Name : register(t ## n, space2);
#define SAMPLED_TEXTURECUBE_FRAG(Name, n) TextureCube<float4> Name : register(t ## n, space2);
#define SAMPLER_FRAG(Name, n) SamplerState Name : register(s ## n, space2);
#define UNIFORM_BUFFER_FRAG(Name, n) cbuffer Name : register(b ## n, space3)

// Compute
#define UNIFORM_BUFFER_COMPUTE(Name, n) cbuffer Name : register(b ## n, space2)