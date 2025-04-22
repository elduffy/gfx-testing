#pragma once

#include <filesystem>

#define NO_COPY(T) T(const T &) = delete
#define NO_MOVE(T) T(T &&) = delete
#define NO_COPY_NO_MOVE(T) \
NO_COPY(T); \
NO_MOVE(T)


namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();

    struct Extent2D {
        uint32_t mWidth;
        uint32_t mHeight;
    };
}
