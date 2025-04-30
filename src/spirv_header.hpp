#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <SDL3/SDL_gpu.h>

namespace gfx_testing::shader {

    struct EntryPoint {
        char const *mName;
        SDL_GPUShaderStage mStage;
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
        std::string mName;
        std::string mType;
        size_t mSet;
        size_t mBinding;
    };

    struct Ssbo {
        // TODO
    };

    struct StorageTexture {
        // TODO
    };
}
