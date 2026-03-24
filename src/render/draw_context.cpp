#include <render/draw_context.hpp>

namespace gfx_testing::render {
    sdl::ScopedCommandBuffer acquireCommandBuffer(sdl::SdlContext const &sdlContext) {
        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(sdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        return sdl::ScopedCommandBuffer(commandBuffer);
    }

    SDL_GPUTexture *acquireSwapchainTexture(SDL_GPUCommandBuffer *commandBuffer, SDL_Window *window,
                                            util::Extent2D &extentOut) {
        SDL_GPUTexture *swapchainTexture = nullptr;
        CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &extentOut.mWidth,
                                                    &extentOut.mHeight))
                << "Failed to acquire swapchain texture: " << SDL_GetError();
        return swapchainTexture;
    }

    sdl::SdlGpuTexture createOffscreenColorTarget(sdl::SdlContext const &sdlContext, util::Extent2D extent) {
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = sdlContext.mColorTargetFormat,
                .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = extent.mWidth,
                .height = extent.mHeight,
                .layer_count_or_depth = 1,
                .num_levels = 1,
        };
        auto *texture = SDL_CreateGPUTexture(sdlContext.mDevice, &createInfo);
        CHECK_NE(texture, nullptr) << "Failed to create offscreen color target: " << SDL_GetError();
        return sdl::SdlGpuTexture{sdlContext, texture};
    }

    DrawContext::DrawContext(sdl::SdlContext const &sdlContext) :
        mCommandBuffer(acquireCommandBuffer(sdlContext)), mExtent(0, 0),
        mColorTarget(acquireSwapchainTexture(*mCommandBuffer, sdlContext.mWindow, mExtent)) {}

    DrawContext::DrawContext(sdl::SdlContext const &sdlContext, util::Extent2D extent) :
        mCommandBuffer(acquireCommandBuffer(sdlContext)), mExtent(extent), mColorTarget(nullptr),
        mOffscreenTextureOpt(createOffscreenColorTarget(sdlContext, extent)) {
        mColorTarget = *mOffscreenTextureOpt.value();
    }
} // namespace gfx_testing::render
