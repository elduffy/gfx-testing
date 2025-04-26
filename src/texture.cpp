#include <texture.hpp>

#include "SDL3_image/SDL_image.h"

namespace gfx_testing::io {
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
}
