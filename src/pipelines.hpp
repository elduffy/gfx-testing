#pragma once

#include <map>
#include <sdl.hpp>

#include "resource_loader.hpp"
#include "default.vert.hpp"
#include "gooch.frag.hpp"
#include "basic_textured.frag.hpp"
#include "vertex_color.frag.hpp"

namespace gfx_testing::pipeline {
    static constexpr auto MSAA_SAMPLE_COUNT = SDL_GPU_SAMPLECOUNT_4;

    enum class PipelineName {
        Diffuse,
        Gooch,
        Textured,
        Lines,
    };

    struct ShaderDefinition {
        friend bool operator<(const ShaderDefinition &lhs, const ShaderDefinition &rhs) {
            return lhs.mFilename < rhs.mFilename;
        }

        template<typename SpirvMeta>
        static constexpr ShaderDefinition create(char const *filename, SpirvMeta const &spirvMeta) {
            return {
                    .mFilename = filename,
                    .mStage = spirvMeta.mEntryPoint.mStage,
                    .mSamplers = spirvMeta.mSeparateSamplers.size(),
                    .mUniformBuffers = spirvMeta.mUbos.size(),
                    .mStorageBuffers = spirvMeta.mSsbos.size(),
                    .mStorageTextures = spirvMeta.mStorageTextures.size(),
            };
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
        SDL_GPUPrimitiveType mPrimitiveType{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};
    };

    // Shader definitions

    static constexpr ShaderDefinition SHADER_BASIC_TEXTURED = ShaderDefinition::create(
            "basic_textured.frag.hlsl",
            spirv_header_gen::generated::basic_textured_frag::META);
    static constexpr ShaderDefinition SHADER_GOOCH = ShaderDefinition::create(
            "gooch.frag.hlsl",
            spirv_header_gen::generated::gooch_frag::META);
    static constexpr ShaderDefinition SHADER_VERTEX_COLOR = ShaderDefinition::create(
            "vertex_color.frag.hlsl",
            spirv_header_gen::generated::vertex_color_frag::META);
    static constexpr ShaderDefinition SHADER_DEFAULT_VERTEX = ShaderDefinition::create(
            "default.vert.hlsl",
            spirv_header_gen::generated::default_vert::META);
    static constexpr std::array ALL_SHADERS{
            SHADER_BASIC_TEXTURED,
            SHADER_GOOCH,
            SHADER_VERTEX_COLOR,
            SHADER_DEFAULT_VERTEX,
    };

    // Pipeline definitions

    static constexpr PipelineDefinition PIPELINE_DIFFUSE{
            .mName = PipelineName::Diffuse,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_VERTEX_COLOR,
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
    static constexpr PipelineDefinition PIPELINE_LINES{
            .mName = PipelineName::Lines,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_VERTEX_COLOR,
            .mPrimitiveType = SDL_GPU_PRIMITIVETYPE_LINELIST,
    };
    static constexpr std::array ALL_PIPELINES{
            PIPELINE_DIFFUSE,
            PIPELINE_GOOCH,
            PIPELINE_TEXTURED,
            PIPELINE_LINES,
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
