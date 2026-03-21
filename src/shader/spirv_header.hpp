#pragma once
#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <array>
#include <util/util.hpp>
#include <variant>
#include <vector>

namespace gfx_testing::shader {

    enum class ShaderType {
        Vertex,
        Fragment,
        Compute,
    };

    constexpr static SDL_GPUShaderStage getGpuShaderStage(ShaderType type) {
        switch (type) {
            case ShaderType::Vertex:
                return SDL_GPU_SHADERSTAGE_VERTEX;
            case ShaderType::Fragment:
                return SDL_GPU_SHADERSTAGE_FRAGMENT;
            case ShaderType::Compute:
                break;
        }
        FAIL("Not a GPU shader type: {}", static_cast<uint32_t>(type));
    }

    struct EntryPoint {
        char const *mName;
        ShaderType mType;
        // Only set for compute shaders
        std::array<uint32_t, 3> mWorkgroupSize;
    };

    struct MatrixData {
        size_t mMaxtrixStride;
        bool mRowMajor;
    };

    using TypeSpecificData = std::variant<MatrixData>;

    struct TypeMember {
        char const *mName;
        char const *mType;
        size_t mOffset;

        TypeSpecificData mTypeData;
    };

    template<size_t NUM_MEMBERS>
    struct KeyedType {
        char const *mKey;
        char const *mName;
        std::array<TypeMember, NUM_MEMBERS> mMembers;
    };

    struct KeyedTypeDynamic {
        char const *mKey;
        char const *mName;
        std::vector<TypeMember> mMembers;

        template<size_t N>
        static KeyedTypeDynamic fromStatic(KeyedType<N> const &type) {
            KeyedTypeDynamic result{
                    .mKey = type.mKey,
                    .mName = type.mName,
            };
            std::copy(type.mMembers.begin(), type.mMembers.end(), std::back_inserter(result.mMembers));
            return result;
        }
    };

    struct Input {
        char const *mName;
        char const *mType;
        size_t mLocation;
    };

    struct Output {
        char const *mName;
        char const *mType;
        size_t mLocation;
    };

    struct Ubo {
        char const *mName;
        // Refers to the key in the type map
        char const *mType;
        size_t mBlockSize;
        size_t mSet;
        size_t mBinding;
    };

    struct SeparateImage {
        char const *mName;
        char const *mType;
        size_t mSet;
        size_t mBinding;
    };

    struct SeparateSampler {
        char const *mName;
        char const *mType;
        size_t mSet;
        size_t mBinding;
    };

    struct Ssbo {
        char const *mName;
        // Refers to the key in the type map
        char const *mType;
        size_t mBlockSize;
        size_t mSet;
        size_t mBinding;
        bool mReadOnly;
    };

    struct StorageTexture {
        // TODO
        size_t mBinding;
    };

    template<typename Bindables>
    constexpr uint32_t getBindingSlotCount(Bindables const &bindables) {
        size_t result = 0;
        for (auto const &b: bindables) {
            result = std::max(result, b.mBinding + 1);
        }
        return static_cast<uint32_t>(result);
    }
} // namespace gfx_testing::shader
