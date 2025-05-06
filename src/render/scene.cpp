#include <array>
#include <boost/scope/scope_exit.hpp>
#include <cmath>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <pipeline/pipelines.hpp>
#include <render/scene.hpp>
#include <util/util.hpp>


namespace gfx_testing::render {

    static constexpr glm::vec3 INITIAL_CAMERA_POSITION(5, 5, 5);

    SDL_GPUTexture *createDepthTexture(sdl::SdlContext const &context, util::Extent2D extent) {
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                .width = extent.mWidth,
                .height = extent.mHeight,
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = pipeline::MSAA_SAMPLE_COUNT,
        };
        return SDL_CreateGPUTexture(context.mDevice, &createInfo);
    }

    std::optional<sdl::SdlGpuTexture> createMultisampleTexture(sdl::SdlContext const &context, util::Extent2D extent) {
        if constexpr (pipeline::MSAA_SAMPLE_COUNT == SDL_GPU_SAMPLECOUNT_1) {
            return std::nullopt;
        }
        auto const format = SDL_GetGPUSwapchainTextureFormat(context.mDevice, context.mWindow);
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = format,
                .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
                .width = extent.mWidth,
                .height = extent.mHeight,
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = pipeline::MSAA_SAMPLE_COUNT,
        };
        return sdl::SdlGpuTexture{context, SDL_CreateGPUTexture(context.mDevice, &createInfo)};
    }

    glm::mat4x4 getProjection(const util::Extent2D extent) {
        auto const aspect = static_cast<float>(extent.mWidth) / static_cast<float>(extent.mHeight);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

    SceneObjects::SceneObjects(game::GameContext &gameContext) :
        mGameContext(gameContext),
        mPropObjects(gameContext,
                     gameContext.mResourceLoader.loadObjModel("basic-shapes.obj", util::NormalTreatment::SPLIT),
                     pipeline::PipelineName::Gooch, translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION)),
        mLandscape(gameContext, gameContext.mResourceLoader.loadObjModel("cube.obj", util::NormalTreatment::SPLIT),
                   pipeline::PipelineName::Lambert,
                   glm::scale(translate(glm::mat4(1.0f), LANDSCAPE_POSITION), LANDSCAPE_SCALE)),
        mTextureObject(gameContext,
                       gameContext.mResourceLoader.loadObjModel("viking-room.obj", util::NormalTreatment::SPLIT),
                       gameContext.mResourceLoader.loadTexture("viking-room.png"),
                       glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION), TEXTURE_OBJECT_SCALE)),
        mDebugAxes(gameContext), mPointLight(gameContext, INITIAL_LIGHT_POSITION) {
        for (auto const *objPtr:
             {&mDebugAxes.mRenderObject, &mPointLight.mRenderObject, &mPropObjects, &mLandscape, &mTextureObject}) {
            mRenderObjectsByPipeline.at(pipeline::getIndex(objPtr->getPipelineName())).push_back(objPtr);
        }
    }

    void SceneObjects::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mPropObjects.mTransform =
                rotate(mPropObjects.mTransform, mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                       glm::vec3(0, 0, 1));
        mPointLight.update();
    }


    Scene::Scene(game::GameContext &gameContext, imgui::ImGuiContext &imGuiContext) :
        mGameContext(gameContext), mImGuiContext(imGuiContext), mViewportExtent(sdl::SdlContext::INITIAL_EXTENT),
        mCamera(INITIAL_CAMERA_POSITION), mProjection(getProjection(mViewportExtent)), mSceneObjects(gameContext),
        mDepthTexture(gameContext.mSdlContext,
                      createDepthTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)),
        mMultisampleTextureOpt(createMultisampleTexture(gameContext.mSdlContext, sdl::SdlContext::INITIAL_EXTENT)) {}

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
        if (mMultisampleTextureOpt.has_value()) {
            mMultisampleTextureOpt->reset(createDepthTexture(mGameContext.mSdlContext, extent));
        }
    }

    void Scene::update() { mSceneObjects.update(); }

    void Scene::draw() const {
        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(mGameContext.mSdlContext.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        SDL_GPUTexture *swapchainTexture = nullptr;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, mGameContext.mSdlContext.mWindow, &swapchainTexture,
                                                   nullptr, nullptr)) {
            throw std::runtime_error("Failed to acquire swapchain texture");
        }

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
        auto const viewProj = mProjection * mCamera.mView;

        for (auto const &pipelineDef: pipeline::ALL_PIPELINES) {
            auto const renderObjects = mSceneObjects.getRenderObjects(pipelineDef.mName);

            if (renderObjects.empty()) {
                continue;
            }

            auto const &pipeline = mGameContext.mPipelines.get(pipelineDef.mName);
            SDL_BindGPUGraphicsPipeline(renderPass, *pipeline.mSdlPipeline);
            pipeline.bindStorageBuffers(renderPass);

            for (auto const *renderObject: renderObjects) {
                renderObject->pushPerObjectUniforms(pipelineDef, commandBuffer, viewProj,
                                                    mSceneObjects.mPointLight.mPosWs, mCamera.mPosWs);
                renderObject->render(renderPass);
            }
        }
    }
} // namespace gfx_testing::render
