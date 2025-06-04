#pragma once

#include <sdl.hpp>

namespace gfx_testing::render {
    class DrawContext {
    public:
        explicit DrawContext(sdl::SdlContext const &sdlContext);

        bool hasSwapchain() const { return mSwapchainTexture != nullptr; }

        sdl::ScopedCommandBuffer mCommandBuffer;
        SDL_GPUTexture *mSwapchainTexture;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
} // namespace gfx_testing::render
