#include <absl/log/check.h>
#include <pipeline/pipelines.hpp>
#include <sdl_factories.hpp>

namespace gfx_testing::sdl {
    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, SDL_GPUTextureCreateInfo const &createInfo) {
        SdlGpuTexture tex = {sdlContext, SDL_CreateGPUTexture(sdlContext.mDevice, &createInfo)};
        CHECK_NE(*tex, nullptr) << "Failed to create GPU texture: " << SDL_GetError();
        return tex;
    }

    SdlGpuTexture createGpuTexture(SdlContext const &sdlContext, util::Extent2D extent, SDL_GPUTextureType type,
                                   uint32_t layerCount) {
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = type,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = util::narrow_u32(extent.mWidth),
                .height = util::narrow_u32(extent.mHeight),
                .layer_count_or_depth = layerCount,
                .num_levels = 1,
        };
        return createGpuTexture(sdlContext, createInfo);
    }

    SdlGpuTexture createDepthTexture(SdlContext const &sdlContext, util::Extent2D const &extent) {
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = SDL_GPU_TEXTUREFORMAT_D16_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
                .width = extent.mWidth,
                .height = extent.mHeight,
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = pipeline::MSAA_SAMPLE_COUNT,
        };
        return createGpuTexture(sdlContext, createInfo);
    }

    std::optional<SdlGpuTexture> createMultisampleTexture(SdlContext const &sdlContext, util::Extent2D const &extent) {
        if constexpr (pipeline::MSAA_SAMPLE_COUNT == SDL_GPU_SAMPLECOUNT_1) {
            return std::nullopt;
        }
        auto const format = sdlContext.mColorTargetFormat;
        const SDL_GPUTextureCreateInfo createInfo = {
                .type = SDL_GPU_TEXTURETYPE_2D,
                .format = format,
                .usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET,
                .width = extent.mWidth,
                .height = extent.mHeight,
                .layer_count_or_depth = 1,
                .num_levels = 1,
                .sample_count = pipeline::MSAA_SAMPLE_COUNT,
        };
        return createGpuTexture(sdlContext, createInfo);
    }
} // namespace gfx_testing::sdl
