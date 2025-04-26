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
    static constexpr glm::vec3 OBJECT_POSITION(0, 0, 0);
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
                .sample_count = SDL_GPU_SAMPLECOUNT_1,
        };
        return SDL_CreateGPUTexture(context.mDevice, &createInfo);
    }

    glm::mat4x4 getProjection(const util::Extent2D extent) {
        auto const aspect = static_cast<float>(extent.mWidth) / static_cast<float>(extent.mHeight);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

    Scene::Scene(game::GameContext &gameContext) :
        mGameContext(gameContext),
        mViewportExtent(sdl::SdlContext::INITIAL_EXTENT),
        mCamera(INITIAL_CAMERA_POSITION),
        mProjection(getProjection(mViewportExtent)),
        mPropObjects(gameContext,
                     gameContext.mResourceLoader.loadObjModel("viking-room.obj", model::NormalTreatment::SPLIT),
                     gameContext.mResourceLoader.loadTexture("viking-room.png"),
                     glm::scale(translate(glm::mat4(1.0f), OBJECT_POSITION), glm::vec3(3))),
        mDebugAxes(gameContext,
                   gameContext.mResourceLoader.loadObjModel(
                           "debug-axes.obj", model::NormalTreatment::AVERAGE),
                   glm::mat4(1.0f)),
        mPointLight(gameContext, INITIAL_LIGHT_POSITION),
        mDepthTexture(gameContext.mSdlContext,
                      createDepthTexture(gameContext.mSdlContext,
                                         sdl::SdlContext::INITIAL_EXTENT)) {
    }

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
    }

    void Scene::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mPropObjects.mTransform = rotate(mPropObjects.mTransform,
                                         mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
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


        const SDL_GPUColorTargetInfo colorTargetInfo{
                .texture = swapchainTexture,
                .clear_color = {0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
        };
        const SDL_GPUDepthStencilTargetInfo depthStencilTargetInfo{
                .texture = *mDepthTexture,
                .clear_depth = 1.f,
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
                .stencil_load_op = SDL_GPU_LOADOP_DONT_CARE,
                .stencil_store_op = SDL_GPU_STOREOP_DONT_CARE,
                .cycle = true,
        };
        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1,
                                                               &depthStencilTargetInfo);

        // Debug axes
        {
            shader::MvpTransform mvpTransform{
                    .mModel = mDebugAxes.mTransform,
                    .mView = mCamera.mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCamera.mPosWs,
                    .mLightPosWs = mPointLight.mPosWs,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mDiffuse);

            mDebugAxes.render(renderPass);
        }

        // Point light
        {
            auto const &renderObject = mPointLight.mRenderObject;
            shader::MvpTransform mvpTransform{
                    .mModel = renderObject.mTransform,
                    .mView = mCamera.mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCamera.mPosWs,
                    .mLightPosWs = mPointLight.mPosWs,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mDiffuse);

            renderObject.render(renderPass);
        }

        // Prop objects
        {
            shader::MvpTransform mvpTransform{
                    .mModel = mPropObjects.mTransform,
                    .mView = mCamera.mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCamera.mPosWs,
                    .mLightPosWs = mPointLight.mPosWs,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));

            // const shader::GoochParams params{
            //         .mCoolColor = {0, 0, 0.55},
            //         .mWarmColor = {0.3, 0.3, 0},
            // };
            // static_assert(sizeof(params) % 16 == 0);
            // SDL_PushGPUFragmentUniformData(commandBuffer, 0, &params, sizeof(params));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mTextured);

            mPropObjects.render(renderPass);
        }
        SDL_EndGPURenderPass(renderPass);
    }
}
