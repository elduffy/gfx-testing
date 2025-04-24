#include <array>
#include <pipelines.hpp>
#include <shader_models.hpp>

namespace gfx_testing::pipeline {

    SDL_GPUGraphicsPipeline *createPipeline(sdl::SdlContext const &context, SDL_GPUShader *vertexShader,
                                            SDL_GPUShader *fragmentShader) {

        SDL_GPUColorTargetDescription colorTargetDescription = {
                .format = SDL_GetGPUSwapchainTextureFormat(context.mDevice, context.mWindow),
        };
        constexpr SDL_GPUVertexBufferDescription vertexBufferDescription = {
                .slot = 0,
                .pitch = sizeof(shader::PositionColorVertex),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
        };
        constexpr std::array vertexAttributes{
                SDL_GPUVertexAttribute{
                        .location = 0,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = 0,
                },
                SDL_GPUVertexAttribute{
                        .location = 1,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = sizeof(shader::PositionColorVertex::mPosition),
                },
                SDL_GPUVertexAttribute{
                        .location = 2,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                        .offset = sizeof(shader::PositionColorVertex::mPosition) + sizeof(
                                      shader::PositionColorVertex::mNormal),
                }
        };

        const SDL_GPUGraphicsPipelineCreateInfo graphicsPipelineInfo = {
                .vertex_shader = vertexShader,
                .fragment_shader = fragmentShader,
                .vertex_input_state = SDL_GPUVertexInputState{
                        .vertex_buffer_descriptions = &vertexBufferDescription,
                        .num_vertex_buffers = 1,
                        .vertex_attributes = &vertexAttributes[0],
                        .num_vertex_attributes = vertexAttributes.size(),
                },
                .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                .rasterizer_state = {
                        .cull_mode = SDL_GPU_CULLMODE_BACK
                },
                .depth_stencil_state = {
                        .compare_op = SDL_GPU_COMPAREOP_LESS,
                        .enable_depth_test = true,
                        .enable_depth_write = true,
                },
                .target_info = {
                        .color_target_descriptions = &colorTargetDescription,
                        .num_color_targets = 1,
                        .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                        .has_depth_stencil_target = true,
                },
        };
        auto *pipeline = SDL_CreateGPUGraphicsPipeline(context.mDevice, &graphicsPipelineInfo);
        if (pipeline == nullptr) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
        SDL_Log("Created graphics pipeline");
        return pipeline;
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext, std::filesystem::path const &projectRoot):
        Pipelines(sdlContext,
                  *sdl::SdlShader::loadShader(sdlContext,
                                              projectRoot /
                                              "content/shaders/src/pos_norm_color_transform.vert.hlsl",
                                              0,
                                              2, 0, 0),
                  *sdl::SdlShader::loadShader(sdlContext,
                                              projectRoot /
                                              "content/shaders/src/norm_color.frag.hlsl",
                                              0,
                                              1, 0, 0),
                  *sdl::SdlShader::loadShader(sdlContext,
                                              projectRoot /
                                              "content/shaders/src/gooch.frag.hlsl",
                                              0,
                                              1, 0, 0)) {
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext, SDL_GPUShader *defaultVertexShader,
                         SDL_GPUShader *normColorFragShader, SDL_GPUShader *goochFragShader):
        mDiffuse(sdlContext, createPipeline(sdlContext, defaultVertexShader, normColorFragShader)),
        mGooch(sdlContext, createPipeline(sdlContext, defaultVertexShader, goochFragShader)) {
    }
}
