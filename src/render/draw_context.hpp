#pragma once

#include <sdl.hpp>

namespace gfx_testing::render {
    class DrawContext {
    public:
        explicit DrawContext(sdl::SdlContext const &sdlContext);
        DrawContext(sdl::SdlContext const &sdlContext, util::Extent2D extent);

        bool hasColorTarget() const { return mColorTarget != nullptr; }

        sdl::ScopedCommandBuffer mCommandBuffer;
        util::Extent2D mExtent;
        SDL_GPUTexture *mColorTarget;
        std::optional<sdl::SdlGpuTexture> mOffscreenTextureOpt;
        std::optional<sdl::SdlGpuTexture> mMultisampleTextureOpt;
    };
} // namespace gfx_testing::render
