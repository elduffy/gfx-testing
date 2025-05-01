#include <array>
#include <cmath>
#include <scene.hpp>
#include <shader_models.hpp>
#include <obj_loader.hpp>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include <pipelines.hpp>


namespace gfx_testing::scene {

    static constexpr glm::vec3 INITIAL_CAMERA_POSITION(5, 5, 5);
    static constexpr glm::vec3 PROP_OBJECTS_POSITION(0, 0, 0);
    static constexpr glm::vec3 CUBE_POSITION(3, 3, -1);
    static constexpr glm::vec3 TEXTURE_OBJECT_POSITION(-5, -5, 0);
    static constexpr glm::vec3 TEXTURE_OBJECT_SCALE(2);
    static constexpr glm::vec3 INITIAL_LIGHT_POSITION(2, 2, 2);

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

    Scene::Scene(game::GameContext &gameContext, imgui::ImGuiContext &imGuiContext) :
        mGameContext(gameContext),
        mImGuiContext(imGuiContext),
        mViewportExtent(sdl::SdlContext::INITIAL_EXTENT),
        mCamera(INITIAL_CAMERA_POSITION),
        mProjection(getProjection(mViewportExtent)),
        mPropObjects(gameContext,
                     gameContext.mResourceLoader.loadObjModel("basic-shapes.obj", model::NormalTreatment::SPLIT),
                     translate(glm::mat4(1.0f), PROP_OBJECTS_POSITION)),
        mCube(gameContext,
              gameContext.mResourceLoader.loadObjModel("cube.obj", model::NormalTreatment::SPLIT),
              glm::translate(glm::identity<glm::mat4>(), CUBE_POSITION)),
        mTextureObject(gameContext,
                       gameContext.mResourceLoader.loadObjModel("viking-room.obj", model::NormalTreatment::SPLIT),
                       gameContext.mResourceLoader.loadTexture("viking-room.png"),
                       glm::scale(translate(glm::mat4(1.0f), TEXTURE_OBJECT_POSITION), TEXTURE_OBJECT_SCALE)),
        mDebugAxes(gameContext),
        mPointLight(gameContext, INITIAL_LIGHT_POSITION),
        mDepthTexture(gameContext.mSdlContext,
                      createDepthTexture(gameContext.mSdlContext,
                                         sdl::SdlContext::INITIAL_EXTENT)),
        mMultisampleTextureOpt(createMultisampleTexture(gameContext.mSdlContext,
                                                        sdl::SdlContext::INITIAL_EXTENT)) {
    }

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
        if (mMultisampleTextureOpt.has_value()) {
            mMultisampleTextureOpt->reset(createDepthTexture(mGameContext.mSdlContext, extent));
        }
    }

    void Scene::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mPropObjects.mTransform = rotate(mPropObjects.mTransform,
                                         mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                                         glm::vec3(0, 0, 1));
        mCube.mTransform = rotate(mCube.mTransform,
                                  -mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND * 2,
                                  glm::vec3(0, 0, 1));
        mPointLight.update();
    }

    void Scene::draw() const {
        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(mGameContext.mSdlContext.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        SDL_GPUTexture *swapchainTexture = nullptr;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, mGameContext.mSdlContext.mWindow, &swapchainTexture,
                                                   nullptr,
                                                   nullptr)) {
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
        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &mainColorTarget, 1,
                                                               &depthStencilTargetInfo);
        auto const vp = mProjection * mCamera.mView;
        shader::MvpTransform mvpTransform{};

        // Debug axes
        {
            mvpTransform.mMvp = vp * mDebugAxes.mRenderObject.mTransform;
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.get(pipeline::PipelineName::Lines));
            // TODO: how to bind separate pipeline for the geometry?
            mDebugAxes.mRenderObject.render(renderPass);
        }
        // Point light
        {
            mvpTransform.mMvp = vp * mPointLight.mRenderObject.mTransform;
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.get(pipeline::PipelineName::Diffuse));
            mPointLight.mRenderObject.render(renderPass);
        }
        // Gooch shaded objects
        SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.get(pipeline::PipelineName::Gooch));
        constexpr shader::GoochParams goochParams{
                .mCoolColor = {0, 0, 0.55},
                .mWarmColor = {0.3, 0.3, 0},
        };
        // TODO: this data is constant and could be uploaded once into a buffer
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &goochParams, sizeof(goochParams));
        // Prop objects
        {
            auto const objectLighting = shader::ObjectLighting::create(mPropObjects.mTransform, mPointLight.mPosWs,
                                                                       mCamera.mPosWs);
            SDL_PushGPUFragmentUniformData(commandBuffer, 1, &objectLighting, sizeof(goochParams));
            mvpTransform.mMvp = vp * mPropObjects.mTransform;
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            mPropObjects.render(renderPass);
        }
        // Test cube
        {
            auto const objectLighting = shader::ObjectLighting::create(mCube.mTransform, mPointLight.mPosWs,
                                                                       mCamera.mPosWs);
            SDL_PushGPUFragmentUniformData(commandBuffer, 1, &objectLighting, sizeof(goochParams));
            mvpTransform.mMvp = vp * mCube.mTransform;
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            SDL_PushGPUFragmentUniformData(commandBuffer, 0, &goochParams, sizeof(goochParams));
            mCube.render(renderPass);
        }
        // Textured object
        {
            mvpTransform.mMvp = vp * mTextureObject.mTransform;
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.get(pipeline::PipelineName::Textured));
            mTextureObject.render(renderPass);
        }
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
}
