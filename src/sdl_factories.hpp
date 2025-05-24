#pragma once

#include <sdl.hpp>

namespace gfx_testing::sdl {

    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, util::Extent2D extent, SDL_GPUTextureType type,
                                   uint32_t layerCount);
}
