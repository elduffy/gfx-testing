#include <absl/log/check.h>
#include <game.hpp>
#include <render/render_object.hpp>
#include <utility>
#include <vector>

namespace gfx_testing::render {
    template<typename index_t>
    void copyIndexData(sdl::SdlMappedTransferBuffer const &mappedBuffer, shader::MeshData const &meshData) {
        auto *indexData = mappedBuffer.get<index_t>(meshData.getVertexBufferSize());
        auto const *source = meshData.mIndices.as<index_t>();
        std::copy_n(source, meshData.mIndices.count(), indexData);
    }

    void transferBufferData(sdl::SdlContext const &context, shader::MeshData const &meshData,
                            SDL_GPUBuffer *vertexBuffer, SDL_GPUBuffer *indexBuffer) {
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

    RenderObject::RenderObject(game::GameContext const &gameContext, shader::ShaderObject shaderObject,
                               pipeline::PipelineName pipelineName, const glm::mat4 &initialTransform) :
        mTransform(initialTransform),
        mVertexBuffer(sdl::SdlGpuBuffer::create(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_VERTEX,
                                                shaderObject.mMeshData.getVertexBufferSize())),
        mIndexBuffer(sdl::SdlGpuBuffer::create(gameContext.mSdlContext, SDL_GPU_BUFFERUSAGE_INDEX,
                                               shaderObject.mMeshData.getIndexBufferSize())),
        mPipelineName(pipelineName), mRenderResources(std::move(shaderObject.mRenderResources)),
        mIndexCount(shaderObject.mMeshData.mIndices.count()) {
        transferBufferData(gameContext.mSdlContext, shaderObject.mMeshData, *mVertexBuffer, *mIndexBuffer);
        for (auto &textureData: mRenderResources.mTextures) {
            CHECK(textureData.mTexture.mUploaded) << "Texture must be uploaded before passing to RenderObject";
            mTextureBindings.push_back({.texture = *textureData.mTexture, .sampler = *textureData.mSampler});
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

        if (!mTextureBindings.empty()) {
            SDL_BindGPUFragmentSamplers(renderPass, 0, mTextureBindings.data(), mTextureBindings.size());
        }
        SDL_DrawGPUIndexedPrimitives(renderPass, mIndexCount, 1, 0, 0, 0);
    }

    void RenderObject::pushPerObjectUniforms(pipeline::PipelineDefinition const &pipelineDefinition,
                                             SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                             std::vector<glm::vec3> const &lightPosWs, Camera const &camera) const {
        if (pipelineDefinition.mVertexShader.mShaderBindings.mMvpTransformBinding.has_value()) {
            glm::mat4 mvpTransform;
            if (mPipelineName == pipeline::PipelineName::Skybox) {
                auto view = camera.mView;
                view[3] = {0, 0, 0, 1};
                mvpTransform = projection * view * mTransform;
            } else {
                mvpTransform = projection * camera.mView * mTransform;
            }
            SDL_PushGPUVertexUniformData(commandBuffer,
                                         *pipelineDefinition.mVertexShader.mShaderBindings.mMvpTransformBinding,
                                         &mvpTransform, sizeof(mvpTransform));
        }
        if (pipelineDefinition.mFragmentShader.mShaderBindings.mObjectLightingBinding.has_value()) {
            auto const objectLighting = shader::ObjectLighting::create(mTransform, lightPosWs, camera.mPosWs);
            SDL_PushGPUFragmentUniformData(commandBuffer,
                                           *pipelineDefinition.mFragmentShader.mShaderBindings.mObjectLightingBinding,
                                           &objectLighting, sizeof(objectLighting));
        }
    }
} // namespace gfx_testing::render
