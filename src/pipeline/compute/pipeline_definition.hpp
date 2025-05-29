#pragma once

#include <debug_normals.comp.hpp>
#include <util/util.hpp>

namespace gfx_testing::pipeline::compute {

    enum class PipelineName {
        DebugNormals,
    };

    constexpr size_t getIndex(PipelineName pipelineName) { return static_cast<size_t>(pipelineName); }

    constexpr char const *getName(PipelineName pipelineName) {
        switch (pipelineName) {
            case PipelineName::DebugNormals:
                return "DebugNormals";
        }
        FAIL("Unknown pipeline name '{}'", static_cast<uint32_t>(pipelineName));
    }


    struct ShaderDefinition {
        friend bool operator<(const ShaderDefinition &lhs, const ShaderDefinition &rhs) {
            return lhs.mFilename < rhs.mFilename;
        }

        char const *mFilename;
        // uint32_t mSamplers{0};
        // uint32_t mUniformBuffers{0};
        uint32_t mReadwriteStorageBuffers{0};
        uint32_t mReadonlyStorageBuffers{0};
        // uint32_t mStorageTextures{0};
        // ShaderBindings mShaderBindings;
        std::array<uint32_t, 3> mWorkgroupSize{0, 0, 0};
    };

    struct PipelineDefinition {
        PipelineName mName;
        ShaderDefinition mShader;
    };

    template<typename Bindables>
    constexpr uint32_t getBindingSlotCount(Bindables const &bindables, bool readOnly) {
        size_t result = 0;
        for (auto const &b: bindables) {
            if (readOnly != b.mReadOnly) {
                continue;
            }
            result = std::max(result, b.mBinding + 1);
        }
        return boost::safe_numerics::checked::cast<uint32_t>(result);
    }

    static constexpr auto SHADER_DEBUG_NORMALS = ShaderDefinition{
            .mFilename = spirv_header_gen::generated::debug_normals_comp::META.mSourceFilename,
            .mReadwriteStorageBuffers =
                    getBindingSlotCount(spirv_header_gen::generated::debug_normals_comp::META.mSsbos, false),
            .mReadonlyStorageBuffers =
                    getBindingSlotCount(spirv_header_gen::generated::debug_normals_comp::META.mSsbos, true),
            .mWorkgroupSize = spirv_header_gen::generated::debug_normals_comp::META.mEntryPoint.mWorkgroupSize,
    };

    static constexpr std::array ALL_SHADERS{
            SHADER_DEBUG_NORMALS,
    };

    static constexpr PipelineDefinition PIPELINE_DEBUG_NORMALS{
            .mName = PipelineName::DebugNormals,
            .mShader = SHADER_DEBUG_NORMALS,
    };

    static constexpr std::array ALL_PIPELINES{
            PIPELINE_DEBUG_NORMALS,
    };
} // namespace gfx_testing::pipeline::compute
