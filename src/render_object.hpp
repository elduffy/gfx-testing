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
                     const glm::mat4 &initialTransform);

        RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                     sdl::SdlSurface const &textureData,
                     const glm::mat4 &initialTransform);

    private:
        RenderObject(game::GameContext const &gameContext, shader::MeshData const &meshData,
                     sdl::SdlSurface const *textureDataOpt,
                     const glm::mat4 &initialTransform);

    public:
        void render(SDL_GPURenderPass *) const;

        glm::mat4 mTransform;

    private:
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
        std::optional<TextureAndSampler> mTextureOpt = std::nullopt;
        uint32_t mIndexCount;
    };
}
