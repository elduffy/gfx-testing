#include <array>
#include <scene.hpp>
#include <shader_models.hpp>
#include <obj_loader.hpp>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>
#include <boost/safe_numerics/checked_default.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace {
    gfx_testing::shader::MVPMatrix createMVPMatrix(gfx_testing::sdl::SdlContext const &context) {
        auto const proj = glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(context.mWidth) / static_cast<float>(context.mHeight),
                0.1f,
                100.0f
                );
        auto const view = lookAt(
                glm::vec3(5, 5, 5),
                glm::vec3(0, 0, 0),
                glm::vec3(0, 1, 0)
                );
        auto const model = glm::mat4(1.0f);
        return {proj * view * model};
    }
}

namespace gfx_testing::scene {
    static constexpr size_t NUM_VERTICES = 3;
    static constexpr size_t NUM_INDICES = 3;
    static constexpr size_t VERTEX_BUFFER_SIZE = sizeof(shader::PositionColorVertex) * NUM_VERTICES;
    static constexpr size_t INDEX_BUFFER_SIZE = sizeof(uint16_t) * NUM_INDICES;

    SDL_GPUGraphicsPipeline *createPipeline(sdl::SdlContext const &context,
                                            std::filesystem::path const &projectRoot) {

        const auto vertexShader = util::loadShader(context,
                                                   projectRoot /
                                                   "content/shaders/src/pos_color_transform.vert.hlsl",
                                                   0,
                                                   1, 0, 0);
        const auto fragmentShader = util::loadShader(context,
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

    SDL_GPUBuffer *createVertexBuffer(sdl::SdlContext const &context) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = boost::safe_numerics::checked::cast<uint32_t>(VERTEX_BUFFER_SIZE),
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
    }

    SDL_GPUBuffer *createIndexBuffer(sdl::SdlContext const &context) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = SDL_GPU_BUFFERUSAGE_INDEX,
                .size = boost::safe_numerics::checked::cast<uint32_t>(INDEX_BUFFER_SIZE),
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
    }

    void transferVertexIndexData(sdl::SdlContext const &context, SDL_GPUBuffer *vertexBuffer,
                                 SDL_GPUBuffer *indexBuffer) {
        const SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo = {
                .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
                .size = boost::safe_numerics::checked::cast<uint32_t>(VERTEX_BUFFER_SIZE + INDEX_BUFFER_SIZE),
        };
        const sdl::SdlTransferBuffer transferBuffer{
                context, SDL_CreateGPUTransferBuffer(context.mDevice, &transferBufferCreateInfo)};

        // Set the index data
        {
            const auto mappedBuffer = transferBuffer.map(false);
            auto *vertexData = mappedBuffer.get<shader::PositionColorVertex>();
            static_assert(NUM_VERTICES == 3, "Code expects exactly 3 vertices");
            vertexData[0].mPosition = glm::vec3(-1, -1, 0);
            vertexData[1].mPosition = glm::vec3(1, -1, 0);
            vertexData[2].mPosition = glm::vec3(0, 1, 0);
            vertexData[0].mColor = glm::vec4(1, 0, 0, 1);
            vertexData[1].mColor = glm::vec4(0, 1, 0, 1);
            vertexData[2].mColor = glm::vec4(0, 0, 1, 1);

            auto *indexData = mappedBuffer.get<uint16_t>(VERTEX_BUFFER_SIZE);
            static_assert(NUM_INDICES == 3, "Code expects exactly 3 indices");
            indexData[0] = 0;
            indexData[1] = 1;
            indexData[2] = 2;
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
                    .size = boost::safe_numerics::checked::cast<uint32_t>(VERTEX_BUFFER_SIZE),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        // Index upload
        {
            const SDL_GPUTransferBufferLocation source = {
                    .transfer_buffer = *transferBuffer,
                    .offset = VERTEX_BUFFER_SIZE,
            };
            const SDL_GPUBufferRegion destination = {
                    .buffer = indexBuffer,
                    .offset = 0,
                    .size = boost::safe_numerics::checked::cast<uint32_t>(INDEX_BUFFER_SIZE),
            };
            SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
        }
        SDL_EndGPUCopyPass(copyPass);
    }

    Scene::Scene(sdl::SdlContext const &context, std::filesystem::path const &projectRoot) :
        mProjection(glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(context.mWidth) / static_cast<float>(context.mHeight),
                0.1f,
                100.0f)),
        mView(lookAt(glm::vec3(5, 5, 5),
                     glm::vec3(0, 0, 0),
                     glm::vec3(0, 1, 0)
                )),
        mModel(glm::mat4(1.0f)),
        mPipeline(context, createPipeline(context, projectRoot)),
        mVertexBuffer(context, createVertexBuffer(context)),
        mIndexBuffer(context, createIndexBuffer(context)) {

        // TODO: use the vertex info from the obj file
        model::loadObjFile(projectRoot / "content/models/basic-shapes.obj");
        transferVertexIndexData(context, *mVertexBuffer, *mIndexBuffer);
    }

    void Scene::Draw(sdl::SdlContext const &context) {
        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire command buffer");
        }
        auto scopedSubmit = sdl::scopedSubmitCommandBuffer(commandBuffer);

        SDL_GPUTexture *swapchainTexture = nullptr;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, context.mWindow, &swapchainTexture, nullptr,
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
        SDL_DrawGPUIndexedPrimitives(renderPass, NUM_INDICES, 1, 0, 0, 0);
        SDL_EndGPURenderPass(renderPass);

        auto mvpMatrix = createMVPMatrix(context);
    }
}
