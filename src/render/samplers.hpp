#pragma once

#include <sdl.hpp>

namespace gfx_testing::render {
    class Samplers {
    public:
        explicit Samplers(sdl::SdlContext const &context);

        sdl::SdlGpuSampler mAnisotropicWrap;
    };
} // namespace gfx_testing::render
