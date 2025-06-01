#include <absl/log/check.h>
#include <array>
#include <boost/scope/scope_exit.hpp>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <pipeline/pipelines.hpp>
#include <render/samplers.hpp>
#include <render/scene.hpp>
#include <sdl_factories.hpp>
#include <tiny_obj_loader.h>
#include <util/util.hpp>


namespace gfx_testing::render {

    static constexpr glm::vec3 INITIAL_CAMERA_POSITION(5, 5, 5);
    static constexpr util::AttribTreatment UNTEXTURED_ATTRIB_TREATMENT{
            .mNormal = util::NormalTreatment::SPLIT,
            .mTexCoord = util::TexCoordTreatment::DISCARD,
    };

    glm::mat4x4 getProjection(const util::Extent2D extent) {
        auto const aspect = static_cast<float>(extent.mWidth) / static_cast<float>(extent.mHeight);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 1000.0f);
    }

    std::vector<PointLight> initPointLights(game::GameContext &gameContext) {
        constexpr auto NUM_POINT_LIGHTS = 3;
        auto const shaderObject = PointLight::loadShaderObject(gameContext.mResourceLoader);
        std::vector<PointLight> pointLights;
        pointLights.reserve(NUM_POINT_LIGHTS);
        for (auto i = 0; i < NUM_POINT_LIGHTS; ++i) {
            auto const phase = (2 * glm::pi<float>() * static_cast<float>(i)) / NUM_POINT_LIGHTS;
            pointLights.emplace_back(gameContext, shaderObject, glm::length(INITIAL_LIGHT_POSITION), phase);
        }
        return pointLights;
    }

    SceneObjects::SceneObjects(game::GameContext &gameContext) :
        mGameContext(gameContext), mSkyBox(gameContext, gameContext.mResourceLoader.loadCubeMap("field")),
        mPropObjects(gameContext,
                     gameContext.mResourceLoader.loadGltfModel("basic-shapes.glb", UNTEXTURED_ATTRIB_TREATMENT),
                     pipeline::gfx::PipelineName::Gooch, translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION)),
        mLandscape(gameContext, gameContext.mResourceLoader.loadGltfModel("cube.glb", UNTEXTURED_ATTRIB_TREATMENT),
                   pipeline::gfx::PipelineName::Lambert,
                   glm::scale(translate(glm::mat4(1.0f), LANDSCAPE_POSITION), LANDSCAPE_SCALE)),
        mTextureObject(gameContext, gameContext.mResourceLoader.loadGltfModel("viking-room.glb"),
                       pipeline::gfx::PipelineName::Textured,
                       glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION), TEXTURE_OBJECT_SCALE)),
        mDebugAxes(gameContext), mDebugNormals(gameContext, mPropObjects, {}),
        mPointLights(initPointLights(gameContext)) {
        for (auto const *objPtr: {
                     &mSkyBox.mRenderObject,
                     &mDebugAxes.mRenderObject,
                     &mPropObjects,
                     &mLandscape,
                     &mTextureObject,
                     &mDebugNormals.mRenderObject,
             }) {
            mRenderObjectsByPipeline.at(pipeline::gfx::getIndex(objPtr->getPipelineName())).push_back(objPtr);
        }
        for (auto const &light: mPointLights) {
            mRenderObjectsByPipeline.at(pipeline::gfx::getIndex(light.mRenderObject.getPipelineName()))
                    .push_back(&light.mRenderObject);
        }
    }

    void SceneObjects::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mPropObjects.mTransform =
                rotate(mPropObjects.mTransform, mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                       glm::vec3(0, 0, 1));
        // Needs to come after the prop objects update
        mDebugNormals.update();
        for (auto &light: mPointLights) {
            light.update();
        }
    }

    Scene::Scene(game::GameContext &gameContext, imgui::ImGuiContext &imGuiContext) :
        mGameContext(gameContext), mImGuiContext(imGuiContext), mViewportExtent(sdl::SdlContext::INITIAL_EXTENT),
        mCamera(INITIAL_CAMERA_POSITION), mProjection(getProjection(mViewportExtent)), mSceneObjects(gameContext),
        mDepthTexture(createDepthTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)),
        mMultisampleTextureOpt(createMultisampleTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)) {}

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture = createDepthTexture(mGameContext.mSdlContext, extent);
        if (mMultisampleTextureOpt.has_value()) {
            mMultisampleTextureOpt = createMultisampleTexture(mGameContext.mSdlContext, extent);
        }
    }

    void Scene::update() { mSceneObjects.update(); }

    void Scene::draw() const {
        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(mGameContext.mSdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        SDL_GPUTexture *swapchainTexture = nullptr;
        CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, mGameContext.mSdlContext.mWindow, &swapchainTexture,
                                                    nullptr, nullptr))
                << "Failed to acquire swapchain texture: " << SDL_GetError();

        mGameContext.maybeLimitFps();

        if (swapchainTexture == nullptr) {
            SDL_Log("Swapchain texture is null");
            return;
        }

        SDL_GPUColorTargetInfo mainColorTarget{
                .texture = swapchainTexture,
                .clear_color = {0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .resolve_texture = nullptr,
        };
        if (mMultisampleTextureOpt.has_value()) {
            mainColorTarget.texture = *mMultisampleTextureOpt.value();
            mainColorTarget.store_op = SDL_GPU_STOREOP_RESOLVE;
            mainColorTarget.resolve_texture = swapchainTexture;
        }

        const SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{
                .texture = *mDepthTexture,
                .clear_depth = 1.f,
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
                .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
                .cycle = true,
        };
        SDL_GPURenderPass *renderPass =
                SDL_BeginGPURenderPass(commandBuffer, &mainColorTarget, 1, &depthStencilTargetInfo);
        drawObjects(commandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);

        // imgui -- must occur after render pass has ended
        {
            SDL_GPUColorTargetInfo swapchainTargetInfo{
                    .texture = swapchainTexture,
                    .clear_color = {0, 0, 0, 1},
                    .load_op = SDL_GPU_LOADOP_LOAD,
                    .store_op = SDL_GPU_STOREOP_STORE,
            };
            mImGuiContext.renderFrame(commandBuffer, swapchainTargetInfo);
        }
    }

    void Scene::drawObjects(SDL_GPUCommandBuffer *commandBuffer, SDL_GPURenderPass *renderPass) const {
        std::vector<glm::vec3> lightPosWs{mSceneObjects.mPointLights.size()};
        for (size_t i = 0; i < mSceneObjects.mPointLights.size(); i++) {
            lightPosWs[i] = mSceneObjects.mPointLights.at(i).mPosWs;
        }
        for (auto const &pipelineDef: pipeline::gfx::ALL_PIPELINES) {
            auto const renderObjects = mSceneObjects.getRenderObjects(pipelineDef.mName);

            if (renderObjects.empty()) {
                continue;
            }

            auto const &pipeline = mGameContext.mPipelines.get(pipelineDef.mName);
            SDL_BindGPUGraphicsPipeline(renderPass, *pipeline.mSdlPipeline);
            pipeline.bindStorageBuffers(renderPass);

            for (auto const *renderObject: renderObjects) {
                renderObject->pushPerObjectUniforms(pipelineDef, commandBuffer, mProjection, lightPosWs, mCamera);
                renderObject->render(renderPass);
            }
        }
    }
} // namespace gfx_testing::render
