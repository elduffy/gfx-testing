#include <absl/log/check.h>
#include <format>
#include <io/texture_loader.hpp>
#include <map>
#include <util/cube_map.hpp>

#include "SDL3_image/SDL_image.h"
#include "boost/algorithm/string/split.hpp"

namespace gfx_testing::io {
    sdl::SdlSurface convertFormat(SDL_Surface *surface) {
        if (constexpr auto DESIRED_FORMAT = SDL_PIXELFORMAT_ABGR8888; surface->format != DESIRED_FORMAT) {
            auto *converted = SDL_ConvertSurface(surface, DESIRED_FORMAT);
            SDL_DestroySurface(surface);
            surface = converted;
        }
        return sdl::SdlSurface{surface};
    }

    sdl::SdlSurface loadImage(const std::string &path) {
        auto *surface = IMG_Load(path.c_str());
        CHECK_NE(surface, nullptr) << "Failed to load image at " << path << ": " << SDL_GetError();
        return convertFormat(surface);
    }

    sdl::SdlSurface loadImage(void const *mem, size_t size, char const *type) {
        auto *stream = SDL_IOFromConstMem(mem, size);
        SDL_Surface *surface = nullptr;
        if (type == nullptr) {
            surface = IMG_Load_IO(stream, true);
        } else {
            surface = IMG_LoadTyped_IO(stream, true, type);
        }
        CHECK_NE(surface, nullptr) << "Failed to load image from stream: " << SDL_GetError();
        return convertFormat(surface);
    }

    util::CubeMap loadCubeMap(std::filesystem::path const &dir) {
        CHECK(is_directory(dir)) << "Path is not a directory: " << dir;

        // The files are defined in a left-handed system (Y+ up, Z+ forward) that differs from our standard.
        // Load them as is, then swizzle X/Y in the shader.
        const std::map<char const *, SDL_GPUCubeMapFace> prefixToFaceMap{
                {"px.", SDL_GPU_CUBEMAPFACE_POSITIVEX}, {"nx.", SDL_GPU_CUBEMAPFACE_NEGATIVEX},
                {"py.", SDL_GPU_CUBEMAPFACE_POSITIVEY}, {"ny.", SDL_GPU_CUBEMAPFACE_NEGATIVEY},
                {"pz.", SDL_GPU_CUBEMAPFACE_POSITIVEZ}, {"nz.", SDL_GPU_CUBEMAPFACE_NEGATIVEZ},
        };

        std::vector<sdl::SdlSurface> result{6};
        for (auto const &file: std::filesystem::directory_iterator(dir)) {
            for (auto const &[prefix, index]: prefixToFaceMap) {
                if (file.path().filename().string().starts_with(prefix)) {
                    result[index] = loadImage(file.path().c_str());
                }
            }
        }
        for (size_t face = 0; face < 6; face++) {
            auto const &surface = result[face];
            CHECK(*surface != nullptr) << "Cubemap " << dir << " is missing surface "
                                       << util::toString(static_cast<SDL_GPUCubeMapFace>(face));
        }
        return util::CubeMap{std::move(result)};
    }
} // namespace gfx_testing::io
