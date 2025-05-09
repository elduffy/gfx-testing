#include <format>
#include <map>
#include <util/cube_map.hpp>
#include <util/texture_loader.hpp>

#include "SDL3_image/SDL_image.h"
#include "boost/algorithm/string/split.hpp"

namespace gfx_testing::util {
    sdl::SdlSurface loadImage(const std::string &path) {
        auto *surface = IMG_Load(path.c_str());
        if (!surface) {
            throw std::runtime_error("Failed to load image: " + path);
        }
        if (constexpr auto DESIRED_FORMAT = SDL_PIXELFORMAT_ABGR8888; surface->format != DESIRED_FORMAT) {
            auto *converted = SDL_ConvertSurface(surface, DESIRED_FORMAT);
            SDL_DestroySurface(surface);
            surface = converted;
        }
        return sdl::SdlSurface{surface};
    }

    CubeMap loadCubeMap(std::filesystem::path const &dir) {
        if (!is_directory(dir)) {
            throw std::runtime_error("Path is not a directory");
        }

        // The files are defined in the blender standard right-handed world orientation (Z+ up, Y+ forward).
        // Cubemaps expect a left-handed system (Y+ up, Z+ forward), so swizzle X/Y.
        // See https://community.khronos.org/t/image-orientation-for-cubemaps-actually-a-very-old-topic/105338/4
        const std::map<char const *, SDL_GPUCubeMapFace> prefixToFaceMap{
                {"px.", SDL_GPU_CUBEMAPFACE_POSITIVEX}, {"nx.", SDL_GPU_CUBEMAPFACE_NEGATIVEX},
                {"py.", SDL_GPU_CUBEMAPFACE_POSITIVEZ}, {"ny.", SDL_GPU_CUBEMAPFACE_NEGATIVEZ},
                {"pz.", SDL_GPU_CUBEMAPFACE_POSITIVEY}, {"nz.", SDL_GPU_CUBEMAPFACE_NEGATIVEY},
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
            if (auto const &surface = result[face]; *surface == nullptr) {
                throw std::runtime_error(std::format("Cubmap {} is missing surface {}", dir.c_str(),
                                                     toString(static_cast<SDL_GPUCubeMapFace>(face))));
            }
        }
        return CubeMap{std::move(result)};
    }
} // namespace gfx_testing::util
