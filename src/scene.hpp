#pragma once

#include <sdl.hpp>
#include <filesystem>
#include <game.hpp>
#include <shader_models.hpp>
#include <glm/mat4x4.hpp>

namespace gfx_testing::scene {
    class Scene {
    public:
        Scene(game::GameContext const &gameContext, std::filesystem::path const &projectRoot);

        void update();

        void onResize(util::Extent2D extent);

        void draw() const;

    private:
        game::GameContext const &mGameContext;
        glm::mat4x4 mProjection;
        glm::mat4x4 mView;
        glm::mat4x4 mModel;
        shader::MeshData mMeshData; // TODO we don't need this to match the lifetime of the Scene
        sdl::SdlGfxPipeline mPipeline;
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
    };
}
