#pragma once
#include <render/camera.hpp>
#include <sdl.hpp>
#include <shader/object.hpp>

namespace gfx_testing::render {

    class RenderObject {
    public:
        RenderObject(game::GameContext const &gameContext, shader::ShaderObject shaderObject,
                     pipeline::PipelineName pipelineName, const glm::mat4 &initialTransform);


        void render(SDL_GPURenderPass *) const;

        pipeline::PipelineName getPipelineName() const { return mPipelineName; }

        void pushPerObjectUniforms(pipeline::PipelineDefinition const &pipelineDefinition,
                                   SDL_GPUCommandBuffer *commandBuffer, glm::mat4 const &projection,
                                   std::vector<glm::vec3> const &lightPosWs, Camera const &camera) const;

        glm::mat4 mTransform;

    private:
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
        pipeline::PipelineName mPipelineName;
        shader::RenderResources mRenderResources;
        uint32_t mIndexCount;
        std::vector<SDL_GPUTextureSamplerBinding> mTextureBindings;
    };
} // namespace gfx_testing::render
