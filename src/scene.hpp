#pragma once

#include <sdl.hpp>
#include <filesystem>
#include <game.hpp>
#include <render_object.hpp>
#include <glm/mat4x4.hpp>

namespace gfx_testing::scene {
    class Scene {
    public:
        Scene(game::GameContext &gameContext, std::filesystem::path const &projectRoot);

        void update();

        void onResize(util::Extent2D extent);

        void pivotCamera(glm::vec2 const &radians);

        void draw() const;

        [[nodiscard]] util::Extent2D getViewportExtent() const;

    private:
        [[nodiscard]] glm::vec3 getLightPosition() const;

        game::GameContext &mGameContext;
        util::Extent2D mViewportExtent;
        glm::vec3 mCameraPosWs;
        glm::mat4x4 mProjection;
        glm::mat4x4 mView;
        render::RenderObject mRenderObject;
        render::RenderObject mDebugAxes;
        render::RenderObject mPointLight;
        sdl::SdlGpuTexture mDepthTexture;
    };
}
