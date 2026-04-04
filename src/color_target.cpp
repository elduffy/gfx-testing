#include <color_target.hpp>
#include <sdl_factories.hpp>

namespace gfx_testing::sdl {
    SdlGpuTexture createHeadlessColorTarget(const SdlContext &sdlContext) {
        if (sdlContext.isHeadless()) {
            return createGpuTexture(sdlContext, SdlContext::INITIAL_EXTENT, SDL_GPU_TEXTURETYPE_2D,
                                    sdlContext.mColorTargetFormat,
                                    SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER, 1);
        }
        return SdlGpuTexture(sdlContext, nullptr);
    }

    SDL_GPUTexture *acquireSwapchainTexture(SDL_GPUCommandBuffer *commandBuffer, SDL_Window *window,
                                            util::Extent2D &extentOut) {
        SDL_GPUTexture *swapchainTexture = nullptr;
        CHECK(SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window, &swapchainTexture, &extentOut.mWidth,
                                                    &extentOut.mHeight))
                << "Failed to acquire swapchain texture: " << SDL_GetError();
        return swapchainTexture;
    }

    ColorTargetSource::ColorTargetSource(SdlContext const &sdlContext) :
        mSdlContext(sdlContext), mHeadlessColorTarget(createHeadlessColorTarget(mSdlContext)) {}

    SDL_GPUTexture *ColorTargetSource::acquireColorTarget(SDL_GPUCommandBuffer *commandBuffer,
                                                          util::Extent2D &extentOut) const {
        if (*mHeadlessColorTarget != nullptr) {
            extentOut = SdlContext::INITIAL_EXTENT;
            return *mHeadlessColorTarget;
        }
        return acquireSwapchainTexture(commandBuffer, mSdlContext.mWindow, extentOut);
    }
} // namespace gfx_testing::sdl
