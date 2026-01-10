#pragma once

#include <game.hpp>
#include <glm/mat4x4.hpp>
#include <render/camera.hpp>
#include <render/draw_context.hpp>
#include <render/scene_objects.hpp>
#include <sdl.hpp>

namespace gfx_testing::render {
    class Scene {
    public:
        explicit Scene(game::GameContext &gameContext);

        void update();

        void onResize(util::Extent2D extent);

        void draw(DrawContext const &drawContext) const;

        [[nodiscard]] util::Extent2D getViewportExtent() const { return mViewportExtent; };

        Camera &getCamera() { return mCamera; }

        SceneObjects &getSceneObjects() { return mSceneObjects; }

        game::GameContext &getGameContext() const { return mGameContext; }

    private:
        void drawObjects(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const;

        template<pipeline::gfx::PipelineName>
        void drawForPipeline(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass,
                             glm::mat4x4 const &view, std::vector<glm::vec3> const &lightPosWs) const;

        game::GameContext &mGameContext;
        util::Extent2D mViewportExtent;
        Camera mCamera;
        glm::mat4x4 mProjection;
        SceneObjects mSceneObjects;
        sdl::SdlGpuTexture mDepthTexture;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
} // namespace gfx_testing::render
