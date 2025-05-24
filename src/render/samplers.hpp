#pragma once

#include <boost/container_hash/hash.hpp>
#include <cstring>
#include <sdl.hpp>
#include <unordered_map>

namespace gfx_testing::render {

    struct SamplerKeyOps {
        std::size_t operator()(const SDL_GPUSamplerCreateInfo &obj) const {
            std::size_t hash = 0;
            constexpr std::hash<uint8_t> byteHash;
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.min_filter)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.mag_filter)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.mipmap_mode)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_u)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_v)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.address_mode_w)));
            boost::hash_combine(hash, byteHash(static_cast<uint8_t>(obj.max_anisotropy)));
            boost::hash_combine(hash, byteHash(obj.enable_anisotropy));
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
