#include <scene.hpp>
#include <shader_models.hpp>
#include <obj_loader.hpp>
#include <util.hpp>
#include <boost/scope/scope_exit.hpp>

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

namespace{
   gfx_testing::shader::MVPMatrix createMVPMatrix(gfx_testing::sdl::SdlContext const &context) {
      auto const proj = glm::perspective(
          glm::radians(45.0f),
          static_cast<float>(context.mWidth) / static_cast<float>(context.mHeight),
          0.1f,
          100.0f
      );
      auto const view = glm::lookAt(
          glm::vec3(5, 5, 5),
          glm::vec3(0, 0, 0),
          glm::vec3(0, 1, 0)
      );
      auto const model = glm::mat4(1.0f);
      return {proj * view * model};
   }
}

namespace gfx_testing::scene {

   Scene::Scene(sdl::SdlContext const &context, std::filesystem::path const &projectRoot)
    :mProjection(glm::perspective(
          glm::radians(45.0f),
          static_cast<float>(context.mWidth) / static_cast<float>(context.mHeight),
          0.1f,
          100.0f)),
      mView(glm::lookAt(glm::vec3(5, 5, 5),
                        glm::vec3(0, 0, 0),
                        glm::vec3(0, 1, 0)
                        )),
      mModel(glm::mat4(1.0f)) {

      gfx_testing::model::loadObjFile(projectRoot / "content/models/basic-shapes.obj");

      auto vertexShader = gfx_testing::util::loadShader(context,
                                                        projectRoot / "content/shaders/src/pos_color_transform.vert.hlsl",
                                                        0,
                                                        1, 0, 0);
      auto fragmentShader = gfx_testing::util::loadShader(context,
                                                          projectRoot /
                                                          "content/shaders/src/solid_color.frag.hlsl",
                                                          0,
                                                          0, 0, 0);
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
       if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, context.mWindow, &swapchainTexture, nullptr, nullptr)) {
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