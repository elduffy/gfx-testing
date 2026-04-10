#pragma once

#include <sdl.hpp>

namespace gfx_testing::sdl {
    constexpr auto DEFAULT_TEXTURE_FORMAT = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, util::Extent2D extent, SDL_GPUTextureType type,
                                   SDL_GPUTextureFormat format, SDL_GPUTextureUsageFlags usage, uint32_t layerCount);

    SdlGpuTexture createDepthTexture(SdlContext const &sdlContext, util::Extent2D const &extent);

    std::optional<SdlGpuTexture> createMultisampleTexture(SdlContext const &sdlContext, util::Extent2D const &extent);
} // namespace gfx_testing::sdl
