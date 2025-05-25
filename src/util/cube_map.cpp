#include <absl/log/check.h>
#include <boost/safe_numerics/checked_default.hpp>
#include <util/cube_map.hpp>

namespace gfx_testing::util {
    CubeMap::CubeMap(std::vector<sdl::SdlSurface> surfaces) :
        mSurfaces(std::move(surfaces)), mExtent(mSurfaces.front().getExtent()) {
        CHECK_EQ(mSurfaces.size(), 6) << "CubeMap needs 6 surfaces, provided " << mSurfaces.size();
        for (auto const &surface: mSurfaces) {
            CHECK(surface.getExtent() == mExtent)
                    << "Cube map extent mismatch: " << surface.getExtent() << " != " << mExtent;
        }
    }

} // namespace gfx_testing::util
