#pragma once
#include <sdl.hpp>
#include <shader_models.hpp>

namespace gfx_testing::render {
    class RenderObject {
    public:
        explicit RenderObject(game::GameContext const &gameContext,
                              shader::MeshData const &meshData, const glm::mat4 &initialTransform);

        void render(SDL_GPURenderPass *) const;

        glm::mat4 mTransform;

    private:
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
        uint32_t mIndexCount;
    };
}
