#include <render/draw_context.hpp>

namespace gfx_testing::render {
    sdl::ScopedCommandBuffer acquireCommandBuffer(sdl::SdlContext const &sdlContext) {
        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(sdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        return sdl::ScopedCommandBuffer(commandBuffer);
    }

    SDL_GPUTexture *acquireSwapchainTexture(SDL_GPUCommandBuffer *commandBuffer, SDL_Window *window) {
        SDL_GPUTexture *swapchainTexture = nullptr;
        CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, nullptr, nullptr))
                << "Failed to acquire swapchain texture: " << SDL_GetError();
        return swapchainTexture;
    }

    DrawContext::DrawContext(sdl::SdlContext const &sdlContext) :
        mCommandBuffer(acquireCommandBuffer(sdlContext)),
        mSwapchainTexture(acquireSwapchainTexture(*mCommandBuffer, sdlContext.mWindow)) {}
} // namespace gfx_testing::render
