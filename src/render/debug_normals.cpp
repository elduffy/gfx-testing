#include <render/debug_normals.hpp>

namespace gfx_testing::render {
    sdl::SdlMappedTransferBuffer downloadVertexData(sdl::SdlContext const &sdlContext,
                                                    RenderObject const &targetObject) {

        auto const dataSize = boost::safe_numerics::checked::cast<uint32_t>(targetObject.getVertexCount() *
                                                                            sizeof(shader::VertexData));
        auto const transferBuffer =
                sdl::SdlTransferBuffer::create(sdlContext, SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD, dataSize);
        const SDL_GPUBufferRegion source{
                .buffer = *targetObject.getGpuShaderObject().mVertexBuffer,
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
            SDL_Log("Debug normals: vertex download took %lu us",
                    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start)
                            .count());
        }
        return transferBuffer.map(false);
    }

    shader::ShaderObject createShaderObject(sdl::SdlContext const &sdlContext, RenderObject const &targetObject,
                                            DebugNormals::Options const &options) {
        shader::MeshDataBuilder builder;
        const size_t targetVertexCount = targetObject.getVertexCount();
        builder.mVertices.reserve(2 * targetVertexCount);
        const sdl::SdlMappedTransferBuffer mappedBuffer = downloadVertexData(sdlContext, targetObject);
        shader::VertexData const *vertexData = mappedBuffer.get<shader::VertexData>();

        std::copy_n(vertexData, targetVertexCount, std::back_inserter(builder.mVertices));
        for (uint32_t i = 0; i < targetVertexCount; i++) {
            auto &v0 = builder.mVertices[i];
            v0.mColor = options.mLineColor;
            builder.mVertices.push_back({
                    .mPosition = v0.mPosition + options.mLineLength * v0.mNormal,
                    .mColor = v0.mColor,
            });
            builder.addIndex(i);
            builder.addIndex(builder.mVertices.size() - 1);
        }

        return {builder.build(), {}};
    }

    DebugNormals::DebugNormals(game::GameContext &gameContext, RenderObject &targetObject, Options const &options) :
        mSdlContext(gameContext.mSdlContext),
        mRenderObject(gameContext, createShaderObject(mSdlContext, targetObject, options),
                      pipeline::gfx::PipelineName::Lines, targetObject.mTransform),
        mTargetObject(targetObject) {}

    void DebugNormals::update() { mRenderObject.mTransform = mTargetObject.mTransform; }
} // namespace gfx_testing::render
