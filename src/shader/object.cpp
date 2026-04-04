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
        const sdl::SdlTransferBuffer transferBuffer =
                sdl::SdlTransferBuffer::create(context, SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, [&] {
                    auto const v = meshData.getVertexBufferSize();
                    auto const i = meshData.getIndexBufferSize();
                    CHECK_LE(static_cast<uint64_t>(v) + i, std::numeric_limits<uint32_t>::max());
                    return v + i;
                }());

        // Set the vertex/index data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            auto *vertexData = mappedBuffer.get<VertexData>();
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
        if (gpuShaderObject.mIndexBuffer.has_value()) {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = meshData.getVertexBufferSize(),
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = *gpuShaderObject.mIndexBuffer.value(),
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
                                                        image.mTextureType, sdl::DEFAULT_TEXTURE_FORMAT,
                                                        SDL_GPU_TEXTUREUSAGE_SAMPLER, image.mSurfaces.size()));
        }
        return textures;
    }

    GpuShaderObject::GpuShaderObject(sdl::SdlContext const &sdlContext, render::Samplers &samplers,
                                     MeshData const &meshData, std::vector<ImageData> const &imageData) :
        mVertexCount(meshData.mVertices.size()), mIndexCount(meshData.mIndices.count()),
        mVertexBuffer(sdl::SdlGpuBuffer::create(sdlContext,
                                                SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ,
                                                meshData.getVertexBufferSize())),
        mIndexBuffer(sdl::SdlGpuBuffer::create(sdlContext, SDL_GPU_BUFFERUSAGE_INDEX, meshData.getIndexBufferSize())),
        mIndexSize(meshData.mIndices.mElementSize), mTextures(createTextures(sdlContext, imageData)) {
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

    void GpuShaderObject::reallocVertexBuffer(size_t numVertices, SDL_GPUBufferUsageFlags usage) {
        mVertexBuffer = sdl::SdlGpuBuffer::create(mVertexBuffer.mContext, usage, getVertexBufferSize(numVertices));
        mVertexCount = numVertices;
    }

    GpuShaderObject ShaderObject::upload(sdl::SdlContext const &context, render::Samplers &samplers) const {
        GpuShaderObject gpuShaderObject(context, samplers, mMeshData, mImages);
        return gpuShaderObject;
    }

    sdl::SdlMappedTransferBuffer GpuShaderObject::downloadVertexData() const {
        auto const dataSize = util::narrow_u32(mVertexCount * sizeof(VertexData));
        auto const &sdlContext = mVertexBuffer.mContext;
        auto const transferBuffer =
                sdl::SdlTransferBuffer::create(sdlContext, SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, dataSize);
        const SDL_GPUBufferRegion source{
                .buffer = *mVertexBuffer,
                .offset = 0,
                .size = dataSize,
        };
        const SDL_GPUTransferBufferLocation dest{
                .transfer_buffer = *transferBuffer,
                .offset = 0,
        };

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(sdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire GPU command buffer";
        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        SDL_DownloadFromGPUBuffer(copyPass, &source, &dest);
        SDL_EndGPUCopyPass(copyPass);

        {
            const sdl::SdlGpuFence fence{sdlContext, SDL_SubmitGPUCommandBufferAndAcquireFence(commandBuffer)};
            auto const start = std::chrono::steady_clock::now();
            fence.wait(); // TODO: don't block, keep initializing and only wait later
            SDL_Log("Vertex download took %lu us",
                    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start)
                            .count());
        }
        return transferBuffer.map(false);
    }
} // namespace gfx_testing::shader
