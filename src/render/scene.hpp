#pragma once

#include <game.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <render/camera.hpp>
#include <render/debug_axes.hpp>
#include <render/debug_normals.hpp>
#include <render/draw_context.hpp>
#include <render/point_light.hpp>
#include <render/render_object.hpp>
#include <render/sky_box.hpp>
#include <sdl.hpp>

namespace gfx_testing::render {
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 LANDSCAPE_POSITION(0, 0, -15);
    static constexpr glm::vec3 LANDSCAPE_SCALE(20, 20, .5);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);

    class SceneObjects {

    public:
        explicit SceneObjects(game::GameContext &gameContext);

        std::vector<RenderObject const *> const &getRenderObjects(pipeline::gfx::PipelineName pipelineName) const {
            return mRenderObjectsByPipeline.at(pipeline::gfx::getIndex(pipelineName));
        }

        void update();

    private:
        game::GameContext &mGameContext;
        SkyBox mSkyBox;
        RenderObject mPropObjects;
        RenderObject mLandscape;
        RenderObject mTextureObject;
        DebugAxes mDebugAxes;
        DebugNormals mDebugNormals;

    public:
        std::vector<PointLight> mPointLights;

    private:
        std::vector<std::vector<RenderObject const *>> mRenderObjectsByPipeline{pipeline::gfx::ALL_PIPELINES.size()};
    };

    class Scene {
    public:
        explicit Scene(game::GameContext &gameContext);

        void update();

        void onResize(util::Extent2D extent);

        void draw(DrawContext const &drawContext) const;

        [[nodiscard]] util::Extent2D getViewportExtent() const { return mViewportExtent; };

        Camera &getCamera() { return mCamera; }

    private:
        void drawObjects(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const;

        game::GameContext &mGameContext;
        util::Extent2D mViewportExtent;
        Camera mCamera;
        glm::mat4x4 mProjection;
        SceneObjects mSceneObjects;
        sdl::SdlGpuTexture mDepthTexture;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
} // namespace gfx_testing::render
