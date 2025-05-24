#pragma once
#include <render/camera.hpp>
#include <sdl.hpp>
#include <shader/object.hpp>

namespace gfx_testing::render {

    class RenderObject {
    public:
        RenderObject(game::GameContext &gameContext, shader::ShaderObject shaderObject,
                     pipeline::PipelineName pipelineName, const glm::mat4 &initialTransform);


        void render(SDL_GPURenderPass *) const;

        pipeline::PipelineName getPipelineName() const { return mPipelineName; }

        void pushPerObjectUniforms(pipeline::PipelineDefinition const &pipelineDefinition,
                                   SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                   std::vector<glm::vec3> const &lightPosWs, Camera const &camera) const;

        glm::mat4 mTransform;

    private:
        pipeline::PipelineName mPipelineName;
        shader::GpuShaderObject mGpuShaderObject;
        uint32_t mIndexCount;
    };
} // namespace gfx_testing::render
