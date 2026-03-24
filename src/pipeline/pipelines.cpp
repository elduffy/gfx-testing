#include <absl/log/check.h>
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
                                            SDL_GPUShader *fragmentShader,
                                            gfx::PipelineDefinition const &pipelineDefinition) {

        SDL_GPUColorTargetDescription colorTargetDescription = {
                .format = context.mColorTargetFormat,
        };
        constexpr SDL_GPUVertexBufferDescription vertexBufferDescription = {
                .slot = 0,
                .pitch = sizeof(shader::VertexData),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
        };

        const SDL_GPUGraphicsPipelineCreateInfo PipelineInfo = {
                .vertex_shader = vertexShader,
                .fragment_shader = fragmentShader,
                .vertex_input_state =
                        SDL_GPUVertexInputState{
                                .vertex_buffer_descriptions = &vertexBufferDescription,
                                .num_vertex_buffers = 1,
                                .vertex_attributes = &shader::VertexData::VERTEX_ATTRIBUTES[0],
                                .num_vertex_attributes = shader::VertexData::VERTEX_ATTRIBUTES.size(),
                        },
                .primitive_type = pipelineDefinition.mPrimitiveType,
                .rasterizer_state =
                        {
                                .fill_mode = pipelineDefinition.mFillMode,
                                .cull_mode = SDL_GPU_CULLMODE_BACK,
                        },
                .multisample_state =
                        {
                                .sample_count = MSAA_SAMPLE_COUNT,
                        },
                .depth_stencil_state =
                        {
                                .compare_op = SDL_GPU_COMPAREOP_LESS_OR_EQUAL,
                                .enable_depth_test = true,
                                .enable_depth_write = !pipelineDefinition.mIsBackground,
                        },
                .target_info =
                        {
                                .color_target_descriptions = &colorTargetDescription,
                                .num_color_targets = 1,
                                .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                                .has_depth_stencil_target = true,
                        },
        };
        auto *pipeline = SDL_CreateGPUGraphicsPipeline(context.mDevice, &PipelineInfo);
        CHECK_NE(pipeline, nullptr) << "Failed to create graphics pipeline: " << SDL_GetError();
        return pipeline;
    }

    SDL_GPUComputePipeline *createPipeline(sdl::SdlContext const &context, io::ShaderCode const &shaderCode,
                                           compute::PipelineDefinition const &pipelineDefinition) {
        const SDL_GPUComputePipelineCreateInfo createInfo{
                .code_size = shaderCode.mSize,
                .code = shaderCode.mCode,
                .entrypoint = "main",
                .format = SDL_GPU_SHADERFORMAT_SPIRV,
                .num_readonly_storage_buffers = pipelineDefinition.mShader.mReadonlyStorageBuffers,
                .num_readwrite_storage_buffers = pipelineDefinition.mShader.mReadwriteStorageBuffers,
                .num_uniform_buffers = pipelineDefinition.mShader.mUniformBuffers,
                .threadcount_x = pipelineDefinition.mShader.mWorkgroupSize[0],
                .threadcount_y = pipelineDefinition.mShader.mWorkgroupSize[1],
                .threadcount_z = pipelineDefinition.mShader.mWorkgroupSize[2],
        };
        auto *pipeline = SDL_CreateGPUComputePipeline(context.mDevice, &createInfo);
        CHECK_NE(pipeline, nullptr) << "Failed to create compute pipeline: " << SDL_GetError();
        return pipeline;
    }

    std::map<gfx::ShaderDefinition, io::ShaderCode> loadGfxShaderCode(io::ResourceLoader const &resourceLoader) {
        std::map<gfx::ShaderDefinition, io::ShaderCode> shaderCodes;
        for (auto const &shaderDef: gfx::ALL_SHADERS) {
            shaderCodes.emplace(shaderDef, resourceLoader.loadShaderCode(shaderDef.mFilename));
        }
        return shaderCodes;
    }

    std::map<compute::ShaderDefinition, io::ShaderCode>
    loadComputeShaderCode(io::ResourceLoader const &resourceLoader) {
        std::map<compute::ShaderDefinition, io::ShaderCode> shaderCodes;
        for (auto const &shaderDef: compute::ALL_SHADERS) {
            shaderCodes.emplace(shaderDef, resourceLoader.loadShaderCode(shaderDef.mFilename));
        }
        return shaderCodes;
    }

    std::map<gfx::ShaderDefinition, sdl::SdlShader>
    createShaders(sdl::SdlContext const &context, std::map<gfx::ShaderDefinition, io::ShaderCode> const &code) {
        std::map<gfx::ShaderDefinition, sdl::SdlShader> shaders;
        for (auto const &[shaderDef, shaderCode]: code) {
            shaders.emplace(shaderDef,
                            sdl::SdlShader::createShader(context, shaderCode.mCode, shaderCode.mSize, shaderDef.mStage,
                                                         shaderDef.mSamplers, shaderDef.mUniformBuffers,
                                                         shaderDef.mStorageBuffers, shaderDef.mStorageTextures));
        }
        return shaders;
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext,
                         std::map<gfx::ShaderDefinition, io::ShaderCode> const &gfxShaderCode,
                         std::map<compute::ShaderDefinition, io::ShaderCode> const &computeShaderCode) :
        Pipelines(sdlContext, createShaders(sdlContext, gfxShaderCode), computeShaderCode) {}

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext,
                         std::map<gfx::ShaderDefinition, sdl::SdlShader> const &gfxShaders,
                         std::map<compute::ShaderDefinition, io::ShaderCode> const &computeShaderCode) {
        static_assert(isWellDefined(gfx::ALL_PIPELINES));
        for (auto const &pipelineDefinition: gfx::ALL_PIPELINES) {
            auto *vertexShader = *gfxShaders.at(pipelineDefinition.mVertexShader);
            auto *fragmentShader = *gfxShaders.at(pipelineDefinition.mFragmentShader);
            mGfxPipelines.emplace_back(
                    pipelineDefinition,
                    sdl::SdlGfxPipeline{sdlContext,
                                        createPipeline(sdlContext, vertexShader, fragmentShader, pipelineDefinition)});
            SDL_Log("Created graphics pipeline %s", getName(pipelineDefinition.mName));
        }

        static_assert(isWellDefined(compute::ALL_PIPELINES));
        for (auto const &pipelineDefinition: compute::ALL_PIPELINES) {
            auto const &shaderCode = computeShaderCode.at(pipelineDefinition.mShader);
            mComputePipelines.emplace_back(
                    pipelineDefinition,
                    sdl::SdlComputePipeline{sdlContext, createPipeline(sdlContext, shaderCode, pipelineDefinition)});
            SDL_Log("Created compute pipeline %s", getName(pipelineDefinition.mName));
        }
    }

    Pipelines::Pipelines(sdl::SdlContext const &sdlContext, io::ResourceLoader const &resourceLoader) :
        Pipelines(sdlContext, loadGfxShaderCode(resourceLoader), loadComputeShaderCode(resourceLoader)) {}
} // namespace gfx_testing::pipeline
