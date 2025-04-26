#include <texture.hpp>

namespace gfx_testing::io {
    sdl::SdlSurface loadBmp(const std::string &path) {
        auto *surface = SDL_LoadBMP(path.c_str());
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
