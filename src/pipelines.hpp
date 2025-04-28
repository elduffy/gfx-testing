#pragma once

#include <map>
#include <sdl.hpp>

#include "resource_loader.hpp"

namespace gfx_testing::pipeline {
    static constexpr auto MSAA_SAMPLE_COUNT = SDL_GPU_SAMPLECOUNT_4;

    enum class PipelineName {
        Diffuse,
        Gooch,
        Textured,
    };

    struct ShaderDefinition {
        friend bool operator<(const ShaderDefinition &lhs, const ShaderDefinition &rhs) {
            return lhs.mFilename < rhs.mFilename;
        }

        char const *mFilename;
        SDL_GPUShaderStage mStage;
        uint32_t mSamplers{0};
        uint32_t mUniformBuffers{0};
        uint32_t mStorageBuffers{0};
        uint32_t mStorageTextures{0};
    };

    struct PipelineDefinition {
        PipelineName mName;
        ShaderDefinition mVertexShader;
        ShaderDefinition mFragmentShader;
    };

    // Shader definitions

    static constexpr ShaderDefinition SHADER_BASIC_TEXTURED{
            .mFilename = "basic_textured.frag.hlsl",
            .mStage = SDL_GPU_SHADERSTAGE_FRAGMENT,
            .mSamplers = 1,
    };
    static constexpr ShaderDefinition SHADER_GOOCH{
            .mFilename = "gooch.frag.hlsl",
            .mStage = SDL_GPU_SHADERSTAGE_FRAGMENT,
            .mUniformBuffers = 1,
    };
    static constexpr ShaderDefinition SHADER_NORM_COLOR{
            .mFilename = "norm_color.frag.hlsl",
            .mStage = SDL_GPU_SHADERSTAGE_FRAGMENT,
            .mUniformBuffers = 1,
    };
    static constexpr ShaderDefinition SHADER_DEFAULT_VERTEX{
            .mFilename = "pos_norm_color_transform.vert.hlsl",
            .mStage = SDL_GPU_SHADERSTAGE_VERTEX,
            .mUniformBuffers = 1,
    };
    static constexpr std::array ALL_SHADERS{
            SHADER_BASIC_TEXTURED,
            SHADER_GOOCH,
            SHADER_NORM_COLOR,
            SHADER_DEFAULT_VERTEX,
    };

    // Pipeline definitions

    static constexpr PipelineDefinition PIPELINE_DIFFUSE{
            .mName = PipelineName::Diffuse,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_NORM_COLOR,
    };
    static constexpr PipelineDefinition PIPELINE_GOOCH{
            .mName = PipelineName::Gooch,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_GOOCH,
    };
    static constexpr PipelineDefinition PIPELINE_TEXTURED{
            .mName = PipelineName::Textured,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_BASIC_TEXTURED,
    };
    static constexpr std::array ALL_PIPELINES{
            PIPELINE_DIFFUSE,
            PIPELINE_GOOCH,
            PIPELINE_TEXTURED,
    };

    class Pipelines {
        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, sdl::SdlShader> const &shaders);

        Pipelines(sdl::SdlContext const &sdlContext, std::map<ShaderDefinition, util::ShaderCode> const &shaderCode);

    public:
        Pipelines(sdl::SdlContext const &sdlContext, util::ResourceLoader const &resourceLoader);

        sdl::SdlGfxPipeline const &get(PipelineName pipelineName) const {
            return mPipelines.at(static_cast<size_t>(pipelineName));
        }

    private:
        std::vector<sdl::SdlGfxPipeline> mPipelines;
    };
}
