#pragma once
#include <render/samplers.hpp>
#include <sdl.hpp>
#include <shader/mesh.hpp>

namespace gfx_testing::shader {
    // Pre-upload texture information
    struct ImageData {
        NO_COPY(ImageData);

    private:
        void checkType() const {
            switch (mTextureType) {
                case SDL_GPU_TEXTURETYPE_2D:
                    CHECK_EQ(mSurfaces.size(), 1) << "2D texture should have 1 surface, has " << mSurfaces.size();
                    break;
                case SDL_GPU_TEXTURETYPE_CUBE:
                    CHECK_EQ(mSurfaces.size(), 6) << "Cube texture should have 6 surfaces, has " << mSurfaces.size();
                    break;
                case SDL_GPU_TEXTURETYPE_2D_ARRAY:
                case SDL_GPU_TEXTURETYPE_3D:
                case SDL_GPU_TEXTURETYPE_CUBE_ARRAY:
                    break;
            }
        }

    public:
        explicit ImageData(SDL_GPUTextureType type, std::vector<sdl::SdlSurface> surfaces) :
            mTextureType(type), mSurfaces(std::move(surfaces)), mSamplerCreateInfo(std::nullopt) {
            checkType();
        }

        ImageData(SDL_GPUTextureType type, std::vector<sdl::SdlSurface> surfaces,
                  SDL_GPUSamplerCreateInfo samplerCreateInfo) :
            mTextureType(type), mSurfaces(std::move(surfaces)), mSamplerCreateInfo(std::move(samplerCreateInfo)) {
            checkType();
        }
        ImageData(ImageData &&) = default;

        SDL_GPUTextureType mTextureType;
        std::vector<sdl::SdlSurface> mSurfaces;

        // If not present, a default will be used
        std::optional<SDL_GPUSamplerCreateInfo> mSamplerCreateInfo;
    };

    // Post-upload shader resources
    struct GpuShaderObject {
        NO_COPY(GpuShaderObject);

        GpuShaderObject(sdl::SdlContext const &sdlContext, render::Samplers &samplers, MeshData const &meshData,
                        std::vector<ImageData> const &imageData);
        GpuShaderObject(GpuShaderObject &&) = default;

        size_t mVertexCount;
        uint32_t mIndexCount;
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
        std::vector<sdl::SdlGpuTexture> mTextures;
        std::vector<SDL_GPUTextureSamplerBinding> mTextureSamplerBindings;
    };

    // Pre-upload shader resources
    struct ShaderObject {
        NO_COPY(ShaderObject);

        ShaderObject(MeshData meshData, std::vector<ImageData> images) :
            mMeshData(std::move(meshData)), mImages(std::move(images)) {}
        explicit ShaderObject(MeshData meshData) : mMeshData(std::move(meshData)) {}
        ShaderObject(ShaderObject &&) = default;

        GpuShaderObject upload(sdl::SdlContext const &context, render::Samplers &samplers) const;

        MeshData mMeshData;
        std::vector<ImageData> mImages;
    };
} // namespace gfx_testing::shader
