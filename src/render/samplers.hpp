#pragma once

#include <cstddef>
#include <cstring>
#include <unordered_map>

#include <sdl.hpp>

namespace gfx_testing::render {

    struct SamplerKeyOps {
        std::size_t operator()(const SDL_GPUSamplerCreateInfo &obj) const {
            constexpr std::hash<uint8_t> byteHash;
            auto combine = [](std::size_t &h, std::size_t v) { h ^= v + 0x9e3779b9u + (h << 6) + (h >> 2); };
            std::size_t hash = 0;
            combine(hash, byteHash(static_cast<uint8_t>(obj.min_filter)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.mag_filter)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.mipmap_mode)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_u)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_v)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_w)));
            combine(hash, byteHash(static_cast<uint8_t>(obj.max_anisotropy)));
            combine(hash, byteHash(obj.enable_anisotropy));
            return hash;
        }

        bool operator()(const SDL_GPUSamplerCreateInfo &lhs, const SDL_GPUSamplerCreateInfo &rhs) const {
            return std::memcmp(&lhs, &rhs, sizeof(SDL_GPUSamplerCreateInfo)) == 0;
        }
    };

    class Samplers {
    public:
        static constexpr SDL_GPUSamplerCreateInfo ANISOTROPIC_WRAP_CREATE_INFO{
                .min_filter = SDL_GPU_FILTER_LINEAR,
                .mag_filter = SDL_GPU_FILTER_LINEAR,
                .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
                .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
                .max_anisotropy = 4,
                .enable_anisotropy = true,
        };

        explicit Samplers(sdl::SdlContext const &context);

        sdl::SdlGpuSampler const &getOrCreateSampler(SDL_GPUSamplerCreateInfo const &info);


    private:
        sdl::SdlContext const &mContext;
        std::unordered_map<SDL_GPUSamplerCreateInfo, sdl::SdlGpuSampler, SamplerKeyOps, SamplerKeyOps> mCache;

    public:
        sdl::SdlGpuSampler const &mAnisotropicWrap;
    };
} // namespace gfx_testing::render
