#pragma once
#include <sdl.hpp>
#include <shader/mesh.hpp>

namespace gfx_testing::shader {
    struct TextureData {
        NO_COPY(TextureData);

        TextureData(sdl::SdlGpuTexture texture, sdl::SdlGpuSampler const &sampler) :
            mSampler(sampler), mTexture(std::move(texture)) {}
        TextureData(TextureData &&) = default;

        sdl::SdlGpuSampler const &mSampler;
        sdl::SdlGpuTexture mTexture;
    };

    // Long-lived resources needed to render the object
    struct RenderResources {
        NO_COPY(RenderResources);
        RenderResources() = default;
        RenderResources(RenderResources &&) = default;
        std::vector<TextureData> mTextures;
    };

    struct ShaderObject {
        NO_COPY(ShaderObject);

        ShaderObject(MeshData meshData, RenderResources renderResources) :
            mMeshData(std::move(meshData)), mRenderResources(std::move(renderResources)) {}
        explicit ShaderObject(MeshData meshData) : mMeshData(std::move(meshData)) {}
        ShaderObject(ShaderObject &&) = default;

        MeshData mMeshData;
        RenderResources mRenderResources;
    };
} // namespace gfx_testing::shader
