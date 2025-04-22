#pragma once

#include <sdl.hpp>

namespace gfx_testing::pipeline {
    class Pipelines {
    public:
        Pipelines(sdl::SdlContext const &sdlContext, std::filesystem::path const &projectRoot);

        sdl::SdlGfxPipeline mDiffuse;
        sdl::SdlGfxPipeline mGooch;
    };
}
