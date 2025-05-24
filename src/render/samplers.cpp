
#include <render/samplers.hpp>

namespace gfx_testing::render {

    Samplers::Samplers(sdl::SdlContext const &context) :
        mContext(context), mAnisotropicWrap{getOrCreateSampler(ANISOTROPIC_WRAP_CREATE_INFO)} {}

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
