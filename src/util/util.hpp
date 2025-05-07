#pragma once

#include <filesystem>

#include "../../../SDL_image/Xcode/iOS/SDL3.framework/Headers/SDL_gpu.h"
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

        friend bool operator==(const Extent2D &lhs, const Extent2D &rhs) {
            return lhs.mWidth == rhs.mWidth && lhs.mHeight == rhs.mHeight;
        }

        [[nodiscard]] glm::vec2 asVec2() const { return {mWidth, mHeight}; }
    };

    glm::vec3 getSphericalCoords(glm::vec3 const &cartesian);

    glm::vec3 getCartesianCoords(glm::vec3 const &spherical);

    inline char const *toString(SDL_GPUCubeMapFace face) {
        switch (face) {
            case SDL_GPU_CUBEMAPFACE_POSITIVEX:
                return "SDL_GPU_CUBEMAPFACE_POSITIVEX";
            case SDL_GPU_CUBEMAPFACE_NEGATIVEX:
                return "SDL_GPU_CUBEMAPFACE_NEGATIVEX";
            case SDL_GPU_CUBEMAPFACE_POSITIVEY:
                return "SDL_GPU_CUBEMAPFACE_POSITIVEY";
            case SDL_GPU_CUBEMAPFACE_NEGATIVEY:
                return "SDL_GPU_CUBEMAPFACE_NEGATIVEY";
            case SDL_GPU_CUBEMAPFACE_POSITIVEZ:
                return "SDL_GPU_CUBEMAPFACE_POSITIVEZ";
            case SDL_GPU_CUBEMAPFACE_NEGATIVEZ:
                return "SDL_GPU_CUBEMAPFACE_NEGATIVEZ";
            default:
                return "unknown";
        }
    }
} // namespace gfx_testing::util
