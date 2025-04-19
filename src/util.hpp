#pragma once

#include <filesystem>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();


    struct Extent2D {
        int32_t mWidth;
        int32_t mHeight;
    };
}
