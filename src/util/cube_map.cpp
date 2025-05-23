#include <absl/log/check.h>
#include <boost/safe_numerics/checked_default.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::util {
    CubeMap::CubeMap(std::vector<sdl::SdlSurface> surfaces) :
        mSurfaces(std::move(surfaces)), mExtent(mSurfaces.front().getExtent()) {
        CHECK_EQ(mSurfaces.size(), 6) << "CubeMap needs 6 surfaces, provided " << mSurfaces.size();
        for (auto const &surface: mSurfaces) {
            CHECK(surface.getExtent() == mExtent)
                    << "Cube map extent mismatch: " << surface.getExtent() << " != " << mExtent;
        }
    }

    sdl::SdlGpuTexture CubeMap::createTexture(sdl::SdlContext const &context) const {
        const SDL_GPUTextureCreateInfo createInfo{
                .type = SDL_GPU_TEXTURETYPE_CUBE,
                .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                .usage = SDL_GPU_TEXTUREUSAGE_SAMPLER,
                .width = mExtent.mWidth,
                .height = mExtent.mHeight,
                .layer_count_or_depth = 6,
                .num_levels = 1,
        };
        auto *texture = SDL_CreateGPUTexture(context.mDevice, &createInfo);
        if (texture == nullptr) {
            throw std::runtime_error("Failed to create texture");
        }
        sdl::SdlGpuTexture result{context, texture};
        result.upload(mSurfaces);
        return result;
    }

} // namespace gfx_testing::util
