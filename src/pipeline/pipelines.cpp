#include <array>
#include <pipeline/pipelines.hpp>
#include <shader/shader_models.hpp>

namespace gfx_testing::pipeline {

    template<typename PipelineDefinitions>
    constexpr bool isWellDefined(PipelineDefinitions const &pipelineDefinitions) {
        std::vector<size_t> actualIndices;
        std::vector<size_t> expectedIndices;
        size_t i = 0;
        for (auto const &pipelineDefinition: pipelineDefinitions) {
            actualIndices.push_back(getIndex(pipelineDefinition.mName));
            expectedIndices.push_back(i++);
        }
        return actualIndices == expectedIndices;
    }

    SDL_GPUGraphicsPipeline *createPipeline(sdl::SdlContext const &context, SDL_GPUShader *vertexShader,
                                            SDL_GPUShader *fragmentShader, SDL_GPUPrimitiveType primitiveType,
                                            bool isBackground) {

        SDL_GPUColorTargetDescription colorTargetDescription = {
                .format = SDL_GetGPUSwapchainTextureFormat(context.mDevice, context.mWindow),
        };
        constexpr SDL_GPUVertexBufferDescription vertexBufferDescription = {
                .slot = 0,
                .pitch = sizeof(shader::VertexData),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
        };

        const SDL_GPUGraphicsPipelineCreateInfo graphicsPipelineInfo = {
                .vertex_shader = vertexShader,
                .fragment_shader = fragmentShader,
                .vertex_input_state =
                        SDL_GPUVertexInputState{
                                .vertex_buffer_descriptions = &vertexBufferDescription,
                                .num_vertex_buffers = 1,
                                .vertex_attributes = &shader::VertexData::VERTEX_ATTRIBUTES[0],
                                .num_vertex_attributes = shader::VertexData::VERTEX_ATTRIBUTES.size(),
                        },
                .primitive_type = primitiveType,
                .rasterizer_state = {.cull_mode = SDL_GPU_CULLMODE_BACK},
                .multisample_state =
                        {
                                .sample_count = MSAA_SAMPLE_COUNT,
                        },
                .depth_stencil_state =
                        {
                                .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                                .enable_depth_test = true,
                                .enable_depth_write = !isBackground,
                        },
                .target_info =
                        {
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
        return pipeline;
    }

    std::map<ShaderDefinition, util::ShaderCode> loadShaderCode(util::ResourceLoader const &resourceLoader) {
        std::map<ShaderDefinition, util::ShaderCode> shaderCodes;
        for (auto const &shaderDef: ALL_SHADERS) {
            shaderCodes.emplace(shaderDef, resourceLoader.loadShaderCode(shaderDef.mFilename));
        }
        return shaderCodes;
    }

    std::map<ShaderDefinition, sdl::SdlShader> createShaders(sdl::SdlContext const &context,
                                                             std::map<ShaderDefinition, util::ShaderCode> const &code) {
        std::map<ShaderDefinition, sdl::SdlShader> shaders;
        for (auto const &[shaderDef, shaderCode]: code) {
            shaders.emplace(shaderDef,
                            sdl::SdlShader::createShader(context, shaderCode.mCode, shaderCode.mSize, shaderDef.mStage,
                                                         shaderDef.mSamplers, shaderDef.mUniformBuffers,
                                                         shaderDef.mStorageBuffers, shaderDef.mStorageTextures));
        }
        return shaders;
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext,
                         std::map<ShaderDefinition, util::ShaderCode> const &shaderCode) :
        Pipelines(sdlContext, createShaders(sdlContext, shaderCode)) {}

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, sdl::SdlShader> const &shaders) {
        static_assert(isWellDefined(ALL_PIPELINES));
        for (auto const &pipelineDefinition: ALL_PIPELINES) {
            auto *vertexShader = *shaders.at(pipelineDefinition.mVertexShader);
            auto *fragmentShader = *shaders.at(pipelineDefinition.mFragmentShader);
            mPipelines.emplace_back(
                    pipelineDefinition,
                    sdl::SdlGfxPipeline{sdlContext, createPipeline(sdlContext, vertexShader, fragmentShader,
                                                                   pipelineDefinition.mPrimitiveType,
                                                                   pipelineDefinition.isBackground)});
            SDL_Log("Created graphics pipeline %s", getName(pipelineDefinition.mName));
        }
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader) :
        Pipelines(sdlContext, loadShaderCode(resourceLoader)) {}
} // namespace gfx_testing::pipeline
