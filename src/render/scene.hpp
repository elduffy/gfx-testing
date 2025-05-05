#pragma once

#include <game.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <imgui_context.hpp>
#include <render/camera.hpp>
#include <render/debug_axes.hpp>
#include <render/point_light.hpp>
#include <render/render_object.hpp>
#include <sdl.hpp>

namespace gfx_testing::render {
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 CUBE_POSITION(3, 3, -1);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);

    class SceneObjects {

    public:
        explicit SceneObjects(game::GameContext &gameContext);

        std::vector<render::RenderObject const *> const &getRenderObjects(pipeline::PipelineName pipelineName) const {
            return mRenderObjectsByPipeline.at(pipeline::getIndex(pipelineName));
        }

        void update();

    private:
        game::GameContext &mGameContext;
        render::RenderObject mPropObjects;
        render::RenderObject mCube;
        render::RenderObject mTextureObject;
        render::DebugAxes mDebugAxes;

    public:
        render::PointLight mPointLight;

    private:
        std::vector<std::vector<render::RenderObject const *>> mRenderObjectsByPipeline{pipeline::ALL_PIPELINES.size()};
    };

    class Scene {
    public:
        Scene(game::GameContext &gameContext, imgui::ImGuiContext &imGuiContext);

        void update();

        void onResize(util::Extent2D extent);

        void draw() const;

        [[nodiscard]] util::Extent2D getViewportExtent() const { return mViewportExtent; };

        render::Camera &getCamera() { return mCamera; }

    private:
        void drawObjects(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const;

        game::GameContext &mGameContext;
        imgui::ImGuiContext &mImGuiContext;
        util::Extent2D mViewportExtent;
        render::Camera mCamera;
        glm::mat4x4 mProjection;
        SceneObjects mSceneObjects;
        sdl::SdlGpuTexture mDepthTexture;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
} // namespace gfx_testing::render
