#include <color_target.hpp>
#include <render/draw_context.hpp>

namespace gfx_testing::render {
    sdl::ScopedCommandBuffer acquireCommandBuffer(sdl::SdlContext const &sdlContext) {
        auto *commandBuffer = SDL_AcquireGPUCommandBuffer(sdlContext.mDevice);
        CHECK_NE(commandBuffer, nullptr) << "Failed to acquire command buffer: " << SDL_GetError();
        return sdl::ScopedCommandBuffer(commandBuffer);
    }

    DrawContext::DrawContext(sdl::SdlContext const &sdlContext, sdl::ColorTargetSource const &colorTargetSource) :
        mCommandBuffer(acquireCommandBuffer(sdlContext)), mExtent(0, 0),
        mColorTarget(colorTargetSource.acquireColorTarget(*mCommandBuffer, mExtent)) {}
} // namespace gfx_testing::render
