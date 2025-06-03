#pragma once
#include <render/samplers.hpp>
#include <sdl.hpp>
#include <shader/mesh.hpp>

namespace gfx_testing::shader {
    // Pre-upload texture information
    struct ImageData {
        NO_COPY_DEFAULT_MOVE(ImageData);

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

        SDL_GPUTextureType mTextureType;
        std::vector<sdl::SdlSurface> mSurfaces;

        // If not present, a default will be used
        std::optional<SDL_GPUSamplerCreateInfo> mSamplerCreateInfo;
    };

    // Post-upload shader resources
    struct GpuShaderObject {
        NO_COPY_DEFAULT_MOVE(GpuShaderObject);

        explicit GpuShaderObject(sdl::SdlContext const &sdlContext) :
            mVertexBuffer(sdlContext, nullptr), mIndexBuffer(std::nullopt) {}

        // ctor that handles uploading resources to the gpu
        GpuShaderObject(sdl::SdlContext const &sdlContext, render::Samplers &samplers, MeshData const &meshData,
                        std::vector<ImageData> const &imageData);

        void reallocVertexBuffer(size_t numVertices, SDL_GPUBufferUsageFlags usage);

        sdl::SdlMappedTransferBuffer downloadVertexData() const;

        size_t mVertexCount{0};
        uint32_t mIndexCount{0};
        sdl::SdlGpuBuffer mVertexBuffer;
        // if present, index-based draws are used
        std::optional<sdl::SdlGpuBuffer> mIndexBuffer;
        SDL_GPUIndexElementSize const mIndexSize{SDL_GPU_INDEXELEMENTSIZE_16BIT};
        std::vector<sdl::SdlGpuTexture> mTextures;
        std::vector<SDL_GPUTextureSamplerBinding> mTextureSamplerBindings;
    };

    // Pre-upload shader resources
    struct ShaderObject {
        NO_COPY_DEFAULT_MOVE(ShaderObject);

        ShaderObject(MeshData meshData, std::vector<ImageData> images) :
            mMeshData(std::move(meshData)), mImages(std::move(images)) {}
        explicit ShaderObject(MeshData meshData) : mMeshData(std::move(meshData)) {}

        GpuShaderObject upload(sdl::SdlContext const &context, render::Samplers &samplers) const;

        MeshData mMeshData;
        std::vector<ImageData> mImages;
    };
} // namespace gfx_testing::shader
