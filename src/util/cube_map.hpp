#pragma once

#include <sdl.hpp>
#include <vector>


namespace gfx_testing::util {
    class CubeMap {
    public:
        NO_COPY(CubeMap);
        explicit CubeMap(std::vector<sdl::SdlSurface> surfaces);
        CubeMap(CubeMap &&) = default;

        // sdl::SdlGpuTexture createTexture(sdl::SdlContext const &context) const;

        std::vector<sdl::SdlSurface> mSurfaces;
        Extent2D const mExtent;
    };
} // namespace gfx_testing::util
