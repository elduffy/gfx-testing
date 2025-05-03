#pragma once

#include <camera.hpp>
#include <sdl.hpp>
#include <game.hpp>
#include <render_object.hpp>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "debug_axes.hpp"
#include "imgui_context.hpp"
#include "point_light.hpp"

namespace gfx_testing::scene {
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 CUBE_POSITION(3, 3, -1);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);

    class SceneObjects {

    public:
        explicit SceneObjects(game::GameContext &gameContext) :
            mGameContext(gameContext),
            mDebugAxes(gameContext),
            mPointLight(gameContext, INITIAL_LIGHT_POSITION),
            mPropObjects(gameContext,
                         gameContext.mResourceLoader.loadObjModel(
                                 "basic-shapes.obj", model::NormalTreatment::SPLIT),
                         pipeline::PipelineName::Gooch,
                         translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION)),
            mCube(gameContext,
                  gameContext.mResourceLoader.loadObjModel(
                          "cube.obj", model::NormalTreatment::SPLIT),
                  pipeline::PipelineName::Gooch,
                  glm::translate(glm::identity<glm::mat4>(), CUBE_POSITION)),
            mTextureObject(gameContext,
                           gameContext.mResourceLoader.loadObjModel(
                                   "viking-room.obj", model::NormalTreatment::SPLIT),
                           gameContext.mResourceLoader.loadTexture("viking-room.png"),
                           glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION),
                                      TEXTURE_OBJECT_SCALE)) {
            for (auto const *objPtr: {&mDebugAxes.mRenderObject, &mPointLight.mRenderObject, &mPropObjects, &mCube,
                                      &mTextureObject}) {
                mRenderObjectsByPipeline.at(pipeline::getIndex(objPtr->getPipelineName())).push_back(objPtr);
            }
        }

        std::vector<render::RenderObject const *> const &getRenderObjects(pipeline::PipelineName pipelineName) const {
            return mRenderObjectsByPipeline.at(pipeline::getIndex(pipelineName));
        }

        void update() {
            constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

            mPropObjects.mTransform = rotate(mPropObjects.mTransform,
                                             mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                                             glm::vec3(0, 0, 1));
            mCube.mTransform = rotate(mCube.mTransform,
                                      -mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND * 2,
                                      glm::vec3(0, 0, 1));
            mPointLight.update();
        }

    private:
        game::GameContext &mGameContext;
        render::RenderObject mPropObjects;
        render::RenderObject mCube;
        render::RenderObject mTextureObject;
        render::DebugAxes mDebugAxes;

    public:
        render::PointLight mPointLight;

    private:
        std::vector<std::vector<render::RenderObject const *> > mRenderObjectsByPipeline
                {pipeline::ALL_PIPELINES.size()};
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
}
