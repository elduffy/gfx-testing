#pragma once
#include <game.hpp>
#include <pipeline/gfx/pipeline_definition.hpp>
#include <render/camera.hpp>
#include <sdl.hpp>

namespace gfx_testing::render {

    class RenderObject {
    public:
        // Constructor for mesh data not already uploaded
        RenderObject(game::GameContext &gameContext, shader::ShaderObject const &shaderObject,
                     pipeline::gfx::PipelineName pipelineName, const glm::mat4 &initialTransform);

        // Constructor for mesh data already uploaded to GPU
        RenderObject(shader::GpuShaderObject gpuShaderObject, pipeline::gfx::PipelineName pipelineName,
                     const glm::mat4 &initialTransform);

        void render(SDL_GPURenderPass *) const;

        pipeline::gfx::PipelineName getPipelineName() const { return mPipelineName; }

        shader::GpuShaderObject const &getGpuShaderObject() const { return mGpuShaderObject; }

        size_t getVertexCount() const { return mGpuShaderObject.mVertexCount; }

        void pushPerObjectUniforms(pipeline::gfx::PipelineDefinition const &pipelineDefinition,
                                   SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                   std::vector<glm::vec3> const &lightPosWs, Camera const &camera) const;

        glm::mat4 mTransform;

    private:
        pipeline::gfx::PipelineName mPipelineName;
        shader::GpuShaderObject mGpuShaderObject;
    };
} // namespace gfx_testing::render
