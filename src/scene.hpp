#pragma once

#include <camera.hpp>
#include <sdl.hpp>
#include <filesystem>
#include <game.hpp>
#include <render_object.hpp>
#include <glm/mat4x4.hpp>

#include "point_light.hpp"

namespace gfx_testing::scene {
    class Scene {
    public:
        explicit Scene(game::GameContext &gameContext);

        void update();

        void onResize(util::Extent2D extent);

        void draw() const;

        [[nodiscard]] util::Extent2D getViewportExtent() const { return mViewportExtent; };

        render::Camera &getCamera() { return mCamera; }

    private:
        game::GameContext &mGameContext;
        util::Extent2D mViewportExtent;
        render::Camera mCamera;
        glm::mat4x4 mProjection;
        render::RenderObject mPropObjects;
        render::RenderObject mTextureObject;
        render::RenderObject mDebugAxes;
        render::PointLight mPointLight;
        sdl::SdlGpuTexture mDepthTexture;
    };
}
