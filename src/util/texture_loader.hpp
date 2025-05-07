#pragma once
#include <sdl.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::util {
    sdl::SdlSurface loadImage(const std::string &path);

    CubeMap loadCubeMap(std::filesystem::path const &dir);
} // namespace gfx_testing::util
