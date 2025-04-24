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
    static constexpr glm::vec3 COOL_COLOR(0, 0, 0.55);
    static constexpr glm::vec3 WARM_COLOR(0.3, 0.3, 0);

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

    Scene::Scene(game::GameContext &gameContext, std::filesystem::path const &projectRoot) :
        mGameContext(gameContext),
        mViewportExtent(sdl::SdlContext::INITIAL_EXTENT),
        mCamera(INITIAL_CAMERA_POSITION),
        mProjection(getProjection(mViewportExtent)),
        mRenderObject(gameContext, model::loadObjFile(projectRoot / "content/models/basic-shapes.obj"),
                      translate(glm::mat4(1.0f), OBJECT_POSITION)),
        mDebugAxes(gameContext, model::loadObjFile(projectRoot / "content/models/debug-axes.obj"),
                   glm::mat4(1.0f)),
        mPointLight(gameContext, model::loadObjFile(projectRoot / "content/models/uv-sphere.obj"),
                    translate(glm::mat4(1.0f), INITIAL_LIGHT_POSITION)),
        mDepthTexture(gameContext.mSdlContext,
                      createDepthTexture(gameContext.mSdlContext,
                                         sdl::SdlContext::INITIAL_EXTENT)) {
    }

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
    }

    glm::vec3 Scene::getLightPosition() const {
        // TODO: store the decomposed scale/rot/translation somewhere to avoid this
        auto const totalFloatSecs = static_cast<float>(mGameContext.getFrameSnapshot().mAccumulatedTime) / 1000.f;
        auto const r = length(INITIAL_LIGHT_POSITION);
        auto const theta = -0.5 * totalFloatSecs;
        return {r * cos(theta), r * sin(theta), cos(2 * theta)};
    }

    void Scene::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 8.f;

        mRenderObject.mTransform = rotate(mRenderObject.mTransform,
                                          mGameContext.getFrameSnapshot().mDeltaTime * RADS_PER_SECOND,
                                          glm::vec3(0, 0, 1));
        mPointLight.mTransform = translate(glm::mat4(1.0f), getLightPosition());
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

        auto const lightPos = getLightPosition();
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
                    .mLightPosWs = lightPos,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mDiffuse);

            mDebugAxes.render(renderPass);
        }

        // Point light
        {
            shader::MvpTransform mvpTransform{
                    .mModel = mPointLight.mTransform,
                    .mView = mCamera.mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCamera.mPosWs,
                    .mLightPosWs = lightPos,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mDiffuse);

            mPointLight.render(renderPass);
        }

        // Render object
        {
            shader::MvpTransform mvpTransform{
                    .mModel = mRenderObject.mTransform,
                    .mView = mCamera.mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCamera.mPosWs,
                    .mLightPosWs = lightPos,
            };
            static_assert(sizeof(cameraLight) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 1, &cameraLight, sizeof(cameraLight));

            const shader::GoochParams params{
                    .mCoolColor = COOL_COLOR,
                    .mWarmColor = WARM_COLOR,
            };
            static_assert(sizeof(params) % 16 == 0);
            SDL_PushGPUFragmentUniformData(commandBuffer, 0, &params, sizeof(params));
            SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mGooch);

            mRenderObject.render(renderPass);
        }
        SDL_EndGPURenderPass(renderPass);
    }
}
