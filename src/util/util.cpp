#include <util/util.hpp>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot() {
        std::filesystem::path path = std::filesystem::current_path();
        while (!is_empty(path)) {
            if (path.filename() == "gfx-testing") {
                return path;
            }
            path = path.parent_path();
        }
        FAIL("Could not find project root directory starting at {}", std::filesystem::current_path().c_str());
    }


    glm::vec3 getSphericalCoords(glm::vec3 const &cartesian) {
        // https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
        auto const r = glm::length(cartesian);
        auto const t = std::acos(cartesian.z / r);
        auto const p = glm::sign(cartesian.y) *
                       std::acos(cartesian.x / std::sqrt(cartesian.x * cartesian.x + cartesian.y * cartesian.y));
        return {r, t, p};
    }

    glm::vec3 getCartesianCoords(glm::vec3 const &spherical) {
        // https://en.wikipedia.org/wiki/Spherical_coordinate_system#Cartesian_coordinates
        auto const r = spherical.x;
        auto const t = spherical.y;
        auto const p = spherical.z;
        auto const x = r * sin(t) * cos(p);
        auto const y = r * sin(t) * sin(p);
        auto const z = r * cos(t);
        return {x, y, z};
    }

} // namespace gfx_testing::util
