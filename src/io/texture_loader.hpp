#pragma once
#include <sdl.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::io {
    sdl::SdlSurface loadImage(const std::string &path);

    // type is e.g. "PNG", or null to let SDL guess
    sdl::SdlSurface loadImage(void const *mem, size_t size, char const *type = nullptr);

    util::CubeMap loadCubeMap(std::filesystem::path const &dir);
} // namespace gfx_testing::io
