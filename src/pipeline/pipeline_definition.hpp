#pragma once

#include <basic_textured.frag.hpp>
#include <default.vert.hpp>
#include <gooch.frag.hpp>
#include <lambert.frag.hpp>
#include <optional>
#include <skybox.frag.hpp>
#include <skybox.vert.hpp>
#include <util/util.hpp>
#include <vertex_color.frag.hpp>

namespace gfx_testing::pipeline {

    enum class PipelineName {
        SimpleColor,
        Gooch,
        Textured,
        Lines,
        Lambert,
        Skybox,
    };

    constexpr size_t getIndex(PipelineName pipelineName) { return static_cast<size_t>(pipelineName); }

    constexpr char const *getName(PipelineName pipelineName) {
        switch (pipelineName) {
            case PipelineName::SimpleColor:
                return "SimpleColor";
            case PipelineName::Gooch:
                return "Gooch";
            case PipelineName::Textured:
                return "Textured";
            case PipelineName::Lines:
                return "Lines";
            case PipelineName::Lambert:
                return "Lambert";
            case PipelineName::Skybox:
                return "Skybox";
        }
        FAIL("Unknown pipeline name '{}'", static_cast<uint32_t>(pipelineName));
    }

    struct ShaderBindings {
        std::optional<uint32_t> mMvpTransformBinding;
        std::optional<uint32_t> mObjectLightingBinding;
    };

    struct ShaderDefinition {
        friend bool operator<(const ShaderDefinition &lhs, const ShaderDefinition &rhs) {
            return lhs.mFilename < rhs.mFilename;
        }

        template<typename SpirvMeta>
        static constexpr ShaderDefinition create(SpirvMeta const &spirvMeta, ShaderBindings shaderBindings) {
            return {
                    .mFilename = spirvMeta.mSourceFilename,
                    .mStage = spirvMeta.mEntryPoint.mStage,
                    .mSamplers = getBindingSlotCount(spirvMeta.mSeparateSamplers),
                    .mUniformBuffers = getBindingSlotCount(spirvMeta.mUbos),
                    .mStorageBuffers = getBindingSlotCount(spirvMeta.mSsbos),
                    .mStorageTextures = getBindingSlotCount(spirvMeta.mStorageTextures),
                    .mShaderBindings = std::move(shaderBindings),
            };
        }

        char const *mFilename;
        SDL_GPUShaderStage mStage;
        uint32_t mSamplers{0};
        uint32_t mUniformBuffers{0};
        uint32_t mStorageBuffers{0};
        uint32_t mStorageTextures{0};
        ShaderBindings mShaderBindings;
    };

    struct PipelineDefinition {
        PipelineName mName;
        ShaderDefinition mVertexShader;
        ShaderDefinition mFragmentShader;
        SDL_GPUPrimitiveType mPrimitiveType{SDL_GPU_PRIMITIVETYPE_TRIANGLELIST};
        SDL_GPUFillMode mFillMode{SDL_GPU_FILLMODE_FILL};
        /**
         * Treat the rendered objects as infinitely distant. E.g. for skybox
         */
        bool mIsBackground = false;
    };

    // Shader definitions

    // Vertex

    static constexpr ShaderDefinition SHADER_DEFAULT_VERTEX = ShaderDefinition::create(
            spirv_header_gen::generated::default_vert::META,
            {.mMvpTransformBinding = spirv_header_gen::generated::default_vert::UBO_MvpTransform.mBinding});

    static constexpr ShaderDefinition SHADER_SKYBOX_VERTEX = ShaderDefinition::create(
            spirv_header_gen::generated::skybox_vert::META,
            {.mMvpTransformBinding = spirv_header_gen::generated::skybox_vert::UBO_MvpTransform.mBinding});

    // Fragment

    static constexpr ShaderDefinition SHADER_BASIC_TEXTURED =
            ShaderDefinition::create(spirv_header_gen::generated::basic_textured_frag::META, {});
    static constexpr ShaderDefinition SHADER_GOOCH = ShaderDefinition::create(
            spirv_header_gen::generated::gooch_frag::META,
            {.mObjectLightingBinding = spirv_header_gen::generated::gooch_frag::UBO_ObjectLighting.mBinding});
    static constexpr ShaderDefinition SHADER_VERTEX_COLOR =
            ShaderDefinition::create(spirv_header_gen::generated::vertex_color_frag::META, {});
    static constexpr ShaderDefinition SHADER_LAMBERT = ShaderDefinition::create(
            spirv_header_gen::generated::lambert_frag::META,
            {.mObjectLightingBinding = spirv_header_gen::generated::lambert_frag::UBO_ObjectLighting.mBinding});
    static constexpr ShaderDefinition SHADER_SKYBOX_FRAGMENT =
            ShaderDefinition::create(spirv_header_gen::generated::skybox_frag::META, {});

    static constexpr std::array ALL_SHADERS{
            // Vertex
            SHADER_DEFAULT_VERTEX,
            SHADER_SKYBOX_VERTEX,
            // Fragment
            SHADER_BASIC_TEXTURED,
            SHADER_GOOCH,
            SHADER_VERTEX_COLOR,
            SHADER_LAMBERT,
            SHADER_SKYBOX_FRAGMENT,
    };
    // Pipeline definitions

    static constexpr PipelineDefinition PIPELINE_SIMPLE_COLOR{
            .mName = PipelineName::SimpleColor,
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
    static constexpr PipelineDefinition PIPELINE_LAMBERT{
            .mName = PipelineName::Lambert,
            .mVertexShader = SHADER_DEFAULT_VERTEX,
            .mFragmentShader = SHADER_LAMBERT,
    };
    static constexpr PipelineDefinition PIPELINE_SKYBOX{
            .mName = PipelineName::Skybox,
            .mVertexShader = SHADER_SKYBOX_VERTEX,
            .mFragmentShader = SHADER_SKYBOX_FRAGMENT,
            .mIsBackground = true,
    };
    static constexpr std::array ALL_PIPELINES{PIPELINE_SIMPLE_COLOR, PIPELINE_GOOCH,   PIPELINE_TEXTURED,
                                              PIPELINE_LINES,        PIPELINE_LAMBERT, PIPELINE_SKYBOX};
} // namespace gfx_testing::pipeline
