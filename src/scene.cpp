#include <array>
#include <scene.hpp>
#include <shader_models.hpp>
#include <obj_loader.hpp>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/safe_numerics/checked_default.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>


namespace gfx_testing::scene {
    SDL_GPUGraphicsPipeline *createPipeline(sdl::SdlContext const &context,
                                            std::filesystem::path const &projectRoot) {
        const auto vertexShader = sdl::SdlShader::loadShader(context,
                                                             projectRoot /
                                                             "content/shaders/src/pos_color_transform.vert.hlsl",
                                                             0,
                                                             1, 0, 0);
        const auto fragmentShader = sdl::SdlShader::loadShader(context,
                                                               projectRoot /
                                                               "content/shaders/src/solid_color.frag.hlsl",
                                                               0,
                                                               0, 0, 0);

        SDL_GPUColorTargetDescription colorTargetDescription = {
                .format = SDL_GetGPUSwapchainTextureFormat(context.mDevice, context.mWindow),
        };
        constexpr SDL_GPUVertexBufferDescription vertexBufferDescription = {
                .slot = 0,
                .pitch = sizeof(shader::PositionColorVertex),
                .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instance_step_rate = 0,
        };
        constexpr std::array vertexAttributes{
                SDL_GPUVertexAttribute{
                        .location = 0,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                        .offset = 0,
                },
                SDL_GPUVertexAttribute{
                        .location = 1,
                        .buffer_slot = 0,
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                        .offset = sizeof(shader::PositionColorVertex::mPosition),
                }
        };

        const SDL_GPUGraphicsPipelineCreateInfo graphicsPipelineInfo = {
                .vertex_shader = *vertexShader,
                .fragment_shader = *fragmentShader,
                .vertex_input_state = SDL_GPUVertexInputState{
                        .vertex_buffer_descriptions = &vertexBufferDescription,
                        .num_vertex_buffers = 1,
                        .vertex_attributes = &vertexAttributes[0],
                        .num_vertex_attributes = vertexAttributes.size(),
                },
                .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
                .target_info = {
                        .color_target_descriptions = &colorTargetDescription,
                        .num_color_targets = 1,
                },
        };
        auto *pipeline = SDL_CreateGPUGraphicsPipeline(context.mDevice, &graphicsPipelineInfo);
        if (pipeline == nullptr) {
            throw std::runtime_error("Failed to create graphics pipeline");
        }
        SDL_Log("Created graphics pipeline");
        return pipeline;
    }

    SDL_GPUBuffer *createVertexBuffer(sdl::SdlContext const &context, shader::MeshData const &meshData) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = meshData.getVertexBufferSize(),
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
    }

    SDL_GPUBuffer *createIndexBuffer(sdl::SdlContext const &context, shader::MeshData const &meshData) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                .size = meshData.getIndexBufferSize(),
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
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
            std::ranges::copy(meshData.vertices, vertexData);

            auto *indexData = mappedBuffer.get<uint16_t>(meshData.getVertexBufferSize());
            std::ranges::copy(meshData.indices, indexData);
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

    Scene::Scene(game::GameContext const &gameContext, std::filesystem::path const &projectRoot) :
        mGameContext(gameContext),
        mProjection(getProjection(sdl::SdlContext::INITIAL_EXTENT)),
        mView(lookAt(glm::vec3(5, 5, 5),
                     glm::vec3(0, 0, 0),
                     glm::vec3(0, 0, 1)
                )),
        mModel(glm::mat4(1.0f)),
        mMeshData(model::loadObjFile(projectRoot / "content/models/basic-shapes.obj")),
        mPipeline(gameContext.mSdlContext, createPipeline(gameContext.mSdlContext, projectRoot)),
        mVertexBuffer(gameContext.mSdlContext, createVertexBuffer(gameContext.mSdlContext, mMeshData)),
        mIndexBuffer(gameContext.mSdlContext, createIndexBuffer(gameContext.mSdlContext, mMeshData)) {

        transferVertexIndexData(gameContext.mSdlContext, mMeshData, *mVertexBuffer, *mIndexBuffer);
    }

    void Scene::onResize(const util::Extent2D extent) {
        mProjection = getProjection(extent);
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

        const auto mvpMatrix = mProjection * mView * mModel;
        SDL_PushGPUVertexUniformData(commandBuffer, 0, &mvpMatrix, sizeof(mvpMatrix));

        const SDL_GPUColorTargetInfo colorTargetInfo{
                .texture = swapchainTexture,
                .clear_color = {0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
        SDL_BindGPUGraphicsPipeline(renderPass, *mPipeline);

        SDL_GPUBufferBinding vertexBufferBinding = {
                .buffer = *mVertexBuffer,
                .offset = 0,
        };
        SDL_GPUBufferBinding indexBufferBinding = {
                .buffer = *mIndexBuffer,
                .offset = 0,
        };
        SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);
        // SDL_DrawGPUPrimitives(renderPass, NUM_VERTICES, 1, 0, 0);
        SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_DrawGPUIndexedPrimitives(renderPass, mMeshData.indices.size(), 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);
    }
}
