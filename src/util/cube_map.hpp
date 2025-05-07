#pragma once

#include <sdl.hpp>
#include <vector>


namespace gfx_testing::util {
    class CubeMap {
    public:
        NO_COPY(CubeMap);

        explicit CubeMap(std::vector<sdl::SdlSurface> surfaces);
        std::vector<sdl::SdlSurface> mSurfaces;
    };
} // namespace gfx_testing::util
