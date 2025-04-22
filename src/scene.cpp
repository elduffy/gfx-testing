#include <array>
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
#include <buffer.hpp>


namespace gfx_testing::scene {

    static constexpr glm::vec3 CAMERA_POSITION(5, 5, 5);
    static constexpr glm::vec3 OBJECT_POSITION(0, 0, 0);
    static constexpr glm::vec3 LIGHT_POSITION(1, -1, 1);
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

    void transferVertexIndexData(sdl::SdlContext const &context, shader::MeshData const &meshData,
                                 SDL_GPUBuffer *vertexBuffer,
                                 SDL_GPUBuffer *indexBuffer) {
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = boost::safe_numerics::checked::add(meshData.getVertexBufferSize(),
                                                           meshData.getIndexBufferSize()),
        };
        const sdl::SdlTransferBuffer transferBuffer{
                context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};

        // Set the vertex/index data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            auto *vertexData = mappedBuffer.get<shader::PositionColorVertex>();
            std::ranges::copy(meshData.mVertices, vertexData);

            auto *indexData = mappedBuffer.get<uint16_t>(meshData.getVertexBufferSize());
            std::ranges::copy(meshData.mIndices, indexData);
        }

        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire GPU command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        auto *copyPass = SDL_BeginGPUCopyPass(commandBuffer);
        // Vertex upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = 0,
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = vertexBuffer,
                    .offset = 0,
                    .size = meshData.getVertexBufferSize(),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        // Index upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = meshData.getVertexBufferSize(),
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = indexBuffer,
                    .offset = 0,
                    .size = meshData.getIndexBufferSize(),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        SDL_EndGPUCopyPass(copyPass);
    }

    glm::mat4x4 getProjection(const util::Extent2D extent) {
        auto const aspect = static_cast<float>(extent.mWidth) / static_cast<float>(extent.mHeight);
        return glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    }

    Scene::Scene(game::GameContext &gameContext, std::filesystem::path const &projectRoot) :
        mGameContext(gameContext),
        mProjection(getProjection(sdl::SdlContext::INITIAL_EXTENT)),
        mView(lookAt(CAMERA_POSITION,
                     glm::vec3(0, 0, 0),
                     glm::vec3(0, 0, 1)
                )),
        mModel(translate(glm::mat4(1.0f), OBJECT_POSITION)),
        mMeshData(model::loadObjFile(projectRoot / "content/models/basic-shapes.obj")),
        mVertexBuffer(mGameContext.mBufferManager.allocate(
                SDL_GPU_BUFFERUSAGE_VERTEX, mMeshData.getVertexBufferSize())),
        mIndexBuffer(mGameContext.mBufferManager.allocate(
                SDL_GPU_BUFFERUSAGE_INDEX, mMeshData.getIndexBufferSize())),
        mDepthTexture(gameContext.mSdlContext,
                      createDepthTexture(gameContext.mSdlContext,
                                         sdl::SdlContext::INITIAL_EXTENT)) {
        transferVertexIndexData(gameContext.mSdlContext, mMeshData, mVertexBuffer, mIndexBuffer);
    }

    void Scene::onResize(const util::Extent2D extent) {
        mProjection = getProjection(extent);
        mDepthTexture.reset(createDepthTexture(mGameContext.mSdlContext, extent));
    }


    void Scene::update() {
        constexpr auto RADS_PER_SECOND = glm::pi<float>() / 4.f;
        mModel = rotate(mModel, mGameContext.mDeltaTime * RADS_PER_SECOND, glm::vec3(0, 0, 1));
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


        shader::MvpTransform mvpTransform{
                .mModelView = mView * mModel,
                .mProjection = mProjection,
        };
        // auto const mvpTransform = mProjection * mView * mModel;
        static_assert(sizeof(mvpTransform) % 16 == 0);
        SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpTransform, sizeof(mvpTransform));

        const shader::GoochParams params{
                .mCameraPos = glm::vec3(mProjection * mView * mModel * glm::vec4(CAMERA_POSITION, 1)),
                .mLightPos = glm::vec3(mProjection * mView * mModel * glm::vec4(LIGHT_POSITION, 1)),
                .mCoolColor = COOL_COLOR,
                .mWarmColor = WARM_COLOR,
        };
        static_assert(sizeof(params) % 16 == 0);
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &params, sizeof(params));

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
        SDL_BindGPUGraphicsPipeline(renderPass, *mGameContext.mPipelines.mGooch);

        SDL_GPUBufferBinding vertexBufferBinding = {
                .buffer = mVertexBuffer,
                .offset = 0,
        };
        SDL_GPUBufferBinding indexBufferBinding = {
                .buffer = mIndexBuffer,
                .offset = 0,
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
        // SDL_DrawGPUPrimitives(renderPass, NUM_VERTICES, 1, 0, 0);
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_DrawGPUIndexedPrimitives(renderPass, mMeshData.mIndices.size(), 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);
    }
}
