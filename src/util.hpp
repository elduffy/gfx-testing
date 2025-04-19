#pragma once

#include <filesystem>

namespace gfx_testing::util {
    std::filesystem::path getProjectRoot();


    struct Extent2D {
        uint32_t mWidth;
        uint32_t mHeight;
    };
}
