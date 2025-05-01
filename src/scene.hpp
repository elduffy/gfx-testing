#pragma once

#include <camera.hpp>
#include <sdl.hpp>
#include <game.hpp>
#include <render_object.hpp>
#include <glm/mat4x4.hpp>

#include "debug_axes.hpp"
#include "imgui_context.hpp"
#include "point_light.hpp"

namespace gfx_testing::scene {
    class Scene {
    public:
        Scene(game::GameContext &gameContext, imgui::ImGuiContext &imGuiContext);

        void update();

        void onResize(util::Extent2D extent);

        void draw() const;

        [[nodiscard]] util::Extent2D getViewportExtent() const { return mViewportExtent; };

        render::Camera &getCamera() { return mCamera; }

    private:
        game::GameContext &mGameContext;
        imgui::ImGuiContext &mImGuiContext;
        util::Extent2D mViewportExtent;
        render::Camera mCamera;
        glm::mat4x4 mProjection;
        render::RenderObject mPropObjects;
        render::RenderObject mCube;
        render::RenderObject mTextureObject;
        render::DebugAxes mDebugAxes;
        render::PointLight mPointLight;
        sdl::SdlGpuTexture mDepthTexture;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
}
