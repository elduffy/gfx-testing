#include <array>
#include <cmath>
#include <scene.hpp>
#include <shader_models.hpp>
#include <obj_loader.hpp>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/safe_numerics/checked_default.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

#include "glm/gtc/random.hpp"
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
        mCameraPosWs(INITIAL_CAMERA_POSITION),
        mView(glm::identity<glm::mat4x4>()),
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
        updateViewMatrix();
    }

    void Scene::onResize(const util::Extent2D extent) {
        mViewportExtent = extent;
        mProjection = getProjection(mViewportExtent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
    }

    glm::vec3 getSphericalCoords(glm::vec3 const &cartesian) {
        // https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
        auto const r = length(cartesian);
        auto const t = std::acos(cartesian.z / r);
        auto const p = glm::sign(cartesian.y) * std::acos(
                               cartesian.x / std::sqrt(cartesian.x * cartesian.x + cartesian.y * cartesian.y));
        return {r, t, p};
    }

    glm::vec3 getCartesianCoords(glm::vec3 spherical) {
        // https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
        auto const r = spherical.x;
        auto const t = spherical.y;
        auto const p = spherical.z;
        auto const x = r * glm::sin(t) * glm::cos(p);
        auto const y = r * glm::sin(t) * glm::sin(p);
        auto const z = r * glm::cos(t);
        return {x, y, z};
    }

    void Scene::pivotCamera(glm::vec2 const &radians) {
        // Using ISO/physics convention.
        // radians.x (theta): pivot top to bottom
        // radians.y (phi): pivot left to right
        auto const deltaTheta = radians.x;
        auto const deltaPhi = radians.y;
        auto newSpherical = getSphericalCoords(mCameraPosWs) + glm::vec3{0, deltaTheta, deltaPhi};
        constexpr auto MIN_THETA = glm::radians(.1f);
        constexpr auto MAX_THETA = glm::radians(180.f) - MIN_THETA;
        newSpherical.y = glm::clamp(newSpherical.y, MIN_THETA, MAX_THETA);
        mCameraPosWs = getCartesianCoords(newSpherical);
        updateViewMatrix();
    }

    void Scene::approachCamera(float const deltaRadius) {
        constexpr auto MIN_RADIUS = 1.f;
        auto newSpherical = getSphericalCoords(mCameraPosWs) + glm::vec3(deltaRadius, 0, 0);
        newSpherical.x = std::max(newSpherical.x, MIN_RADIUS);
        mCameraPosWs = getCartesianCoords(newSpherical);
        updateViewMatrix();
    }

    glm::vec3 Scene::getLightPosition() const {
        // TODO: store the decomposed scale/rot/translation somewhere to avoid this
        auto const totalFloatSecs = static_cast<float>(mGameContext.getFrameSnapshot().mAccumulatedTime) / 1000.f;
        auto const r = length(INITIAL_LIGHT_POSITION);
        auto const theta = -0.5 * totalFloatSecs;
        return {r * cos(theta), r * sin(theta), cos(2 * theta)};
    }

    void Scene::updateViewMatrix() {
        mView = lookAt(mCameraPosWs, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
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
                    .mView = mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCameraPosWs,
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
                    .mView = mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCameraPosWs,
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
                    .mView = mView,
                    .mProjection = mProjection,
            };
            static_assert(sizeof(mvpTransform) % 16 == 0);
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));
            shader::CameraLight cameraLight{
                    .mCameraPosWs = mCameraPosWs,
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

    util::Extent2D Scene::getViewportExtent() const {
        return mViewportExtent;
    }
}
