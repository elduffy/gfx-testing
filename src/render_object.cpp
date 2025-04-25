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

            auto *indexData = mappedBuffer.get<uint16_t>(meshData.getVertexBufferSize());
            std::ranges::copy(meshData.mIndices, indexData);
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

    RenderObject::RenderObject(game::GameContext const &gameContext,
                               shader::MeshData const &meshData, const glm::mat4 &initialTransform):
        mTransform(initialTransform),
        mVertexBuffer(gameContext.mSdlContext,
                      createBuffer(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_VERTEX,
                                   meshData.getVertexBufferSize())),
        mIndexBuffer(gameContext.mSdlContext,
                     createBuffer(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_INDEX, meshData.getIndexBufferSize())),
        mIndexCount(meshData.mIndices.size()) {
        transferBufferData(gameContext.mSdlContext, meshData, *mVertexBuffer, *mIndexBuffer);
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
        SDL_DrawGPUIndexedPrimitives(renderPass, mIndexCount, 1, 0, 0, 0);
    }
}
