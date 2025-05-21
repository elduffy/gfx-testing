
#include <render/samplers.hpp>

namespace gfx_testing::render {
    static constexpr SDL_GPUSamplerCreateInfo ANISOTROPIC_WRAP{
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .max_anisotropy = 4,
            .enable_anisotropy = true,
    };

    Samplers::Samplers(sdl::SdlContext const &context) :
        mContext(context), mAnisotropicWrap{getOrCreateSampler(ANISOTROPIC_WRAP)} {}

    sdl::SdlGpuSampler const &Samplers::getOrCreateSampler(SDL_GPUSamplerCreateInfo const &info) {
        auto const &iter = mCache.find(info);
        if (iter == mCache.end()) {
            auto *sampler = SDL_CreateGPUSampler(mContext.mDevice, &info);
            auto const &inserted = mCache.emplace(info, sdl::SdlGpuSampler{mContext, sampler});
            return inserted.first->second;
        }
        return iter->second;
    }
} // namespace gfx_testing::render
