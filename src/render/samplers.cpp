
#include <render/samplers.hpp>

namespace gfx_testing::render {

    constexpr SDL_GPUSamplerCreateInfo ANISOTROPIC_WRAP{
            .min_filter = SDL_GPU_FILTER_LINEAR,
            .mag_filter = SDL_GPU_FILTER_LINEAR,
            .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
            .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
            .max_anisotropy = 4,
            .enable_anisotropy = true,
    };

    Samplers::Samplers(sdl::SdlContext const &context):
        mAnisotropicWrap(context, SDL_CreateGPUSampler(context.mDevice, &ANISOTROPIC_WRAP)) {
    }
}
