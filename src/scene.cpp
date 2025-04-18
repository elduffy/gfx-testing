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
                        // Can use SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4?
                        .format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM,
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

    SDL_GPUBuffer *createVertexBuffer(sdl::SdlContext const &context, size_t vertexCount) {
        const SDL_GPUBufferCreateInfo createInfo = {
                .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
                .size = boost::safe_numerics::checked::cast<
                    uint32_t>(sizeof(shader::PositionColorVertex) * vertexCount),
        };
        return SDL_CreateGPUBuffer(context.mDevice, &createInfo);
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
        mBuffer(context, createVertexBuffer(context, 3)) {

        model::loadObjFile(projectRoot / "content/models/basic-shapes.obj");
    }

    void Scene::Update(sdl::SdlContext const &context) {
        SDL_GPUCommandBuffer *commandBuffer = SDL_AcquireGPUCommandBuffer(context.mDevice);
        if (commandBuffer == nullptr) {
            throw std::runtime_error("Failed to acquire command buffer");
        }
        boost::scope::scope_exit submitCommandBufferGuard([commandBuffer] {
            SDL_SubmitGPUCommandBuffer(commandBuffer);
        });

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
                .clear_color = {1.0, 0, 0, 1},
                .load_op = SDL_GPU_LOADOP_CLEAR,
                .store_op = SDL_GPU_STOREOP_STORE,
        };
        SDL_GPURenderPass *renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, nullptr);
        SDL_EndGPURenderPass(renderPass);

        auto mvpMatrix = createMVPMatrix(context);
    }
}
