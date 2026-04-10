#include <utility>
#include <vector>

#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    RenderObject::RenderObject(game::GameContext &gameContext, shader::ShaderObject const &shaderObject,
                               pipeline::gfx::PipelineName pipelineName, const glm::mat4 &initialTransform) :
        mTransform(initialTransform), mPipelineName(pipelineName),
        mGpuShaderObject(shaderObject.upload(gameContext.mSdlContext, gameContext.mSamplers)) {}

    RenderObject::RenderObject(shader::GpuShaderObject gpuShaderObject, pipeline::gfx::PipelineName pipelineName,
                               const glm::mat4 &initialTransform) :
        mTransform(initialTransform), mPipelineName(pipelineName), mGpuShaderObject(std::move(gpuShaderObject)) {}

    void RenderObject::render(SDL_GPURenderPass *renderPass) const {
        const SDL_GPUBufferBinding vertexBufferBinding = {
                .buffer = *mGpuShaderObject.mVertexBuffer,
                .offset = 0,
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

        if (!mGpuShaderObject.mTextureSamplerBindings.empty()) {
            SDL_BindGPUFragmentSamplers(renderPass, 0, mGpuShaderObject.mTextureSamplerBindings.data(),
                                        mGpuShaderObject.mTextureSamplerBindings.size());
        }
        if (mGpuShaderObject.mIndexBuffer.has_value()) {
            const SDL_GPUBufferBinding indexBufferBinding = {
                    .buffer = *mGpuShaderObject.mIndexBuffer.value(),
                    .offset = 0,
            };
            SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, mGpuShaderObject.mIndexSize);
            SDL_DrawGPUIndexedPrimitives(renderPass, mGpuShaderObject.mIndexCount, 1, 0, 0, 0);
        } else {
            SDL_DrawGPUPrimitives(renderPass, mGpuShaderObject.mVertexCount, 1, 0, 0);
        }
    }

    void RenderObject::pushPerObjectUniforms(pipeline::gfx::PipelineDefinition const &pipelineDefinition,
                                             SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                             glm::mat4 const &view, std::vector<glm::vec3> const &lightPosWs,
                                             glm::vec3 const &cameraPosWs) const {
        if (pipelineDefinition.mVertexShader.mShaderBindings.mMvpTransformBinding.has_value()) {

            glm::mat4 mvpTransform;
            if (pipelineDefinition.mIsBackground) {
                auto newView = view;
                newView[3] = {0, 0, 0, 1};
                mvpTransform = projection * newView * mTransform;
            } else {
                mvpTransform = projection * view * mTransform;
            }

            SDL_PushGPUVertexUniformData(commandBuffer,
                                         *pipelineDefinition.mVertexShader.mShaderBindings.mMvpTransformBinding,
                                         &mvpTransform, sizeof(mvpTransform));
        }
        if (pipelineDefinition.mFragmentShader.mShaderBindings.mObjectLightingBinding.has_value()) {
            auto const objectLighting = shader::ObjectLighting::create(mTransform, lightPosWs, cameraPosWs);
            SDL_PushGPUFragmentUniformData(commandBuffer,
                                           *pipelineDefinition.mFragmentShader.mShaderBindings.mObjectLightingBinding,
                                           &objectLighting, sizeof(objectLighting));
        }
    }
} // namespace gfx_testing::render
