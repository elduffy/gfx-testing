#pragma once

#include <sdl.hpp>

namespace gfx_testing::sdl {
    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, util::Extent2D extent, SDL_GPUTextureType type,
                                   uint32_t layerCount);

    SdlGpuTexture createDepthTexture(SdlContext const &sdlContext, util::Extent2D const &extent);

    std::optional<SdlGpuTexture> createMultisampleTexture(SdlContext const &sdlContext, util::Extent2D const &extent);
} // namespace gfx_testing::sdl
