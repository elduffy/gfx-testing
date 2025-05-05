#pragma once

#include <filesystem>

#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#define NO_COPY(T) T(const T &) = delete
#define NO_MOVE(T) T(T &&) = delete
#define NO_COPY_NO_MOVE(T)                                                                                             \
    NO_COPY(T);                                                                                                        \
    NO_MOVE(T)


namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();

    struct Extent2D {
        uint32_t mWidth;
        uint32_t mHeight;

        [[nodiscard]] glm::vec2 asVec2() const { return {mWidth, mHeight}; }
    };

    glm::vec3 getSphericalCoords(glm::vec3 const &cartesian);

    glm::vec3 getCartesianCoords(glm::vec3 const &spherical);
} // namespace gfx_testing::util
