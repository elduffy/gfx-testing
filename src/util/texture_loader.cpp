#include <absl/log/check.h>
#include <format>
#include <map>
#include <util/cube_map.hpp>
#include <util/texture_loader.hpp>

#include "SDL3_image/SDL_image.h"
#include "boost/algorithm/string/split.hpp"

namespace gfx_testing::util {
    sdl::SdlSurface loadImage(const std::string &path) {
        auto *surface = IMG_Load(path.c_str());
        CHECK(surface) << "Failed to load image: " << path;
        if (constexpr auto DESIRED_FORMAT = SDL_PIXELFORMAT_ABGR8888; surface->format != DESIRED_FORMAT) {
            auto *converted = SDL_ConvertSurface(surface, DESIRED_FORMAT);
            SDL_DestroySurface(surface);
            surface = converted;
        }
        return sdl::SdlSurface{surface};
    }

    CubeMap loadCubeMap(std::filesystem::path const &dir) {
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
                                       << toString(static_cast<SDL_GPUCubeMapFace>(face));
        }
        return CubeMap{std::move(result)};
    }
} // namespace gfx_testing::util
