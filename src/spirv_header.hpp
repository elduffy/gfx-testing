#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace gfx_testing::shader {

    enum class ShaderMode {
        Vertex,
        Fragment,
    };

    struct EntryPoint {
        std::string mName;
        ShaderMode mMode;
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

    struct SpirvMeta {
        std::vector<EntryPoint> mEntryPoints;
        std::map<std::string, Type> mTypes;
        std::vector<Input> mInputs;
        std::vector<Output> mOutputs;
        std::vector<Ubo> mUbos;
        std::vector<SeparateImage> mSeparateImages;
        std::vector<SeparateSampler> mSeparateSamplers;
    };

}
