#pragma once
#include <sdl.hpp>
#include <shader_models.hpp>

namespace gfx_testing::render {
    struct TextureAndSampler {
        sdl::SdlGpuTexture mTexture;
        sdl::SdlGpuSampler const &mSampler;
    };

    class RenderObject {
    public:
        RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                     pipeline::PipelineName pipelineName,
                     const glm::mat4 &initialTransform);

        RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                     sdl::SdlSurface const &textureData,
                     const glm::mat4 &initialTransform);

    private:
        RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                     pipeline::PipelineName pipelineName,
                     sdl::SdlSurface const *textureDataOpt,
                     const glm::mat4 &initialTransform);

    public:
        void render(SDL_GPURenderPass *) const;

        pipeline::PipelineName getPipelineName() const { return mPipelineName; }

        void pushPerObjectUniforms(pipeline::PipelineDefinition const &pipelineDefinition,
                                   SDL_GPUCommandBuffer *commandBuffer,
                                   glm::mat4 const &vewProj, glm::vec3 const &lightPosWs,
                                   glm::vec3 const &cameraPosWs) const;

        glm::mat4 mTransform;

    private:
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
        pipeline::PipelineName mPipelineName;
        std::optional<TextureAndSampler> mTextureOpt = std::nullopt;
        uint32_t mIndexCount;
    };
}
