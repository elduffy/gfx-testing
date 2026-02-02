#pragma once

#include <SDL3/SDL_gpu.h>
#include <absl/log/check.h>
#include <filesystem>
#include <ostream>
#include <source_location>

#include "glm/geometric.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#define DEFAULT_MOVE(T) T(T &&) = default
#define NO_COPY(T) T(const T &) = delete;
#define NO_MOVE(T) T(T &&) = delete
#define NO_COPY_NO_MOVE(T)                                                                                             \
    NO_COPY(T);                                                                                                        \
    NO_MOVE(T)
#define NO_COPY_DEFAULT_MOVE(T) NO_COPY(T) DEFAULT_MOVE(T)

// ReSharper disable once CppUnnamedNamespaceInHeaderFile
namespace {
    template<typename... Args>
    [[noreturn]] void failImpl(std::source_location const &location, std::format_string<Args...> message,
                               Args &&...args) {
        ABSL_LOG_INTERNAL_CHECK_IMPL(false, "FAIL() called:").AtLocation(location.file_name(), location.line())
                << std::format(message, std::forward<Args>(args)...);
    }
} // namespace
#define FAIL(...) failImpl(std::source_location::current(), __VA_ARGS__)
#define TODO(...) FAIL(__VA_ARGS__)

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();

    struct Extent2D {
        uint32_t mWidth;
        uint32_t mHeight;

        friend bool operator==(const Extent2D &lhs, const Extent2D &rhs) {
            return lhs.mWidth == rhs.mWidth && lhs.mHeight == rhs.mHeight;
        }

        friend std::ostream &operator<<(std::ostream &os, const Extent2D &obj) {
            return os << "mWidth: " << obj.mWidth << " mHeight: " << obj.mHeight;
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

    template<typename InputContainer, typename Transform>
    std::string joinToString(InputContainer const &input, std::string_view separator, Transform const &t) {
        std::stringstream ss;
        bool first = true;
        for (auto const &elem: input) {
            if (!first) {
                ss << separator;
            } else {
                first = false;
            }
            ss << t(elem);
        }
        return ss.str();
    }

    inline std::string toString(bool b) { return b ? "true" : "false"; }

    inline std::string toString(glm::vec3 const &vec) {
        std::stringstream ss;
        ss << vec.x << ", " << vec.y << ", " << vec.z;
        return ss.str();
    }
} // namespace gfx_testing::util
