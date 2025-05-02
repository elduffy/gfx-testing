#include <game.hpp>
#include <render_object.hpp>
#include <utility>

namespace gfx_testing::render {


    SDL_GPUBuffer *createBuffer(sdl::SdlContext const &context, SDL_GPUBufferUsageFlags usage, uint32_t size) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = usage,
                .size = size,
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
    }

    template<typename index_t>
    void copyIndexData(sdl::SdlMappedTransferBuffer const &mappedBuffer, shader::MeshData const &meshData) {
        auto *indexData = mappedBuffer.get<index_t>(meshData.getVertexBufferSize());
        auto const *source = meshData.mIndices.as<index_t>();
        std::copy_n(source, meshData.mIndices.count(), indexData);
    }

    void transferBufferData(sdl::SdlContext const &context, shader::MeshData const &meshData,
                            SDL_GPUBuffer *vertexBuffer,
                            SDL_GPUBuffer *indexBuffer) {
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = boost::safe_numerics::checked::add(meshData.getVertexBufferSize(),
                                                           meshData.getIndexBufferSize()),
        };
        const sdl::SdlTransferBuffer transferBuffer{
                context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};

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
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire GPU command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        // Vertex upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = 0,
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = vertexBuffer,
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
                    .buffer = indexBuffer,
                    .offset = 0,
                    .size = meshData.getIndexBufferSize(),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        SDL_EndGPUCopyPass(copyPass);
    }


    void transferTextureData(sdl::SdlContext const &context, SDL_Surface *surface,
                             SDL_GPUTexture *texture) {
        auto const width = boost::safe_numerics::checked::cast<uint32_t>(surface->w);
        auto const height = boost::safe_numerics::checked::cast<uint32_t>(surface->h);
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = width * height * 4,
        };
        const sdl::SdlTransferBuffer transferBuffer{
                context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};
        // Upload pixel data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            auto *imageData = mappedBuffer.get<uint8_t>();
            SDL_memcpy(imageData, surface->pixels, surface->w * surface->h * 4);
        }

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire GPU command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);
        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        SDL_GPUTextureTransferInfo const source = {
                .transfer_buffer = *transferBuffer,
                .offset = 0,
        };
        SDL_GPUTextureRegion const dest = {
                .texture = texture,
                .w = width,
                .h = height,
                .d = 1,
        };
        SDL_UploadToGPUTexture(copyPass, &source, &dest, false);
        SDL_EndGPUCopyPass(copyPass);
    }

    std::optional<TextureAndSampler> createGpuTexture(game::GameContext const &gameContext,
                                                      sdl::SdlSurface const *surface) {
        if (surface == nullptr) {
            return {};
        }

        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = boost::safe_numerics::checked::cast<uint32_t>((**surface)->w),
                .height = boost::safe_numerics::checked::cast<uint32_t>((**surface)->h),
                .layer_count_or_depth = 1,
                .num_levels = 1,
        };
        sdl::SdlGpuTexture tex = {gameContext.mSdlContext,
                                  SDL_CreateGPUTexture(gameContext.mSdlContext.mDevice, &createInfo)};
        if (*tex == nullptr) {
            throw std::runtime_error("Failed to create GPU texture");
        }
        return {TextureAndSampler{.mTexture = std::move(tex), .mSampler = gameContext.mSamplers.mAnisotropicWrap}};
    }

    RenderObject::RenderObject(game::GameContext const &gameContext,
                               shader::MeshData const &meshData, pipeline::PipelineName pipelineName,
                               const glm::mat4 &initialTransform) :
        RenderObject(gameContext, meshData, pipelineName, nullptr, initialTransform) {
        if (pipelineName == pipeline::PipelineName::Textured) {
            throw std::runtime_error("Use constructor with texture data for the Textured pipeline");
        }
    }

    RenderObject::RenderObject(game::GameContext const &gameContext,
                               shader::MeshData const &meshData, sdl::SdlSurface const &textureData,
                               const glm::mat4 &initialTransform) :
        RenderObject(gameContext, meshData, pipeline::PipelineName::Textured, &textureData, initialTransform) {
    }

    RenderObject::RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                               pipeline::PipelineName pipelineName,
                               sdl::SdlSurface const *textureDataOpt,
                               const glm::mat4 &initialTransform):
        mTransform(initialTransform),
        mVertexBuffer(gameContext.mSdlContext,
                      createBuffer(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_VERTEX,
                                   meshData.getVertexBufferSize())),
        mIndexBuffer(gameContext.mSdlContext,
                     createBuffer(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_INDEX, meshData.getIndexBufferSize())),
        mPipelineName(pipelineName),
        mTextureOpt(createGpuTexture(gameContext, textureDataOpt)),
        mIndexCount(meshData.mIndices.count()) {
        transferBufferData(gameContext.mSdlContext, meshData, *mVertexBuffer, *mIndexBuffer);
        if (mTextureOpt.has_value()) {
            assert(textureDataOpt != nullptr);
            transferTextureData(gameContext.mSdlContext, textureDataOpt->mSurface, *mTextureOpt->mTexture);
        }
    }

    void RenderObject::render(SDL_GPURenderPass *renderPass) const {
        const SDL_GPUBufferBinding vertexBufferBinding = {
                .buffer = *mVertexBuffer,
                .offset = 0,
        };
        const SDL_GPUBufferBinding indexBufferBinding = {
                .buffer = *mIndexBuffer,
                .offset = 0,
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

        if (mTextureOpt.has_value()) {
            const SDL_GPUTextureSamplerBinding samplerBinding{
                    .texture = *mTextureOpt->mTexture,
                    .sampler = *mTextureOpt->mSampler,
            };
            SDL_BindGPUFragmentSamplers(renderPass, 0, &samplerBinding, 1);
        }
        SDL_DrawGPUIndexedPrimitives(renderPass, mIndexCount, 1, 0, 0, 0);
    }
}
