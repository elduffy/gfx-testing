#pragma once
#include <sdl.hpp>

namespace gfx_testing::io {
    sdl::SdlSurface loadBmp(const std::string &path);
}
