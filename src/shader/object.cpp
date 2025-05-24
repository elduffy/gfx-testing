#include <sdl_factories.hpp>
#include <shader/object.hpp>


namespace gfx_testing::shader {

    template<typename index_t>
    void copyIndexData(sdl::SdlMappedTransferBuffer const &mappedBuffer, MeshData const &meshData) {
        auto *indexData = mappedBuffer.get<index_t>(meshData.getVertexBufferSize());
        auto const *source = meshData.mIndices.as<index_t>();
        std::copy_n(source, meshData.mIndices.count(), indexData);
    }

    void transferBufferData(sdl::SdlContext const &context, MeshData const &meshData,
                            GpuShaderObject const &gpuShaderObject) {
        const sdl::SdlTransferBuffer transferBuffer = sdl::SdlTransferBuffer::create(
                context, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                boost::safe_numerics::checked::add(meshData.getVertexBufferSize(), meshData.getIndexBufferSize()));

        // Set the vertex/index data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            auto *vertexData = mappedBuffer.get<shader::VertexData>();
            std::ranges::copy(meshData.mVertices, vertexData);

            switch (meshData.mIndices.mElementSize) {
                case SDL_GPU_INDEXELEMENTSIZE_16BIT: {
                    copyIndexData<uint16_t>(mappedBuffer, meshData);
                    break;
                }
                case SDL_GPU_INDEXELEMENTSIZE_32BIT: {
                    copyIndexData<uint32_t>(mappedBuffer, meshData);
                    break;
                }
            }
        }

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        // Vertex upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = 0,
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = *gpuShaderObject.mVertexBuffer,
                    .offset = 0,
                    .size = meshData.getVertexBufferSize(),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        // Index upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = meshData.getVertexBufferSize(),
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = *gpuShaderObject.mIndexBuffer,
                    .offset = 0,
                    .size = meshData.getIndexBufferSize(),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        SDL_EndGPUCopyPass(copyPass);
    }

    std::vector<sdl::SdlGpuTexture> createTextures(sdl::SdlContext const &sdlContext,
                                                   std::vector<ImageData> const &imageData) {
        std::vector<sdl::SdlGpuTexture> textures;
        for (auto const &image: imageData) {
            CHECK(!image.mSurfaces.empty()) << "No surfaces for image data";
            textures.emplace_back(sdl::createGpuTexture(sdlContext, image.mSurfaces.front().getExtent(),
                                                        image.mTextureType, image.mSurfaces.size()));
        }
        return textures;
    }

    GpuShaderObject::GpuShaderObject(sdl::SdlContext const &sdlContext, render::Samplers &samplers,
                                     MeshData const &meshData, std::vector<ImageData> const &imageData) :
        mVertexBuffer(
                sdl::SdlGpuBuffer::create(sdlContext, SDL_GPU_BUFFERUSAGE_VERTEX, meshData.getVertexBufferSize())),
        mIndexBuffer(sdl::SdlGpuBuffer::create(sdlContext, SDL_GPU_BUFFERUSAGE_INDEX, meshData.getIndexBufferSize())),
        mTextures(createTextures(sdlContext, imageData)) {
        transferBufferData(sdlContext, meshData, *this);

        CHECK_EQ(mTextures.size(), imageData.size());
        mTextureSamplerBindings.resize(imageData.size());
        for (size_t i = 0; i < imageData.size(); ++i) {
            auto &texture = mTextures.at(i);
            auto const &image = imageData.at(i);
            texture.upload(image.mSurfaces);

            auto &binding = mTextureSamplerBindings.at(i);
            binding.texture = *texture;
            if (image.mSamplerCreateInfo.has_value()) {
                binding.sampler = *samplers.getOrCreateSampler(image.mSamplerCreateInfo.value());
            } else {
                binding.sampler = *samplers.mAnisotropicWrap;
            }
        }
    }

    GpuShaderObject ShaderObject::upload(sdl::SdlContext const &context, render::Samplers &samplers) const {
        GpuShaderObject gpuShaderObject(context, samplers, mMeshData, mImages);
        return gpuShaderObject;
    }
} // namespace gfx_testing::shader
