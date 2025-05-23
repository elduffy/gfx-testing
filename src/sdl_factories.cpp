#include <absl/log/check.h>
#include <sdl_factories.hpp>

namespace gfx_testing::sdl {
    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, util::Extent2D extent) {
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = boost::safe_numerics::checked::cast<uint32_t>(extent.mWidth),
                .height = boost::safe_numerics::checked::cast<uint32_t>(extent.mHeight),
                .layer_count_or_depth = 1,
                .num_levels = 1,
        };

        SdlGpuTexture tex = {sdlContext, SDL_CreateGPUTexture(sdlContext.mDevice, &createInfo)};
        CHECK_NE(*tex, nullptr) << "Failed to create GPU texture: " << SDL_GetError();
        return tex;
    }
} // namespace gfx_testing::sdl
