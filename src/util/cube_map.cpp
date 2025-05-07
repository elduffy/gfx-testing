#include <util/cube_map.hpp>

namespace gfx_testing::util {
    CubeMap::CubeMap(std::vector<sdl::SdlSurface> surfaces) : mSurfaces(std::move(surfaces)) {}
} // namespace gfx_testing::util
