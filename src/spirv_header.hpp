#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <SDL3/SDL_gpu.h>

namespace gfx_testing::shader {

    struct EntryPoint {
        std::string mName;
        SDL_GPUShaderStage mStage;
    };

    struct MatrixData {
        size_t mMaxtrixStride;
        bool mRowMajor;
    };

    using TypeSpecificData = std::variant<MatrixData>;

    struct TypeMember {
        std::string mName;
        std::string mType;
        size_t mOffset;

        TypeSpecificData mTypeData;
    };

    struct Type {
        std::string mName;
        std::vector<TypeMember> mMembers;
    };

    struct Input {
        std::string mName;
        std::string mType;
        size_t mLocation;
    };

    struct Output {
        std::string mName;
        std::string mType;
        size_t mLocation;
    };

    struct Ubo {
        std::string mName;
        // Refers to the key in the type map
        std::string mType;
        size_t mBlockSize;
        size_t mSet;
        size_t mBinding;
    };

    struct SeparateImage {
        std::string mName;
        std::string mType;
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

    struct SpirvMeta {
        std::vector<EntryPoint> mEntryPoints;
        std::map<std::string, Type> mTypes;
        std::vector<Input> mInputs;
        std::vector<Output> mOutputs;
        std::vector<Ubo> mUbos;
        std::vector<SeparateImage> mSeparateImages;
        std::vector<SeparateSampler> mSeparateSamplers;
        std::vector<Ssbo> mSsbos;
        std::vector<StorageTexture> mStorageTextures;
    };

    struct SpirvConsts {
        uint32_t mEntryPointCount;
        uint32_t mTypeCount;
        uint32_t mInputCount;
        uint32_t mOutputCount;
        uint32_t mUboCount;
        uint32_t mSeparateImageCount;
        uint32_t mSeparateSamplerCount;
        uint32_t mSsboCount;
        uint32_t mStorageTextureCount;
    };

}
