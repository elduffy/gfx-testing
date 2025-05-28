#pragma once
#include <render/camera.hpp>
#include <sdl.hpp>
#include <shader/object.hpp>

namespace gfx_testing::render {

    class RenderObject {
    public:
        RenderObject(game::GameContext &gameContext, shader::ShaderObject const &shaderObject,
                     pipeline::gfx::PipelineName pipelineName, const glm::mat4 &initialTransform);

        void render(SDL_GPURenderPass *) const;

        pipeline::gfx::PipelineName getPipelineName() const { return mPipelineName; }

        shader::GpuShaderObject const &getGpuShaderObject() const { return mGpuShaderObject; }

        size_t getVertexCount() const { return mVertexCount; }

        void pushPerObjectUniforms(pipeline::gfx::PipelineDefinition const &pipelineDefinition,
                                   SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                   std::vector<glm::vec3> const &lightPosWs, Camera const &camera) const;

        glm::mat4 mTransform;

    private:
        pipeline::gfx::PipelineName mPipelineName;
        shader::GpuShaderObject mGpuShaderObject;
        size_t mVertexCount;
        uint32_t mIndexCount;
    };
} // namespace gfx_testing::render
